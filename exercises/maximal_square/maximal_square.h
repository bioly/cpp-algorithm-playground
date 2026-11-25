#pragma once

// ============================================================================
// Exercise: Maximal Square  -  "The Farmer's Biggest Square Field"
// Pattern : dynamic programming on a grid         Level: medium
// ============================================================================
//
// STORY
//   The farmer's land is a grid: 1 = good land, 0 = bad land. He wants to farm
//   the BIGGEST SQUARE made only of good land. Return its area.
//
//        0 1 1 1 0
//        1 1 1 1 1     ->  biggest square is 3x3  ->  area 9
//        0 1 1 1 0     (a row of ones is a rectangle, not a square)
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     Return area or side? (area)  Empty grid? (return 0)
//   2. Brute force. Treat every cell as a top-left corner and grow the square
//                   while it is all ones. Re-checks the same cells again and
//                   again: roughly O(rows * cols * min(rows, cols)^2).
//   3. Insight.     Ask where a square ENDS (its bottom-right corner), and
//                   remember one number per cell:
//                     dp = side of the biggest all-ones square ending here.
//                   Cell is 0 -> dp = 0.  Cell is 1 -> look at three neighbours,
//                   take the SMALLEST and add 1:
//
//                         diagonal | above
//                         ---------+------
//                         left     | THIS CELL        dp = 1 + min(left, above, diagonal)
//
//                   Why the smallest? A 3x3 square here needs all three
//                   neighbours to hold a square of at least 2. The weakest
//                   neighbour decides.
//   4. Complexity.  Time O(rows * cols): each cell visited once.
//                   Space O(cols): we only need the previous row, so we keep
//                   one row and overwrite it.
//
// Run:  make run ONLY=maximal_square V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace maximal_square {

using Grid = std::vector<std::vector<int>>;  // alias: a shorter name for a long type

// Area of the largest square of 1s; 0 for an empty grid or a grid with no 1s.
int LargestSquareArea(const Grid& grid);

// Entry point called by the runner: runs the tests, returns true if all pass.
bool Run();

}  // namespace maximal_square
}  // namespace refresher
