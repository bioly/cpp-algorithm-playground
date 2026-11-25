#include "consistent_hashing/consistent_hashing.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "check.h"

namespace refresher {
namespace consistent_hashing {

// ASSUMPTIONS
//  * Single thread. A real router is read by many threads and changed rarely,
//    so you would swap in a whole new ring (or use a reader/writer lock).
//  * Server names are unique and contain no '#' ambiguity problems.
//  * Two points with the same hash: the first one inserted keeps the spot. With
//    64-bit hashes this practically never happens, so we do not handle it more.

std::uint64_t Hash64(const std::string& text) {
    // Step 1, FNV-1a: for each byte, xor it in, then multiply by a prime.
    // Cheap and good enough to spread names; the constants are the standard ones.
    std::uint64_t h = 1469598103934665603ULL;
    for (unsigned char c : text) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    // Step 2, "finalizer" (from MurmurHash3): FNV alone leaves similar names
    // like "A#1", "A#2" clumped together. These shifts/multiplies scramble all
    // bits, so nearby names land far apart on the ring. Even spread = fair.
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

int ModuloServer(const std::string& key, int server_count) {
    return static_cast<int>(Hash64(key) % static_cast<std::uint64_t>(server_count));
}

HashRing::HashRing(int virtual_nodes, HashFn hash)
    : virtual_nodes_(virtual_nodes), hash_(std::move(hash)) {}

bool HashRing::AddServer(const std::string& name) {
    // set::insert returns {iterator, was_inserted}; `.second` is the bool.
    if (!servers_.insert(name).second) return false;
    for (int i = 0; i < virtual_nodes_; ++i) {
        // Each virtual node is the SAME server under a different label:
        // "A#0", "A#1", ... Hash each label to get that node's spot.
        // emplace() does nothing if the spot is taken (see ASSUMPTIONS).
        ring_.emplace(hash_(name + "#" + std::to_string(i)), name);
    }
    return true;
}

bool HashRing::RemoveServer(const std::string& name) {
    if (servers_.erase(name) == 0) return false;
    for (int i = 0; i < virtual_nodes_; ++i) {
        auto it = ring_.find(hash_(name + "#" + std::to_string(i)));
        // Check the owner: after a (rare) collision the spot may belong to
        // someone else, and we must not delete THEIR point.
        if (it != ring_.end() && it->second == name) ring_.erase(it);
    }
    return true;
}

std::optional<std::string> HashRing::ServerFor(const std::string& key) const {
    if (ring_.empty()) return std::nullopt;
    // lower_bound = binary search for the first spot >= the key's spot. This is
    // "walk clockwise until you meet a server" in O(log N). A key sitting
    // exactly on a server's spot belongs to that server (>= includes equality).
    auto it = ring_.lower_bound(hash_(key));
    if (it == ring_.end()) it = ring_.begin();  // walked past the top: wrap around
    return it->second;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------
namespace {

// The notes' clock. A hash that returns hand-picked spots instead of real
// hashes, so we can check the exact example from the lecture.
// Servers have ONE virtual node each, so their labels are "A#0", "B#0", ...
std::uint64_t ClockHash(const std::string& text) {
    static const std::map<std::string, std::uint64_t> spots = {
        {"A#0", 2}, {"B#0", 6}, {"C#0", 10}, {"D#0", 4},      // servers
        {"Anna", 1}, {"Ben", 3}, {"Cara", 7}, {"Dan", 11},    // customers
    };
    return spots.at(text);
}

// Who owns each of key0..key{n-1}?
std::vector<std::string> Owners(const HashRing& ring, int n) {
    std::vector<std::string> owners;
    owners.reserve(n);
    for (int i = 0; i < n; ++i) owners.push_back(*ring.ServerFor("key" + std::to_string(i)));
    return owners;
}

// How far is the busiest/emptiest server from a fair share? 0.0 = perfect,
// 0.5 = someone holds 50% more (or less) than their fair share.
double Unfairness(const HashRing& ring, int keys) {
    std::unordered_map<std::string, int> load;
    for (const std::string& owner : Owners(ring, keys)) ++load[owner];
    double fair = static_cast<double>(keys) / ring.ServerCount();
    double worst = 0;
    for (const auto& entry : load) worst = std::max(worst, std::abs(entry.second - fair) / fair);
    return worst;
}

// Servers are named "A", "B", ... A different `prefix` gives different names,
// so different spots on the ring: a different "random draw" of the same setup.
HashRing RingWith(int virtual_nodes, int servers, const std::string& prefix = "") {
    HashRing ring(virtual_nodes);
    for (int i = 0; i < servers; ++i) {
        ring.AddServer(prefix + std::string(1, static_cast<char>('A' + i)));
    }
    return ring;
}

void PrintTrace() {
    const int keys = 10000;
    // One ring is one lucky/unlucky draw, so we average 20 different rings.
    std::cout << "\n  Unfairness with 4 servers (worst server vs fair share, avg of 20 rings):\n";
    for (int v : {1, 10, 100, 1000}) {
        double sum = 0;
        for (int r = 0; r < 20; ++r) sum += Unfairness(RingWith(v, 4, "r" + std::to_string(r) + "-"), keys);
        double u = sum / 20;
        std::cout << "    " << std::setw(4) << v << " spots/server  "
                  << std::string(static_cast<size_t>(u * 30), '#') << ' '
                  << static_cast<int>(u * 100) << "%\n";
    }
    HashRing before = RingWith(100, 3), after = RingWith(100, 4);
    int ring_moved = 0, mod_moved = 0;
    std::vector<std::string> a = Owners(before, keys), b = Owners(after, keys);
    for (int i = 0; i < keys; ++i) {
        ring_moved += a[i] != b[i];
        std::string k = "key" + std::to_string(i);
        mod_moved += ModuloServer(k, 3) != ModuloServer(k, 4);
    }
    std::cout << "\n  Adding a 4th server, share of " << keys << " keys that move:\n"
              << "    remainder rule  " << mod_moved * 100 / keys << "%\n"
              << "    ring            " << ring_moved * 100 / keys << "%\n";
}

}  // namespace

bool Run() {
    // --- The notes' clock, exactly -------------------------------------------
    {
        HashRing ring(1, &ClockHash);
        for (const char* s : {"A", "B", "C"}) ring.AddServer(s);
        CHECK(ring.ServerFor("Anna") == "A");  // 1 -> walks to 2
        CHECK(ring.ServerFor("Ben") == "B");   // 3 -> 6
        CHECK(ring.ServerFor("Cara") == "C");  // 7 -> 10
        CHECK(ring.ServerFor("Dan") == "A");   // 11 -> past 12 -> 2  (the wrap-around)

        ring.AddServer("D");                   // D lands on 4
        CHECK(ring.ServerFor("Ben") == "D");   // only Ben was in the stretch before D
        CHECK(ring.ServerFor("Anna") == "A");  // nobody else moved
        CHECK(ring.ServerFor("Cara") == "C");
        CHECK(ring.ServerFor("Dan") == "A");

        ring.RemoveServer("D");
        ring.RemoveServer("B");                // B crashes
        CHECK(ring.ServerFor("Ben") == "C");   // Ben walks on to C
        CHECK(ring.ServerFor("Anna") == "A");
    }

    // --- Basics and edge cases ------------------------------------------------
    {
        HashRing empty;
        CHECK(!empty.ServerFor("anna").has_value());       // no servers: no answer

        HashRing ring(50);
        CHECK(ring.AddServer("A"));
        CHECK(!ring.AddServer("A"));                       // adding twice does nothing
        CHECK(ring.PointCount() == 50);                    // 1 server = 50 spots
        CHECK(!ring.RemoveServer("nobody"));
        bool all_a = true;
        for (int i = 0; i < 100; ++i) all_a = all_a && ring.ServerFor("k" + std::to_string(i)) == "A";
        CHECK(all_a);                                      // the only server owns every key
        CHECK(ring.ServerFor("anna") == ring.ServerFor("anna"));  // same key, same answer
        CHECK(ring.RemoveServer("A") && ring.PointCount() == 0);
    }

    // --- Why remainders break, and why the ring does not ----------------------
    {
        const int keys = 10000;
        int mod_moved = 0;
        for (int i = 0; i < keys; ++i) {
            std::string k = "key" + std::to_string(i);
            mod_moved += ModuloServer(k, 3) != ModuloServer(k, 4);
        }
        CHECK(mod_moved > keys * 6 / 10);                  // about 75% move

        HashRing ring = RingWith(100, 3);
        std::vector<std::string> before = Owners(ring, keys);
        ring.AddServer("D");
        std::vector<std::string> after = Owners(ring, keys);
        int ring_moved = 0;
        bool movers_go_to_d = true;
        for (int i = 0; i < keys; ++i) {
            if (before[i] != after[i]) {
                ++ring_moved;
                movers_go_to_d = movers_go_to_d && after[i] == "D";
            }
        }
        CHECK(movers_go_to_d);                             // movers ONLY go to the new server
        CHECK(ring_moved > keys * 15 / 100 && ring_moved < keys * 35 / 100);  // about 25%
        CHECK(ring_moved * 2 < mod_moved);                 // far fewer than the remainder rule
    }

    // --- Removing a server: only ITS keys move --------------------------------
    {
        const int keys = 5000;
        HashRing ring = RingWith(100, 4);
        std::vector<std::string> before = Owners(ring, keys);
        ring.RemoveServer("B");
        std::vector<std::string> after = Owners(ring, keys);
        bool b_keys_rehomed = true, others_stayed = true;
        for (int i = 0; i < keys; ++i) {
            if (before[i] == "B") b_keys_rehomed = b_keys_rehomed && after[i] != "B";
            else others_stayed = others_stayed && after[i] == before[i];
        }
        CHECK(b_keys_rehomed);                             // B's keys found a new home
        CHECK(others_stayed);                              // everyone else stayed put
    }

    // --- Virtual nodes make it fair --------------------------------------------
    {
        double one = Unfairness(RingWith(1, 4), 10000);
        double many = Unfairness(RingWith(200, 4), 10000);
        CHECK(many < one);                                 // more spots, fairer
        CHECK(many < 0.2);                                 // nobody off by more than 20%
    }

    // --- What the ring does NOT solve: one popular key --------------------------
    {
        HashRing ring = RingWith(1000, 4);
        auto adele = ring.ServerFor("adele");
        bool always_same = true;
        for (int i = 0; i < 100; ++i) always_same = always_same && ring.ServerFor("adele") == adele;
        CHECK(always_same);
        // However many spots we add, "adele" is ONE key on ONE spot: all her
        // fans hit one server. Cache in front or split the key (adele#1..#8).
    }

    if (Verbose()) PrintTrace();
    return AllChecksPassed();
}

}  // namespace consistent_hashing
}  // namespace refresher
