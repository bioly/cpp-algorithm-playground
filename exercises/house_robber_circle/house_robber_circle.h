#pragma once

// ============================================================================
// Exercise: Robbing a Circular Street  -  "House Robber II"
// Pattern : dynamic programming with two variables, run twice   Level: medium
// ============================================================================
//
// STORY
//   A robber plans one night on a street whose houses form a CIRCLE: the first
//   house is the neighbour of the last one. Each house holds some money. Two
//   neighbouring houses share an alarm: robbing both on the same night calls
//   the police. What is the most money that can be taken without that?
//
//        [0]---[1]            houses 0..5 around a ring;
//       /        \            0 and 5 are neighbours too,
//     [5]        [2]          so "the last house" is NOT the end of the road.
//       \        /
//        [4]---[3]
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Can amounts be negative? (assume not: 0 or more.)  Empty
//                 street? (answer 0.)  One house? (no neighbour: rob it.)
//                 Two houses? (they are neighbours: take the richer one.)
//                 Do we need WHICH houses, or only the total? (only the total.)
//                 How big can the total get? (use long long to be safe.)
//   2. Brute      Try every subset of houses (2^n), throw away the ones that
//      force.     contain two neighbours, keep the best. Correct but hopeless
//                 for n = 100: 2^100 subsets. (We use it in the tests, for n <= 12,
//                 to PROVE the fast solution right.)
//   3. Insight.   First solve the easy version, a straight street (no circle):
//                   at each house either SKIP it (keep the best so far) or ROB
//                   it (its money + the best from two houses back).
//                   best(i) = max( best(i-1),  best(i-2) + money[i] )
//                 That needs only the last two answers, so two variables.
//                 Now the circle. House 0 and house n-1 touch, so the robber
//                 can NEVER take both. Therefore at least one of them is
//                 skipped, and we can try both worlds and take the better:
//
//                   world A: skip the LAST house   -> straight street 0 .. n-2
//                   world B: skip the FIRST house  -> straight street 1 .. n-1
//
//                 Any valid plan fits in at least one world, and each world is
//                 an ordinary straight street. Answer = max(world A, world B).
//   4. Complexity. Time O(n): two passes over the houses. Space O(1): the
//                 two-variable trick needs no table. (Plain DP would use an
//                 O(n) array; we keep only what the next step reads.)
//
// TRAPS (all covered by tests)
//   * Solving the straight street once and returning that: wrong for {2,3,2}
//     (it takes both 2s, which are neighbours on the circle).
//   * One house: both "worlds" would be empty or out of range. Handle it first.
//   * Overflow: 100000 houses with 1e9 each exceed what an int can hold.
//
// Run:  make run ONLY=house_robber_circle V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace house_robber_circle {

// Most money that can be taken from houses arranged in a circle. `nums[i]` is
// the money in house i (assumed >= 0). Empty input gives 0.
long long MaxLoot(const std::vector<int>& nums);

bool Run();

}  // namespace house_robber_circle
}  // namespace refresher
