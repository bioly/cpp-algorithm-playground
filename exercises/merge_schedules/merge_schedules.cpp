#include "merge_schedules/merge_schedules.h"

#include <algorithm>
#include <iostream>

#include "check.h"

namespace refresher {
namespace merge_schedules {

std::vector<Interval> MergeSchedules(std::vector<Interval> schedules) {
    // std::sort needs to know HOW to order two intervals. A lambda is a small
    // function written right where it is used: `[]` captures nothing, the
    // parentheses are its parameters, the braces its body.
    std::sort(schedules.begin(), schedules.end(),
              [](const Interval& a, const Interval& b) { return a.start < b.start; });

    std::vector<Interval> merged;
    for (const Interval& cur : schedules) {
        // `<=` (not `<`): sharing a single day counts as overlapping.
        // That is a one-character decision worth saying out loud.
        if (!merged.empty() && cur.start <= merged.back().end) {
            // std::max matters: `cur` may sit fully INSIDE the block (like
            // [2,3] inside [1,10]). Writing `end = cur.end` would shrink it.
            merged.back().end = std::max(merged.back().end, cur.end);
        } else {
            merged.push_back(cur);  // a gap (or the first one): start a new block
        }
    }
    return merged;
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Same sweep with prints.
void Trace() {
    std::vector<Interval> v = {{8, 10}, {1, 3}, {2, 6}, {3, 4}, {6, 7}, {15, 18}, {17, 20}};
    std::sort(v.begin(), v.end(),
              [](const Interval& a, const Interval& b) { return a.start < b.start; });
    std::vector<Interval> merged;
    for (const Interval& cur : v) {
        std::cout << "  next [" << cur.start << "," << cur.end << "]  ";
        if (!merged.empty() && cur.start <= merged.back().end) {
            std::cout << cur.start << " <= " << merged.back().end << " -> overlap: ";
            merged.back().end = std::max(merged.back().end, cur.end);
        } else {
            std::cout << "gap or first -> new block: ";
            merged.push_back(cur);
        }
        for (const Interval& m : merged) std::cout << "[" << m.start << "," << m.end << "] ";
        std::cout << "\n";
    }
}

}  // namespace

bool Run() {
    using V = std::vector<Interval>;  // a local alias keeps the tests short

    CHECK(MergeSchedules({}).empty());
    CHECK(MergeSchedules({{1, 4}}) == (V{{1, 4}}));
    CHECK(MergeSchedules({{1, 3}, {2, 6}, {8, 10}, {15, 18}}) == (V{{1, 6}, {8, 10}, {15, 18}}));
    CHECK(MergeSchedules({{1, 3}, {3, 5}}) == (V{{1, 5}}));          // touching merges
    CHECK(MergeSchedules({{1, 2}, {3, 4}}) == (V{{1, 2}, {3, 4}}));  // adjacent days do not
    // TRAP: a nested interval must not shrink the block.
    CHECK(MergeSchedules({{1, 10}, {2, 3}, {4, 5}}) == (V{{1, 10}}));
    CHECK(MergeSchedules({{5, 6}, {1, 2}, {3, 4}}) == (V{{1, 2}, {3, 4}, {5, 6}}));  // unsorted
    CHECK(MergeSchedules({{2, 3}, {2, 3}, {2, 3}}) == (V{{2, 3}}));                  // duplicates
    CHECK(MergeSchedules({{8, 10}, {1, 3}, {2, 6}, {3, 4}, {6, 7}, {15, 18}, {17, 20}}) ==
          (V{{1, 7}, {8, 10}, {15, 20}}));

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace merge_schedules
}  // namespace refresher
