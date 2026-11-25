#pragma once

// ============================================================================
// Exercise: Rate Limiter  -  "The Bookstore's Doorman"
// Pattern : Strategy pattern (one interface, many algorithms)   Level: design
// Source  : System Design notes, chapter 4 (rate limiter)
// ============================================================================
//
// STORY
//   An online bookstore must stop scripts from hammering its API. Rules:
//     search: 100 requests per minute per customer
//     login : 10 attempts per 15 minutes per account
//   Request number 101 gets HTTP 429 ("too many requests"), plus a hint like
//   "Retry-After: 12 seconds". The notes present FIVE counting algorithms; each
//   fixes a problem of the one before it. We build them all behind ONE
//   interface, so the rest of the system never knows which one it talks to.
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Server side or client side? (server: clients can be forged)
//                 Who is counted? (customer id / account, flexible)  Different
//                 rules per feature? (yes)  Tell the user when blocked? (yes)
//   2. Simplest   "Keep a counter per customer in a map." That IS the fixed
//      idea.      window counter. Then ask: what goes wrong?
//   3. Insight.   Each algorithm trades something away (limit 100/min):
//
//      Strategy              Idea                          Gives up
//      --------------------  ----------------------------  ------------------
//      Fixed window counter  one counter per minute         burst at the edge:
//                                                           100 at 0:59 + 100
//                                                           at 1:00 = 200 in 2 s
//      Sliding window log    keep every timestamp           memory: limit
//                                                           timestamps/client
//      Sliding window count  current + previous counter,    an ESTIMATE (assumes
//                            weighted by time               even spread)
//      Token bucket          tokens refill steadily; each   bursts up to the
//                            request costs one              bucket size pass
//      Leaky bucket (meter)  level rises per request,       like token bucket,
//                            drains steadily                mirrored
//
//      Pick per feature: SEARCH -> token bucket (real users click in bursts).
//      LOGIN -> sliding window log (10 attempts must mean 10; little memory).
//   4. Complexity. Every Allow() is O(1) time (the log is O(1) amortized).
//                 Space per client: O(1), except the log: O(limit).
//
// WHAT THIS SKIPS (on purpose; see the notes)
//   Redis and atomic INCR (chapter 7), many servers, the YAML rules file, real
//   HTTP headers, fail-open vs fail-closed (chapter 8), and the QUEUE flavour
//   of the leaky bucket. Here all state lives in one process, in a std::map.
//   The design question "where does state live?" is the interviewer's follow-up.
//
// Run:  make run ONLY=rate_limiter V=1
// ============================================================================

#include <map>
#include <memory>
#include <string>

namespace refresher {
namespace rate_limiter {

// Time is a plain number of seconds that the CALLER passes in, instead of the
// limiter reading the clock itself. Why: tests become exact and instant (no
// sleeping), and the same limiter works with any clock. Production code would
// pass std::chrono::steady_clock readings converted to seconds.
using Seconds = double;

// The answer to "may this request pass?". Maps straight to the HTTP response.
struct Decision {
    bool allowed;           // false -> answer HTTP 429
    int remaining;          // requests left (header X-RateLimit-Remaining); -1 = no limit applies
    Seconds retry_after;    // when rejected: seconds to wait (header Retry-After)
};

// The STRATEGY interface. Every algorithm implements Allow().
class RateLimiter {
public:
    // C++ NOTE: a class meant to be used through a base pointer needs a
    // VIRTUAL destructor. Without it, deleting a derived object through a
    // RateLimiter* skips the derived destructor (undefined behaviour).
    virtual ~RateLimiter() = default;

    // C++ NOTE: `= 0` makes this a pure virtual function: RateLimiter is now
    // abstract (cannot be instantiated) and every strategy must provide it.
    // `client` identifies who is counted (a user id, an IP, an API key).
    virtual Decision Allow(const std::string& client, Seconds now) = 0;
};

// Factories: callers get a std::unique_ptr<RateLimiter> and never see the
// concrete classes (they live in the .cpp). `limit` requests per `window` seconds.
std::unique_ptr<RateLimiter> MakeFixedWindowCounter(int limit, Seconds window);
std::unique_ptr<RateLimiter> MakeSlidingWindowLog(int limit, Seconds window);
std::unique_ptr<RateLimiter> MakeSlidingWindowCounter(int limit, Seconds window);
// capacity = biggest burst; refill_per_second = long-run average rate.
std::unique_ptr<RateLimiter> MakeTokenBucket(int capacity, double refill_per_second);
std::unique_ptr<RateLimiter> MakeLeakyBucket(int capacity, double leak_per_second);

// The thin layer the bookstore needs: NAMED rules ("search", "login"), each
// backed by its own strategy. Unknown rule: the request is allowed (that is
// "failing open": a missing rule must not take the shop down).
class RateLimitService {
public:
    // The service takes OWNERSHIP of the limiter. unique_ptr is move-only, so
    // callers write AddRule("x", std::move(p)) or pass a factory result directly.
    void AddRule(const std::string& name, std::unique_ptr<RateLimiter> limiter);
    Decision Check(const std::string& rule, const std::string& client, Seconds now);

private:
    std::map<std::string, std::unique_ptr<RateLimiter>> rules_;
};

bool Run();

}  // namespace rate_limiter
}  // namespace refresher
