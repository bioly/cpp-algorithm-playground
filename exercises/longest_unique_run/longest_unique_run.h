#pragma once

// ============================================================================
// Exercise: Longest Unique Run  -  "The Farmer's Longest Clean Row"
// Pattern : sliding window (two pointers)          Level: medium
// ============================================================================
//
// STORY
//   The farmer plants one row of crops, one letter per plant. If the same crop
//   appears twice in a stretch, its pests spread between them. Find the longest
//   continuous stretch with NO crop repeated; return its length.
//
//        "abcbdea"  ->  5   (the stretch "cbdea")
//        "abcabcbb" ->  3        "aaaa" -> 1
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     Which characters? (plain bytes)  Empty row? (return 0)
//   2. Brute force. Check every start/end pair for repeats: O(n^2) with a set,
//                   O(n^3) naively. Too slow for long rows.
//   3. Insight.     A frame (window) sliding along the row. Rule: it never
//                   contains a repeat. Move the RIGHT edge one step at a time:
//                     - new crop not in the window  -> the window grows
//                     - new crop already inside      -> move the LEFT edge to
//                       just past the earlier copy
//                   The left edge only moves right, never back, so one pass is
//                   enough. A table last_seen[crop] gives that earlier position.
//   4. Complexity.  Time O(n): both edges only move forward.
//                   Space O(1): the table has a fixed 256 entries.
//
// Run:  make run ONLY=longest_unique_run V=1
// ============================================================================

#include <string_view>

namespace refresher {
namespace longest_unique_run {

// std::string_view (C++17) is a read-only view of text. It accepts a
// std::string, a string literal or a const char* WITHOUT copying.
int LongestUniqueRun(std::string_view row);

bool Run();

}  // namespace longest_unique_run
}  // namespace refresher
