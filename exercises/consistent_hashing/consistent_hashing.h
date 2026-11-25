#pragma once

// ============================================================================
// Exercise: Consistent Hashing  -  "Which cache server has Anna's data?"
// Pattern : sorted map + binary search on a circle         Level: design
// Source  : System Design notes, chapter 5 (consistent hashing)
// ============================================================================
//
// STORY
//   The bookstore has 3 cache servers; each customer's data lives on exactly
//   one of them. Every web server must answer the same question the same way:
//   "which server has Anna?"  Then the shop adds a 4th server for the holidays.
//   How many customers must change servers (= cache misses) when it arrives?
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Cache or database? (cache: a move is only a miss; database:
//                 the data must be COPIED.)  How often do servers change?
//                 (rarely -> the remainder rule is enough, do not over-build)
//   2. Simplest   server = hash(name) % number_of_servers. Fast, simple.
//      idea.      Break it: go from 3 to 4 servers. About 75% of customers
//                 move (10 -> 11 servers: about 91%). A flood of misses hits
//                 the database on the busiest day of the year.
//   3. Insight.   Stop dividing by the number of servers. Put servers AND keys
//                 on a circle; a key walks clockwise to the first server.
//
//   4. Complexity. S servers, V virtual nodes each, N = S*V points on the ring.
//                 Lookup      O(log N)   (binary search in a sorted map)
//                 Add/Remove  O(V log N) (insert/erase V points)
//                 Space       O(N)
//
// EXPLANATION: THE RING IN ONE PICTURE
//   A clock with 12 spots (real rings have 2^64). Servers A, B, C sit on 2, 6,
//   10. Customers sit on the spot their hash gives them, then walk CLOCKWISE:
//
//     spot:  1     2    3     4  5    6    7      8  9   10   11    12
//           Anna   [A]  Ben          [B]  Cara          [C]  Dan
//             └──►──┘    └─────►──────┘     └──────►────┘     └─►─ wraps ─► A
//
//   Anna(1) -> A.  Ben(3) -> B.  Cara(7) -> C.  Dan(11) walks past 12 and
//   reaches A on 2. On a circle you ALWAYS reach a server.
//
//   Nobody really walks. We keep a SORTED list of server spots {2, 6, 10} and
//   binary-search "first spot >= my spot"; if there is none, wrap to the first.
//
//   Add server D on spot 4: only customers on spots 3-4 (the stretch right
//   before D) now stop at D. That is Ben. Everybody else stops at their own
//   server first, so for them D does not exist. Remove B: Ben walks on to C.
//   With the remainder rule, 75% move; on the ring only ~25% (and that is the
//   fair price: with 4 servers the new one must get its quarter from somewhere).
//
// EXPLANATION: VIRTUAL NODES (why one spot per server is not fair)
//   Hashes are random. A, B, C might land on 1, 2, 3: then A owns spots 4..12
//   (10 of 12!) and B, C own one each. Fix: each server takes MANY spots, e.g.
//   "A#0", "A#1", ... "A#99", hashed separately. Like cutting a pizza blindly:
//   3 slices for 3 friends is unfair, 300 random slices come out nearly even.
//
//     spots per server:      1      10     100     200     1000
//     typical unfairness:   ~70%   ~21%    ~8%     ~5%     ~2%   (4 servers)
//
//   A virtual node is NOT a replica: it is the same machine appearing many
//   times on the clock. Nothing is copied. A stronger machine can simply be
//   given more spots. The price of more spots: a longer sorted list.
//
// WHAT THIS SKIPS (the interviewer's follow-ups; see the notes)
//   * Moving data. The ring only says where a key SHOULD be. For a cache the
//     old copy just expires; for a database someone must copy it from B to D.
//   * Replicas: keep copies on the next few DISTINCT machines clockwise.
//   * A very popular key (the celebrity "Adele"). One key hashes to one spot,
//     so more virtual nodes cannot help. Cache in front, split the key, or give
//     her a server of her own. Do NOT claim the ring solves this.
//   * Weights, thread safety, hash collisions beyond "first one wins".
//
// Run:  make run ONLY=consistent_hashing V=1
// ============================================================================

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>

namespace refresher {
namespace consistent_hashing {

// A stable 64-bit hash. NOT std::hash: its values differ between compilers
// and platforms (and may change per run), which would break our exact tests,
// and in real life every web server MUST compute the same number.
std::uint64_t Hash64(const std::string& text);

// The naive rule, kept only to show how bad it is.
int ModuloServer(const std::string& key, int server_count);

class HashRing {
public:
    // C++ NOTE: std::function can hold any callable (function, lambda,
    // functor). We inject the hash so tests can use the notes' 12-spot clock.
    using HashFn = std::function<std::uint64_t(const std::string&)>;

    // `virtual_nodes` = spots per server on the ring.
    explicit HashRing(int virtual_nodes = 100, HashFn hash = &Hash64);

    bool AddServer(const std::string& name);     // false if it was already there
    bool RemoveServer(const std::string& name);  // false if it was unknown

    // C++ NOTE: std::optional<T> = "a T or nothing". An empty ring has no
    // answer, so we say so in the type instead of returning "" or throwing.
    std::optional<std::string> ServerFor(const std::string& key) const;

    int ServerCount() const { return static_cast<int>(servers_.size()); }
    int PointCount() const { return static_cast<int>(ring_.size()); }

private:
    int virtual_nodes_;
    HashFn hash_;
    // CONTAINER 1, THE RING: std::map<spot, server>. Keys stay SORTED (usually
    //   a red-black tree), so lower_bound() = "first spot >= x" in O(log N).
    //   That one operation is the whole algorithm.
    //   Why not unordered_map: it has no order, so no "first key >= x".
    //   Alternative used by many real systems: a SORTED std::vector of
    //   {spot, server} plus std::lower_bound. Lookups get faster (contiguous
    //   memory) but add/remove cost O(N). Since servers change rarely and
    //   lookups happen on every request, that is a very good trade; the map is
    //   chosen here because add/remove are then one-liners.
    //   Compare all containers: docs/containers.md
    //   Docs: https://en.cppreference.com/w/cpp/container/map
    //         https://en.cppreference.com/w/cpp/container/map/lower_bound
    std::map<std::uint64_t, std::string> ring_;

    // CONTAINER 2: std::set<name>, the members. Only needed to answer "is this
    //   server already in?" (insert() reports that) and to count servers.
    //   unordered_set would work equally well; set also keeps names sorted.
    //   Docs: https://en.cppreference.com/w/cpp/container/set
    std::set<std::string> servers_;
};

bool Run();

}  // namespace consistent_hashing
}  // namespace refresher
