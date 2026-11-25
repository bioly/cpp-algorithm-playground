#pragma once

// ============================================================================
// Exercise: Min Harvest Speed  -  "The Harvester Speed"
// Pattern : binary search on the answer           Level: medium
// ============================================================================
//
// STORY
//   Field i holds fields[i] kg of crop. A harvester works at k kg per hour and
//   handles ONE field per hour: if a field has less than k kg left, the rest of
//   that hour is wasted. The farmer picks k once and has `hours` hours before
//   the frost. What is the SMALLEST whole-number speed that finishes in time?
//
//        fields = [3, 6, 7, 11], hours = 8   ->   4
//        speed 4: 1 + 2 + 2 + 3 = 8 hours (fits)   speed 3: 1+2+3+4 = 10 (too slow)
//
// HOW THE INTERVIEW GOES
//   1. Clarify.     hours >= number of fields? (yes, else impossible)  Piles
//                   at least 1 kg? (yes)  How big? (up to 1e9: watch overflow)
//   2. Brute force. Try speed 1, 2, 3... and add up the hours each time. Far
//                   too slow when piles are around 1e9.
//   3. Insight.     If a speed is fast enough, EVERY faster speed is too (a
//                   faster harvester never does worse). So the answers look
//                   like  N N N N Y Y Y Y  and we want the FIRST Y. A yes/no
//                   test that flips once can be binary searched, even though
//                   there is no sorted array:
//                     speed: 1  2  3  4  5  6 ...
//                     ok?  : N  N  N  Y  Y  Y ...
//   4. Complexity.  Time O(n log M): about log2(M) guesses (M = biggest pile),
//                   each guess costs one O(n) pass. Space O(1).
//
// Run:  make run ONLY=min_harvest_speed V=1
// ============================================================================

#include <vector>

namespace refresher {
namespace min_harvest_speed {

// Preconditions: fields is not empty, every pile >= 1, hours >= fields.size().
int MinHarvestSpeed(const std::vector<int>& fields, int hours);

bool Run();

}  // namespace min_harvest_speed
}  // namespace refresher
