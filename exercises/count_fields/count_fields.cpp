#include "count_fields/count_fields.h"

#include <algorithm>
#include <iostream>
#include <utility>

#include "check.h"

namespace refresher {
namespace count_fields {

Grid LabelFields(const Grid& grid) {
    // int (not size_t) for indices: neighbour math like r - 1 can reach -1, and
    // with an unsigned type that would wrap around to a huge number.
    const int rows = static_cast<int>(grid.size());
    const int cols = rows == 0 ? 0 : static_cast<int>(grid[0].size());

    // The paint map. 0 = not painted. It also doubles as the "visited" record.
    Grid label(rows, std::vector<int>(cols, 0));

    // The four allowed moves: up, down, left, right (no diagonals!). Looping
    // over two small tables avoids four near-identical blocks of code.
    // CONTAINER: plain C arrays of 4. Fine for a tiny fixed table. Modern
    // alternative: std::array<int, 4>, same speed but it knows its own size()
    // and works with algorithms and range-for.
    // Docs: https://en.cppreference.com/w/cpp/container/array
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    // An explicit stack instead of recursion: a big all-land grid would
    // recurse millions of levels and overflow the call stack. A std::vector
    // used with push_back / back / pop_back is a perfectly good stack.
    // CONTAINER: std::vector<pair<row, col>> used as a stack (push_back, back,
    //   pop_back are all O(1) at the end).
    //   Why not std::stack: it is only an adapter that hides everything but
    //   top/push/pop. Same behaviour; the vector is just more direct to read.
    //   Why not std::queue / deque: a QUEUE gives breadth-first order (spread
    //   out ring by ring), a STACK gives depth-first (dive deep first). For
    //   "just label the whole field" both give the same count, so the cheaper
    //   plain vector wins.
    //   Why pair: two ints travelling together without declaring a struct.
    //   Docs: https://en.cppreference.com/w/cpp/container/vector
    //         https://en.cppreference.com/w/cpp/utility/pair
    std::vector<std::pair<int, int>> stack;
    int fields = 0;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Skip water, and skip land that is already painted.
            if (grid[r][c] != 1 || label[r][c] != 0) continue;

            ++fields;  // new unpainted land: a new field
            label[r][c] = fields;
            stack.push_back({r, c});
            while (!stack.empty()) {
                // C++17 structured binding: unpacks the pair into two names.
                // `auto` (a COPY, not auto&) because pop_back() destroys the
                // element; a reference to it would dangle.
                auto [cr, cc] = stack.back();
                stack.pop_back();
                for (int d = 0; d < 4; ++d) {
                    const int nr = cr + dr[d];
                    const int nc = cc + dc[d];
                    if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;  // off the map
                    if (grid[nr][nc] != 1 || label[nr][nc] != 0) continue;       // water / done
                    label[nr][nc] = fields;     // paint when PUSHING, so a cell never
                    stack.push_back({nr, nc});  // sits on the stack twice
                }
            }
        }
    }
    return label;
}

int CountFields(const Grid& grid) {
    int count = 0;
    for (const auto& row : LabelFields(grid))  // range-based for over a temporary is fine
        for (int label : row) count = std::max(count, label);
    return count;  // the highest label number = the number of fields
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Prints the painted map using the REAL code: A, B, C... are the fields.
void Trace() {
    const Grid grid = {{1, 1, 0, 0, 0}, {1, 1, 0, 0, 1}, {0, 0, 1, 0, 1}, {0, 0, 0, 1, 1}};
    std::cout << "  map (. = water)         painted (letter = field)\n";
    const Grid label = LabelFields(grid);
    for (std::size_t r = 0; r < grid.size(); ++r) {
        std::cout << "  ";
        for (int v : grid[r]) std::cout << (v ? "# " : ". ");
        std::cout << "          ";
        for (int v : label[r])
            std::cout << (v ? std::string(1, static_cast<char>('A' + v - 1)) : ".") << " ";
        std::cout << "\n";
    }
    std::cout << "  -> " << CountFields(grid) << " fields\n";
}

}  // namespace

bool Run() {
    CHECK(CountFields({}) == 0);
    CHECK(CountFields({{}}) == 0);
    CHECK(CountFields({{0, 0}, {0, 0}}) == 0);
    CHECK(CountFields({{1}}) == 1);
    CHECK(CountFields({{1, 1}, {1, 1}}) == 1);
    // TRAP: diagonal neighbours are NOT connected.
    CHECK(CountFields({{1, 0}, {0, 1}}) == 2);
    CHECK(CountFields({{1, 0, 1}, {0, 1, 0}, {1, 0, 1}}) == 5);
    CHECK(CountFields({{1, 1, 0, 0, 0}, {1, 1, 0, 0, 1}, {0, 0, 1, 0, 1}, {0, 0, 0, 1, 1}}) == 3);
    // A winding single field (a classic stress shape).
    CHECK(CountFields({{1, 1, 1}, {0, 0, 1}, {1, 1, 1}, {1, 0, 0}, {1, 1, 1}}) == 1);
    // A million connected cells: recursion would overflow the stack here.
    CHECK(CountFields(Grid(1000, std::vector<int>(1000, 1))) == 1);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace count_fields
}  // namespace refresher
