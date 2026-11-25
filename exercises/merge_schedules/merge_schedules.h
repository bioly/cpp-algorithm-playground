#pragma once

// ============================================================================
// Exercise: Merge Schedules  -  "Merging the Planting Schedules"
// Pattern : intervals - sort, then sweep          Level: medium
// ============================================================================
//
// STORY
//   Crews book a field for a range of days: [start, end] (both days included).
//   Many bookings overlap. Produce one clean list of non-overlapping blocks
//   covering exactly the same days.
//
//        [8,10] [1,3] [2,6] [3,4] [6,7] [15,18] [17,20]
//                          |
//                          v
//             [1,7]   [8,10]   [15,20]
//
//   [1,3] and [3,5] share day 3, so they merge into [1,5].
//   [1,3] and [4,5] share no day, so they stay separate.
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     Do bookings that only touch merge? (yes)  Adjacent days like
//                   [1,3],[4,5]? (no)  Sorted input? (no)  Empty? (empty result)
//   2. Brute force. Compare every pair and merge until nothing changes:
//                   O(n^2) or worse, and fiddly.
//   3. Insight.     SORT by start day. Now any booking that can overlap the
//                   current block comes right after it, so each booking is only
//                   compared with the LAST block built:
//                     starts on/before the block's end -> extend the block
//                     starts after the block's end     -> a gap: new block
//   4. Complexity.  Time O(n log n): the sort dominates; the sweep is O(n).
//                   Space O(n) for the result.
//
// Run:  make run ONLY=merge_schedules V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace merge_schedules {

struct Interval {
    int start;
    int end;
};

// So tests can compare intervals with ==. `inline` lets a header define a
// function without duplicate-definition errors at link time.
inline bool operator==(const Interval& a, const Interval& b) {
    return a.start == b.start && a.end == b.end;
}

// The vector is taken BY VALUE on purpose: we sort our own copy, so the
// caller's data stays untouched. (The caller can std::move to avoid the copy.)
std::vector<Interval> MergeSchedules(std::vector<Interval> schedules);

bool Run();

}  // namespace merge_schedules
}  // namespace refresher
