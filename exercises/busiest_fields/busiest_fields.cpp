#include "busiest_fields/busiest_fields.h"

#include <cstddef>
#include <iostream>
#include <map>
#include <queue>
#include <unordered_map>

#include "check.h"

namespace refresher {
namespace busiest_fields {

namespace {

struct Entry {
    int field;
    int count;
};

// std::priority_queue keeps the LARGEST element (by this comparator) on top.
// We want the WORST candidate on top (the one to throw out), so we define
// "a < b" to mean "a ranks HIGHER than b". Then the "largest" is whoever ranks
// lowest. This direction trap is the most common bug with heaps.
// A struct with operator() is called a functor: an object you can call like a
// function. (A lambda works too, but then the heap's type needs decltype.)
struct RanksHigher {
    bool operator()(const Entry& a, const Entry& b) const {
        if (a.count != b.count) return a.count > b.count;  // more loads ranks higher
        return a.field < b.field;                          // tie: smaller id ranks higher
    }
};

// CONTAINER: std::priority_queue<Entry, std::vector<Entry>, RanksHigher>.
//   Template arguments: what it stores, the container it is built ON (a
//   vector, kept as a heap with std::push_heap / pop_heap), and the ordering.
//   It is an ADAPTER: it only offers push, pop, top, size. top() is O(1),
//   push and pop are O(log size). You cannot iterate it or search it, which is
//   why the Trace below copies it and pops the copy to look inside.
//   Why not sort everything: O(m log m) for m distinct fields, versus
//   O(m log k) here. With k = 10 and a million fields that is a big win.
//   Why not std::set: also keeps order, but each node is a separate
//   allocation and we never need "find" or "erase from the middle".
//   Related algorithms for the same job: std::partial_sort, std::nth_element.
//   Compare all containers: docs/containers.md
//   Docs: https://en.cppreference.com/w/cpp/container/priority_queue
//         https://en.cppreference.com/w/cpp/algorithm/nth_element
using Heap = std::priority_queue<Entry, std::vector<Entry>, RanksHigher>;

}  // namespace

std::vector<int> BusiestFields(const std::vector<int>& reports, int k) {
    if (k <= 0) return {};

    // Step 1: count loads per field. operator[] creates a missing entry as 0.
    // CONTAINER: std::unordered_map<field, count>. We only ever ask "what is
    //   THIS field's count?" and "give me everything once". Hashing does that in
    //   O(1) on average. std::map would also work (O(log n), sorted by field);
    //   the Trace uses it only so its printout has a fixed order.
    //   Alternative without hashing: sort a copy of `reports` and count equal
    //   neighbours: O(n log n), no extra table.
    //   Docs: https://en.cppreference.com/w/cpp/container/unordered_map
    std::unordered_map<int, int> loads;
    for (int field : reports) ++loads[field];

    // Step 2: the bouncer. The heap never holds more than k entries for long.
    Heap heap;
    for (const auto& entry : loads) {  // entry.first = field, entry.second = count
        heap.push({entry.first, entry.second});
        // Over capacity: the top is the WORST candidate, so it leaves. If the
        // newcomer is worse than everyone, it is the top and leaves at once.
        if (heap.size() > static_cast<std::size_t>(k)) heap.pop();
    }

    // Step 3: the heap pops worst-first, so fill the answer from the back.
    std::vector<int> result(heap.size());
    for (std::size_t i = result.size(); i-- > 0;) {  // counts down an unsigned index to 0
        result[i] = heap.top().field;
        heap.pop();
    }
    return result;
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Same bouncer loop with prints. (std::map, not unordered_map, only so the
// demo prints in a fixed order.)
void Trace() {
    const std::vector<int> reports = {3, 7, 3, 5, 3, 7, 9, 3, 7, 5, 3, 2, 7, 3};
    const int k = 2;
    std::map<int, int> loads;
    for (int field : reports) ++loads[field];

    Heap heap;
    for (const auto& entry : loads) {
        heap.push({entry.first, entry.second});
        std::cout << "  field " << entry.first << " has " << entry.second << " loads";
        if (heap.size() > static_cast<std::size_t>(k)) {
            std::cout << "  -> too many, worst leaves: field " << heap.top().field;
            heap.pop();
        }
        Heap copy = heap;  // copy so we can pop and look inside without changing the real heap
        std::cout << "   heap now (worst first):";
        for (; !copy.empty(); copy.pop()) std::cout << " " << copy.top().field;
        std::cout << "\n";
    }
}

}  // namespace

bool Run() {
    using V = std::vector<int>;

    CHECK(BusiestFields({}, 3).empty());
    CHECK(BusiestFields({1, 2, 3}, 0).empty());
    CHECK(BusiestFields({1, 1, 1, 2, 2, 3}, 2) == (V{1, 2}));
    CHECK(BusiestFields({4, 4, 4, 4}, 1) == (V{4}));
    CHECK(BusiestFields({1, 2, 3}, 5) == (V{1, 2, 3}));     // k larger than the fields
    CHECK(BusiestFields({5, 9, 5, 9, 7}, 2) == (V{5, 9}));  // tie: smaller id first
    CHECK(BusiestFields({-3, -3, 8, 8, 8, 0}, 2) == (V{8, -3}));
    CHECK(BusiestFields({3, 7, 3, 5, 3, 7, 9, 3, 7, 5, 3, 2, 7, 3}, 2) == (V{3, 7}));
    CHECK(BusiestFields({1, 2, 2, 2, 3, 3}, 3) == (V{2, 3, 1}));  // busiest first

    V big;  // a large input: every field has 1000 loads, field 42 gets one more
    for (int i = 0; i < 1000000; ++i) big.push_back(i % 1000);
    big.push_back(42);
    CHECK(BusiestFields(big, 1) == (V{42}));

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace busiest_fields
}  // namespace refresher
