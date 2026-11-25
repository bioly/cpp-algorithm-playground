#include "lru_cache/lru_cache.h"

#include <iostream>

#include "check.h"

namespace refresher {
namespace lru_cache {

void LruCache::MoveToFront(Shelf::iterator book) {
    // splice() RELINKS an existing node: no copy, no allocation, O(1).
    // Most important: `book` (and the copy of it in where_) stays valid.
    // That guarantee is why we use std::list here. A std::vector would need
    // erase + insert, which is O(n) and invalidates stored positions.
    shelf_.splice(shelf_.begin(), shelf_, book);
}

std::optional<std::string> LruCache::Get(int id) {
    // `auto` = let the compiler write the long iterator type for us.
    auto found = where_.find(id);
    if (found == where_.end()) return std::nullopt;  // miss: change nothing

    // A map entry is a pair: found->first is the id, found->second is the
    // list iterator we stored. So found->second->title is the book's title.
    MoveToFront(found->second);  // reading counts as using
    return found->second->title;
}

void LruCache::Put(int id, const std::string& title) {
    if (capacity_ == 0) return;  // no room at all: store nothing

    auto found = where_.find(id);
    if (found != where_.end()) {
        // The book is already on the shelf: update it and mark it as used.
        // Do NOT evict or add a second entry (the classic bug).
        found->second->title = title;
        MoveToFront(found->second);
        return;
    }

    if (shelf_.size() == capacity_) {
        // Shelf full: the LAST book is the least recently used. Remove it
        // from BOTH containers, or the map keeps a dangling iterator.
        where_.erase(shelf_.back().id);
        shelf_.pop_back();
    }

    // emplace_front builds the Book directly in its new list node.
    shelf_.emplace_front(Book{id, title});
    where_[id] = shelf_.begin();  // remember where this book's node lives
}

std::vector<int> LruCache::Ids() const {
    std::vector<int> ids;
    for (const Book& book : shelf_) ids.push_back(book.id);  // range-based for
    return ids;
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Prints the shelf after each step (only with V=1).
void Trace() {
    LruCache shelf(3);
    // A lambda: a small function defined in place. [&] lets it use `shelf`.
    auto show = [&](const char* step) {
        std::cout << "  " << step << "\n      [most recent]";
        for (int id : shelf.Ids()) std::cout << " " << id;
        std::cout << "  [least recent]\n";
    };
    shelf.Put(1, "Dune");
    show("Put(1)   room left, add at the front");
    shelf.Put(2, "Emma");
    show("Put(2)   room left, add at the front");
    shelf.Put(3, "Ulysses");
    show("Put(3)   shelf is now full");
    shelf.Get(1);
    show("Get(1)   hit: book 1 jumps to the front");
    shelf.Put(4, "Hamlet");
    show("Put(4)   full: evict the back (2), add 4");
    shelf.Get(2);
    show("Get(2)   miss: nothing changes");
    shelf.Put(3, "Annotated");
    show("Put(3)   existing: update + move, no eviction");
    shelf.Put(5, "Emma II");
    show("Put(5)   full: evict the back (1), add 5");
}

}  // namespace

bool Run() {
    using Ids = std::vector<int>;

    {  // nothing on the shelf yet
        LruCache shelf(2);
        CHECK(!shelf.Get(1));  // an empty optional converts to false
    }
    {  // capacity 0 stores nothing
        LruCache shelf(0);
        shelf.Put(1, "Dune");
        CHECK(!shelf.Get(1));
    }
    {  // the oldest book is evicted
        LruCache shelf(2);
        shelf.Put(1, "Dune");
        shelf.Put(2, "Emma");
        shelf.Put(3, "Ulysses");
        CHECK(!shelf.Get(1));
        CHECK(shelf.Get(2) == "Emma");  // optional<string> compares with text
        CHECK(shelf.Get(3) == "Ulysses");
    }
    {  // TRAP 1: reading a book must save it from eviction
        LruCache shelf(2);
        shelf.Put(1, "Dune");
        shelf.Put(2, "Emma");
        shelf.Get(1);             // book 1 is now the most recent...
        shelf.Put(3, "Ulysses");  // ...so book 2 is evicted, not book 1
        CHECK(!shelf.Get(2));
        CHECK(shelf.Get(1) == "Dune");
    }
    {  // TRAP 2: Put of an existing id updates it and never grows the shelf
        LruCache shelf(2);
        shelf.Put(1, "Dune");
        shelf.Put(2, "Emma");
        shelf.Put(1, "Dune, 2nd edition");
        CHECK(shelf.Ids() == (Ids{1, 2}));
        CHECK(shelf.Get(1) == "Dune, 2nd edition");
    }
    {  // a miss must not change the order
        LruCache shelf(2);
        shelf.Put(1, "Dune");
        shelf.Put(2, "Emma");
        shelf.Get(99);
        CHECK(shelf.Ids() == (Ids{2, 1}));
    }

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace lru_cache
}  // namespace refresher
