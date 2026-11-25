#pragma once

// ============================================================================
// Exercise: Busiest Fields  -  "The K Busiest Fields"
// Pattern : heap (priority queue) of size K       Level: medium
// ============================================================================
//
// STORY
//   Each time a truck leaves a field loaded with crop, the office writes down
//   the field's id. After a week: 3 7 3 5 3 7 9 3 7 5 3 2 7 3. Which K fields
//   sent out the most loads?
//
//        reports = [3,7,3,5,3,7,9,3,7,5,3,2,7,3], k = 2  ->  [3, 7]
//        (field 3: 6 loads, field 7: 4 loads). Ties: the smaller id wins.
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     Ties? (smaller id)  Order? (busiest first)  k bigger than
//                   the number of fields? (return them all)  k <= 0? (empty)
//   2. Brute force. Count loads per field, SORT all fields, take the first k:
//                   O(m log m) for m fields. Fine for small data, but it sorts
//                   everything just to keep k items.
//   3. Insight.     A bouncer with K seats. Keep a heap of at most K fields
//                   with the WORST of them on top. Each new field is pushed; if
//                   the club now has K+1, the worst leaves. After all fields
//                   have arrived, the K inside are the K best.
//                   (A heap = fast access to its extreme element: peek O(1),
//                   push and pop O(log size). std::priority_queue is one.)
//   4. Complexity.  Time O(n + m log k): counting is O(n); each of m heap
//                   operations costs O(log k) because the heap never exceeds
//                   k+1 items. Space O(m + k).
//
// Run:  make run ONLY=busiest_fields V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace busiest_fields {

// Ids of the k fields with the most loads, busiest first (ties: smaller id).
std::vector<int> BusiestFields(const std::vector<int>& reports, int k);

bool Run();

}  // namespace busiest_fields
}  // namespace refresher
