#pragma once

// ============================================================================
// Exercise: LRU Cache  -  "The Library Shelf"
// Pattern : hash map + linked list              Level: medium (classic)
// ============================================================================
//
// STORY
//   A library keeps a small shelf at the front desk with room for N books.
//   A reader asks for a book by its id.
//     - On the shelf?  Hand it over at once.
//     - Not there?     Fetch it from the basement (slow) and put it on the shelf.
//   When the shelf is full, the book used LEAST RECENTLY goes back to the
//   basement. Browsers, databases and CPUs all work like this.
//
//   Two operations, and BOTH must be O(1):
//     Get(id)         the title, or "nothing" if the book is not on the shelf
//     Put(id, title)  put the book on the shelf (or update its title)
//   Reading a book and putting a book both count as "using" it.
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   "Does Get count as a use?" (yes)  "Does a miss change
//                  anything?" (no)  "Put of an existing id?" (update, no eviction)
//                  "Capacity 0?" (store nothing)
//   2. Brute force. Keep a vector ordered by use. Finding a book and moving it
//                  to the front are both O(n). Too slow, but correct.
//   3. Insight.   I need TWO things fast:
//                    a) find a book by id        -> hash map          O(1)
//                    b) know and change the order -> linked list      O(1)
//                  A list can move a node in O(1) only if I already hold a
//                  pointer (iterator) to it. So the map stores exactly that:
//
//                    where_ (hash map)        shelf_ (list: front = most recent)
//                      id 3 ───────────────►  [3] ⇄ [1] ⇄ [2]
//                      id 1 ──────────────────────┘     ▲
//                      id 2 ────────────────────────────┘
//
//   4. Complexity. Get and Put: O(1) on average (hash lookup + list relink).
//                  Space: O(capacity).
//
// Run:  make run ONLY=lru_cache V=1      (V=1 prints a step-by-step trace)
// ============================================================================

#include <cstddef>
#include <list>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace refresher {
namespace lru_cache {

class LruCache {
public:
    // `explicit`: no accidental conversion from a number to a cache.
    explicit LruCache(std::size_t capacity) : capacity_(capacity) {}

    // std::optional<T> (C++17) means "a T, or nothing". It beats returning
    // -1 or "" for "not found", because the type itself says it can be empty.
    std::optional<std::string> Get(int id);

    // `const std::string&`: read the title without copying it first.
    void Put(int id, const std::string& title);

    // Ids from most to least recently used (used by the tests and the trace).
    std::vector<int> Ids() const;

private:
    struct Book {
        int id;
        std::string title;
    };
    using Shelf = std::list<Book>;  // alias: a shorter name for a long type

    void MoveToFront(Shelf::iterator book);

    std::size_t capacity_;
    Shelf shelf_;                                     // front = most recently used
    std::unordered_map<int, Shelf::iterator> where_;  // id -> that book's node
    // NOTE: copying this class would be a BUG: the copied map would still point
    // into the ORIGINAL list. We simply never copy it here. A production
    // version would write `LruCache(const LruCache&) = delete;` (a good
    // follow-up to mention to the interviewer).
};

// Entry point called by the runner: runs the tests, returns true if all pass.
bool Run();

}  // namespace lru_cache
}  // namespace refresher
