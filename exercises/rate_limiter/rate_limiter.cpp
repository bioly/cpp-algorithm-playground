#include "rate_limiter/rate_limiter.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

#include "check.h"

namespace refresher {
namespace rate_limiter {

// ASSUMPTIONS (read these first)
//  * One process, one thread. Real systems share counters in Redis and need
//    atomic steps (INCR / Lua), otherwise two servers both read 9 and both
//    save 10 (the "race" in the notes). Not modelled here.
//  * `now` never goes backwards (we trust the caller's clock).
//  * A rejected request does NOT count against the client (simplest choice;
//    the notes mention variants that do count it).
//  * Per-client state is created on first sight and never deleted. A real
//    limiter expires idle clients (Redis EXPIRE) so memory cannot grow forever.

// C++ NOTE: everything below sits in an anonymous namespace. Names there are
// private to this .cpp file (the modern replacement for `static` functions
// and classes). Callers only ever see the factories declared in the header.
namespace {

Decision Allowed(int remaining) { return Decision{true, remaining, 0.0}; }
Decision Rejected(Seconds retry_after) { return Decision{false, 0, retry_after}; }

// Which fixed-size slot of time does `now` fall in?  window 60: t=0..59.9 is
// slot 0, t=60..119.9 is slot 1. floor() works for any double.
long long WindowIndex(Seconds now, Seconds window) {
    return static_cast<long long>(std::floor(now / window));
}

// ---------------------------------------------------------------------------
// 1. FIXED WINDOW COUNTER
//    State per client: slot number + count.   Time O(1), space O(1) per client.
//    Flaw: the counter resets on the slot border, so a client can send `limit`
//    requests just before the border and `limit` just after: 2x in a blink.
// ---------------------------------------------------------------------------
class FixedWindowCounter : public RateLimiter {
public:
    FixedWindowCounter(int limit, Seconds window) : limit_(limit), window_(window) {}

    // C++ NOTE: `override` makes the compiler check that this REALLY overrides a
    // base virtual (a typo in name or signature becomes an error, not a silent
    // new function). Always write it.
    Decision Allow(const std::string& client, Seconds now) override {
        // C++ NOTE: operator[] on a map creates a default State{0,0} for a new
        // client. That is exactly what we want here ("never seen = empty").
        State& s = clients_[client];
        long long slot = WindowIndex(now, window_);
        if (slot != s.slot) {  // a new minute started: start counting from zero
            s.slot = slot;
            s.count = 0;
        }
        if (s.count >= limit_) {
            return Rejected((slot + 1) * window_ - now);  // wait for the next slot
        }
        ++s.count;
        return Allowed(limit_ - s.count);
    }

private:
    struct State { long long slot = -1; int count = 0; };
    int limit_;
    Seconds window_;
    std::unordered_map<std::string, State> clients_;
};

// ---------------------------------------------------------------------------
// 2. SLIDING WINDOW LOG
//    State per client: the timestamps of accepted requests inside the window.
//    Exact answer to "how many in the LAST `window` seconds?".
//    Time O(1) amortized (each timestamp is added once and removed once),
//    space O(limit) per client: the price of accuracy.
// ---------------------------------------------------------------------------
class SlidingWindowLog : public RateLimiter {
public:
    SlidingWindowLog(int limit, Seconds window) : limit_(limit), window_(window) {}

    Decision Allow(const std::string& client, Seconds now) override {
        // C++ NOTE: std::deque = queue with O(1) push_back AND pop_front.
        // Timestamps arrive in order, so the oldest is always at the front and
        // expiring is just "pop while too old". A vector would make pop_front O(n).
        std::deque<Seconds>& log = logs_[client];
        while (!log.empty() && log.front() <= now - window_) {
            log.pop_front();
        }
        if (static_cast<int>(log.size()) >= limit_) {
            // The oldest request leaves the window at front + window.
            return Rejected(log.front() + window_ - now);
        }
        log.push_back(now);
        return Allowed(limit_ - static_cast<int>(log.size()));
    }

private:
    int limit_;
    Seconds window_;
    std::unordered_map<std::string, std::deque<Seconds>> logs_;
};

// ---------------------------------------------------------------------------
// 3. SLIDING WINDOW COUNTER  (fixed window + a weighted guess)
//    Keep two numbers: count in the previous slot and in the current slot.
//    Pretend the previous slot's requests were spread evenly, so only the part
//    that still overlaps the last `window` seconds counts:
//        estimate = previous * (share of previous slot still inside) + current
//    Time O(1), space O(1) per client. An ESTIMATE, but a good one in practice.
// ---------------------------------------------------------------------------
// `elapsed_share` = how far we are into the current slot (0.0 .. 1.0).
// Example from the notes: 45 s into the minute -> 0.75, so 25% of the previous
// minute is still inside the window: Estimate(80, 70, 0.75) = 20 + 70 = 90.
double Estimate(int previous, int current, double elapsed_share) {
    return previous * (1.0 - elapsed_share) + current;
}

class SlidingWindowCounter : public RateLimiter {
public:
    SlidingWindowCounter(int limit, Seconds window) : limit_(limit), window_(window) {}

    Decision Allow(const std::string& client, Seconds now) override {
        State& s = clients_[client];
        long long slot = WindowIndex(now, window_);
        if (slot != s.slot) {
            // Moving to the next slot: current becomes previous. If we skipped
            // more than one slot, the old data is too old to matter at all.
            s.previous = (slot == s.slot + 1) ? s.current : 0;
            s.current = 0;
            s.slot = slot;
        }
        double share = (now - slot * window_) / window_;
        double estimate = Estimate(s.previous, s.current, share);
        if (estimate + 1 > limit_) {  // this request would push us over
            // Simple and safe: the estimate shrinks as time passes, but the
            // exact moment needs maths; "next slot" is an upper bound.
            return Rejected((slot + 1) * window_ - now);
        }
        ++s.current;
        return Allowed(static_cast<int>(std::floor(limit_ - estimate - 1)));
    }

private:
    struct State { long long slot = -1; int previous = 0; int current = 0; };
    int limit_;
    Seconds window_;
    std::unordered_map<std::string, State> clients_;
};

// ---------------------------------------------------------------------------
// 4. TOKEN BUCKET
//    A bucket holds up to `capacity` tokens and refills `rate` tokens/second.
//    Every request takes one token; empty bucket = rejected.
//      capacity -> the biggest burst allowed,  rate -> the long-run average.
//    LAZY refill: no timer thread. We store (tokens, last time) and, when the
//    next request comes, add rate * elapsed. Time O(1), space O(1) per client.
// ---------------------------------------------------------------------------
class TokenBucket : public RateLimiter {
public:
    TokenBucket(int capacity, double rate) : capacity_(capacity), rate_(rate) {}

    Decision Allow(const std::string& client, Seconds now) override {
        // try_emplace: insert a FULL bucket for a new client, or find the
        // existing one. (C++17; the returned pair is {iterator, was_inserted}.)
        auto it = buckets_.try_emplace(client, Bucket{static_cast<double>(capacity_), now}).first;
        Bucket& b = it->second;

        // Refill for the time that passed, but never above capacity.
        b.tokens = std::min<double>(capacity_, b.tokens + (now - b.last) * rate_);
        b.last = now;

        if (b.tokens < 1.0) {
            return Rejected((1.0 - b.tokens) / rate_);  // time until one token exists
        }
        b.tokens -= 1.0;
        return Allowed(static_cast<int>(b.tokens));
    }

private:
    struct Bucket { double tokens; Seconds last; };
    int capacity_;
    double rate_;
    std::unordered_map<std::string, Bucket> buckets_;
};

// ---------------------------------------------------------------------------
// 5. LEAKY BUCKET, "meter" flavour (the notes' version 2, e.g. Shopify)
//    Mirror image of the token bucket: each request ADDS water, the bucket
//    leaks `rate` per second, a full bucket rejects. Same behaviour, so say
//    out loud which leaky bucket you mean in an interview.
//    NOT built here: the QUEUE flavour (Nginx), where requests wait and are
//    released at a fixed speed. That needs a queue plus a worker/timer, and it
//    is the right pick when something DOWNSTREAM needs a perfectly steady flow.
// ---------------------------------------------------------------------------
class LeakyBucket : public RateLimiter {
public:
    LeakyBucket(int capacity, double rate) : capacity_(capacity), rate_(rate) {}

    Decision Allow(const std::string& client, Seconds now) override {
        auto it = buckets_.try_emplace(client, Bucket{0.0, now}).first;  // new = empty
        Bucket& b = it->second;

        b.level = std::max(0.0, b.level - (now - b.last) * rate_);  // leak, floor at 0
        b.last = now;

        if (b.level + 1.0 > capacity_) {
            return Rejected((b.level + 1.0 - capacity_) / rate_);  // until it leaks enough
        }
        b.level += 1.0;
        return Allowed(static_cast<int>(std::floor(capacity_ - b.level)));
    }

private:
    struct Bucket { double level; Seconds last; };
    int capacity_;
    double rate_;
    std::unordered_map<std::string, Bucket> buckets_;
};

}  // namespace

// C++ NOTE: std::make_unique<Derived>(args) builds the object and hands back a
// unique_ptr<Derived>, which converts implicitly to unique_ptr<RateLimiter>.
// unique_ptr = exactly one owner, deletes automatically, no `new`/`delete`.
std::unique_ptr<RateLimiter> MakeFixedWindowCounter(int limit, Seconds window) {
    return std::make_unique<FixedWindowCounter>(limit, window);
}
std::unique_ptr<RateLimiter> MakeSlidingWindowLog(int limit, Seconds window) {
    return std::make_unique<SlidingWindowLog>(limit, window);
}
std::unique_ptr<RateLimiter> MakeSlidingWindowCounter(int limit, Seconds window) {
    return std::make_unique<SlidingWindowCounter>(limit, window);
}
std::unique_ptr<RateLimiter> MakeTokenBucket(int capacity, double refill_per_second) {
    return std::make_unique<TokenBucket>(capacity, refill_per_second);
}
std::unique_ptr<RateLimiter> MakeLeakyBucket(int capacity, double leak_per_second) {
    return std::make_unique<LeakyBucket>(capacity, leak_per_second);
}

// ---------------------------------------------------------------------------
// The service: named rules -> strategies.
// ---------------------------------------------------------------------------
void RateLimitService::AddRule(const std::string& name, std::unique_ptr<RateLimiter> limiter) {
    // C++ NOTE: unique_ptr cannot be copied, only MOVED into the map.
    rules_[name] = std::move(limiter);
}

Decision RateLimitService::Check(const std::string& rule, const std::string& client, Seconds now) {
    auto it = rules_.find(rule);
    if (it == rules_.end()) {
        return Decision{true, -1, 0.0};  // unknown rule: fail OPEN (see header)
    }
    return it->second->Allow(client, now);
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------
namespace {

// How many of `n` back-to-back requests at time `now` get through?
int CountAllowed(RateLimiter& limiter, const std::string& client, Seconds now, int n) {
    int allowed = 0;
    for (int i = 0; i < n; ++i) {
        if (limiter.Allow(client, now).allowed) ++allowed;
    }
    return allowed;
}

// Verbose only: the SAME requests through all five strategies, side by side.
// '#' = allowed, '.' = rejected (HTTP 429). Limit: 3 requests per 10 seconds.
void PrintComparison() {
    const std::vector<Seconds> times = {0, 1, 2, 3, 9, 9.5, 10, 10.5, 11, 12, 19, 20};
    std::vector<std::pair<std::string, std::unique_ptr<RateLimiter>>> all;
    all.emplace_back("fixed window counter", MakeFixedWindowCounter(3, 10));
    all.emplace_back("sliding window log", MakeSlidingWindowLog(3, 10));
    all.emplace_back("sliding window counter", MakeSlidingWindowCounter(3, 10));
    all.emplace_back("token bucket (3, 0.3/s)", MakeTokenBucket(3, 0.3));
    all.emplace_back("leaky bucket (3, 0.3/s)", MakeLeakyBucket(3, 0.3));

    std::cout << "\n  limit 3 per 10 s; requests at t = ";
    for (Seconds t : times) std::cout << t << ' ';
    std::cout << "\n";
    for (auto& entry : all) {
        std::cout << "  " << entry.first << std::string(26 - entry.first.size(), ' ');
        for (Seconds t : times) {
            std::cout << (entry.second->Allow("anna", t).allowed ? '#' : '.') << "  ";
        }
        std::cout << "\n";
    }
}

}  // namespace

bool Run() {
    // --- Fixed window: the burst at the edge (THE classic trap) -------------
    {
        auto fixed = MakeFixedWindowCounter(5, 10);
        CHECK(CountAllowed(*fixed, "anna", 9.5, 7) == 5);   // 5 pass, 2 rejected
        CHECK(CountAllowed(*fixed, "anna", 10.0, 7) == 5);  // new slot: 5 MORE pass
        // 10 requests within half a second, although the limit says 5 per 10 s.

        // The sliding log does not fall for it: same requests, honest answer.
        auto log = MakeSlidingWindowLog(5, 10);
        CHECK(CountAllowed(*log, "anna", 9.5, 7) == 5);
        CHECK(CountAllowed(*log, "anna", 10.0, 7) == 0);    // all 5 still inside the window
        CHECK(CountAllowed(*log, "anna", 19.5, 7) == 5);    // they expired at t = 19.5
    }

    // --- Sliding window log: Retry-After and the exact edge -----------------
    {
        auto log = MakeSlidingWindowLog(3, 10);
        CHECK(log->Allow("a", 0).allowed);
        CHECK(log->Allow("a", 1).allowed);
        Decision third = log->Allow("a", 2);
        CHECK(third.allowed && third.remaining == 0);
        Decision fourth = log->Allow("a", 3);
        CHECK(!fourth.allowed);
        CHECK(fourth.retry_after == 7.0);        // oldest request (t=0) leaves at t=10
        CHECK(log->Allow("a", 10).allowed);      // exactly 10 s later: slot is free again
    }

    // --- Sliding window counter: the formula and its effect -----------------
    {
        CHECK(Estimate(80, 70, 0.75) == 90.0);   // notes: 20 + 70 = 90 -> allowed
        CHECK(Estimate(80, 85, 0.75) == 105.0);  // notes: 105 -> rejected

        auto counter = MakeSlidingWindowCounter(10, 10);
        CHECK(CountAllowed(*counter, "a", 0.0, 8) == 8);    // previous slot: 8 requests
        // 40% into the next slot, 60% of those 8 still count (= 4.8).
        // A fixed window would allow 10 more here; the estimate allows only 5.
        CHECK(CountAllowed(*counter, "a", 14.0, 10) == 5);
    }

    // --- Token bucket: bursts pass, then the refill rate rules --------------
    {
        auto bucket = MakeTokenBucket(20, 1.0);            // notes: 20 tokens, +1 per second
        CHECK(CountAllowed(*bucket, "a", 0.0, 30) == 20);  // 30 at once: 20 pass, 10 rejected
        Decision no = bucket->Allow("a", 0.0);
        CHECK(!no.allowed && no.retry_after == 1.0);       // next token in 1 s
        CHECK(CountAllowed(*bucket, "a", 5.0, 30) == 5);   // 5 s later: 5 tokens came back
        CHECK(CountAllowed(*bucket, "a", 1000.0, 30) == 20);  // idle never banks more than capacity
    }

    // --- Leaky bucket (meter): the mirror image ------------------------------
    {
        auto bucket = MakeLeakyBucket(5, 1.0);
        CHECK(CountAllowed(*bucket, "a", 0.0, 8) == 5);    // full after 5, 3 rejected
        CHECK(CountAllowed(*bucket, "a", 2.0, 8) == 2);    // 2 s leaked 2 units of room
    }

    // --- Clients are independent ---------------------------------------------
    {
        auto bucket = MakeTokenBucket(2, 0.1);
        CHECK(CountAllowed(*bucket, "script", 0.0, 10) == 2);
        CHECK(CountAllowed(*bucket, "anna", 0.0, 10) == 2);  // the script did not hurt Anna
    }

    // --- The service: named rules, each with the strategy that fits it ------
    {
        RateLimitService service;
        service.AddRule("search", MakeTokenBucket(100, 100.0 / 60));          // bursts are fine
        service.AddRule("login", MakeSlidingWindowLog(10, 15 * 60));          // 10 means 10

        CHECK(service.Check("search", "anna", 0).remaining == 99);
        for (int i = 0; i < 10; ++i) CHECK(service.Check("login", "mallory", i).allowed);
        Decision blocked = service.Check("login", "mallory", 10);
        CHECK(!blocked.allowed);                                   // 11th attempt -> 429
        CHECK(blocked.retry_after == 890);                       // attempt at t=0 expires at 900; now is 10
        CHECK(service.Check("login", "anna", 10).allowed);         // other accounts unaffected

        Decision unknown = service.Check("no_such_rule", "anna", 0);
        CHECK(unknown.allowed && unknown.remaining == -1);         // fail open
    }

    if (Verbose()) PrintComparison();
    return AllChecksPassed();
}

}  // namespace rate_limiter
}  // namespace refresher
