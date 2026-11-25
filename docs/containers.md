# C++ container cheat sheet

A refresher for "which container do I pick?". Every exercise in this repo has
inline `CONTAINER:` notes that apply these rules to real code.

Reference: <https://en.cppreference.com/w/cpp/container>

## Pick in 10 seconds

1. **Default: `std::vector`.** One block of memory, fastest to scan, good for almost everything.
2. Need to find a value **by key**? **`std::unordered_map`** (average O(1)).
3. Need keys **sorted**, or "first key >= x", or a range? **`std::map`** (O(log n)).
4. Need a **queue** (add at the back, remove at the front)? **`std::deque`**.
5. Need to **move or erase in the middle** while keeping iterators valid? **`std::list`**.
6. Need the **biggest/smallest** repeatedly? **`std::priority_queue`**.
7. Size known at compile time (e.g. 256 letters)? **`std::array`**.

## Comparison

| Container | Find | Add / remove | Order | Notes |
|---|---|---|---|---|
| [`vector`](https://en.cppreference.com/w/cpp/container/vector) | O(n); by index O(1) | back O(1)*, elsewhere O(n) | as inserted | contiguous; growth may move everything (`*` amortized) |
| [`array`](https://en.cppreference.com/w/cpp/container/array) | by index O(1) | fixed size | as inserted | size is part of the type; lives on the stack |
| [`deque`](https://en.cppreference.com/w/cpp/container/deque) | by index O(1) | both ends O(1) | as inserted | chunks of memory, not one block |
| [`list`](https://en.cppreference.com/w/cpp/container/list) | O(n) | anywhere O(1) given an iterator | as inserted | one allocation per element; iterators survive changes |
| [`map`](https://en.cppreference.com/w/cpp/container/map) / [`set`](https://en.cppreference.com/w/cpp/container/set) | O(log n) | O(log n) | **sorted** | `lower_bound`, `upper_bound`, range scans |
| [`unordered_map`](https://en.cppreference.com/w/cpp/container/unordered_map) / [`unordered_set`](https://en.cppreference.com/w/cpp/container/unordered_set) | O(1) avg, O(n) worst | O(1) avg | none | needs a hash; rehash invalidates iterators |
| [`priority_queue`](https://en.cppreference.com/w/cpp/container/priority_queue) | top O(1) | push/pop O(log n) | heap | adapter over a vector; no iteration, no search |
| [`stack`](https://en.cppreference.com/w/cpp/container/stack) / [`queue`](https://en.cppreference.com/w/cpp/container/queue) | top/front O(1) | O(1) | LIFO / FIFO | adapters over `deque`; hide everything but the ends |

`map` / `set` are usually red-black trees; `unordered_*` are hash tables.
`map` and `set` hold **unique** keys; `multimap` / `multiset` allow duplicates.

## Things that bite

- **`operator[]` on a map inserts** a default value when the key is missing, even on a plain read. Use `find()` / `contains()` (C++20) to only look.
- **Iterator invalidation** (https://en.cppreference.com/w/cpp/container#Iterator_invalidation):
  `vector` push_back may invalidate everything; `list` and `map` keep iterators valid except for the erased one; `unordered_*` rehash invalidates iterators (references to elements stay valid).
- **`priority_queue` is a max-heap**: `top()` is the *largest* by the comparator. For a min-heap or a custom order the comparator direction is reversed from what you expect.
- **`std::sort` needs random access**: it works on `vector`, `deque`, `array`, not on `list` (use `list::sort`).
- **`vector<bool>` is special** (packed bits): avoid it, use `vector<char>` or `bitset`.
- **Move-only elements** (like `unique_ptr`) can be moved into containers, but the container then cannot be copied.

## Algorithms that go with them

Header `<algorithm>`: <https://en.cppreference.com/w/cpp/algorithm>
`sort`, `lower_bound` / `binary_search` (sorted ranges), `max_element`, `nth_element`, `partial_sort`, `unique`, `reverse`, `accumulate` (in `<numeric>`).
