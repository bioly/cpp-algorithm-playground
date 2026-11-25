#include "maximal_square/maximal_square.h"

#include <algorithm>
#include <cstddef>
#include <iostream>

#include "check.h"

namespace refresher {
namespace maximal_square {

int LargestSquareArea(const Grid& grid) {
    // `const Grid&`: read the grid without copying it, and promise not to change it.
    if (grid.empty() || grid[0].empty()) return 0;  // also protects grid[0] below

    // std::size_t is the type vectors use for sizes and indices (unsigned).
    const std::size_t rows = grid.size();
    const std::size_t cols = grid[0].size();

    // dp[c + 1] = side of the biggest square ending at column c of the current
    // row. Slot 0 is a permanent 0 meaning "left of the first column", so we
    // never need an `if (c == 0)` special case.
    std::vector<int> dp(cols + 1, 0);
    int best_side = 0;

    for (std::size_t r = 0; r < rows; ++r) {
        int diagonal = 0;  // dp of the up-left neighbour; nothing at row start
        for (std::size_t c = 0; c < cols; ++c) {
            // dp[c + 1] still holds the PREVIOUS row's value here, so it is
            // "above". Save it before overwriting: it becomes the next cell's
            // diagonal.
            const int above = dp[c + 1];
            if (grid[r][c] == 1) {
                // dp[c] is "left": already updated in this row.
                // std::min with braces {a, b, c} takes the minimum of a list (C++11).
                dp[c + 1] = 1 + std::min({dp[c], above, diagonal});
                best_side = std::max(best_side, dp[c + 1]);
            } else {
                dp[c + 1] = 0;
            }
            diagonal = above;
        }
    }
    return best_side * best_side;  // the question asks for the area
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Same algorithm with prints: shows the one-row memory after each row.
void Trace() {
    const Grid grid = {{0, 1, 1, 1, 0}, {1, 1, 1, 1, 1}, {0, 1, 1, 1, 0}};
    std::vector<int> dp(grid[0].size() + 1, 0);
    for (std::size_t r = 0; r < grid.size(); ++r) {
        int diagonal = 0;
        for (std::size_t c = 0; c < grid[0].size(); ++c) {
            const int above = dp[c + 1];
            dp[c + 1] = grid[r][c] == 1 ? 1 + std::min({dp[c], above, diagonal}) : 0;
            diagonal = above;
        }
        std::cout << "  after row " << r << ":  grid ";
        for (int v : grid[r]) std::cout << v << " ";
        std::cout << "  memory ";
        for (std::size_t c = 1; c < dp.size(); ++c) std::cout << dp[c] << " ";
        std::cout << "\n";
    }
    std::cout << "  biggest number in memory was 3 -> side 3 -> area 9\n";
}

}  // namespace

bool Run() {
    CHECK(LargestSquareArea({}) == 0);
    CHECK(LargestSquareArea({{}}) == 0);
    CHECK(LargestSquareArea({{0, 0}, {0, 0}}) == 0);
    CHECK(LargestSquareArea({{1}}) == 1);
    // TRAP: a row of ones is a rectangle, not a square.
    CHECK(LargestSquareArea({{0, 1, 1, 1, 0}}) == 1);
    CHECK(LargestSquareArea(Grid(3, std::vector<int>(5, 1))) == 9);  // limited by the short side
    CHECK(LargestSquareArea({{0, 1, 1, 1, 0}, {1, 1, 1, 1, 1}, {0, 1, 1, 1, 0}}) == 9);
    CHECK(LargestSquareArea({{1, 1, 1}, {1, 1, 1}, {1, 1, 0}}) == 4);  // one bad corner
    CHECK(LargestSquareArea({{1, 1, 1, 1}, {1, 0, 1, 1}, {1, 1, 1, 1}, {1, 1, 1, 1}}) == 4);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace maximal_square
}  // namespace refresher
