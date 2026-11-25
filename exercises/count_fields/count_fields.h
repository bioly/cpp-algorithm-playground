#pragma once

// ============================================================================
// Exercise: Count Fields  -  "How Many Separate Fields Does the Farmer Own?"
// Pattern : flood fill (grid DFS), connected components    Level: medium
// ============================================================================
//
// STORY
//   Same map: 1 = good land, 0 = bad land. A FIELD is a group of 1s connected
//   up, down, left or right (NOT diagonally). How many separate fields?
//
//        1 1 0 0 0
//        1 1 0 0 1     ->  3 fields: the 2x2 block, the group on the right,
//        0 0 1 0 1         and the lone cell in the middle (it touches the
//        0 0 0 1 1         right group only diagonally, so it is its own field)
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     May I change the input? (no, so I keep a separate "painted"
//                   map)  How big is the grid? (big: avoid deep recursion)
//   2. Brute force. None that is simpler; this problem IS "find the groups".
//   3. Insight.     A surveyor with a paint bucket. Walk the map; when you meet
//                   UNPAINTED land, that is a new field: count it, then paint
//                   the whole field by spreading to connected land. Land that is
//                   already painted belongs to a field already counted.
//   4. Complexity.  Time O(rows * cols): every cell is painted at most once and
//                   checks 4 neighbours. Space O(rows * cols) for the paint map
//                   and the stack.
//
// Run:  make run ONLY=count_fields V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace count_fields {

// CONTAINER: vector of rows (see maximal_square for the trade-off vs one flat vector).
using Grid = std::vector<std::vector<int>>;

// Returns a map the same size as `grid`: 0 for water, otherwise the number
// (1, 2, 3...) of the field that cell belongs to.
Grid LabelFields(const Grid& grid);

// Number of separate fields.
int CountFields(const Grid& grid);

bool Run();

}  // namespace count_fields
}  // namespace refresher
