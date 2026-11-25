#include "min_harvest_speed/min_harvest_speed.h"

#include <algorithm>
#include <iostream>

#include "check.h"

namespace refresher {
namespace min_harvest_speed {

namespace {  // an anonymous namespace: these helpers are private to this file

// Total hours needed at this speed. One field costs ceil(pile / speed) hours.
// `long long`: with speed 1 and 100000 piles of 1e9 the total is 1e14, far
// beyond what an int can hold.
long long HoursNeeded(const std::vector<int>& fields, int speed) {
    long long total = 0;
    for (int pile : fields) {
        // Integer ceiling division: (a + b - 1) / b rounds up without floats.
        // The cast to long long first prevents pile + speed from overflowing int.
        total += (static_cast<long long>(pile) + speed - 1) / speed;
    }
    return total;
}

}  // namespace

int MinHarvestSpeed(const std::vector<int>& fields, int hours) {
    int lo = 1;                                                // slowest sensible speed
    // CONTAINER: std::vector<int>, scanned front to back by HoursNeeded. A
    // contiguous block of ints is the fastest thing to scan (the CPU prefetches
    // the next cells). The binary search below is NOT over the container: it
    // searches the range of possible ANSWERS 1..max. (If you ever search a
    // SORTED container, use std::lower_bound; it is O(log n) on a vector but
    // O(n) steps on a std::list, because it must walk node by node.)
    // max_element returns an ITERATOR, so we dereference it with `*`.
    // Compare all containers: docs/containers.md
    // Docs: https://en.cppreference.com/w/cpp/algorithm/max_element
    //       https://en.cppreference.com/w/cpp/algorithm/lower_bound
    int hi = *std::max_element(fields.begin(), fields.end());  // always enough: 1 hour/field

    // Invariant: the answer is always inside [lo, hi].
    while (lo < hi) {
        // lo + (hi - lo) / 2 equals (lo + hi) / 2 but cannot overflow int.
        const int mid = lo + (hi - lo) / 2;
        if (HoursNeeded(fields, mid) <= hours) {
            hi = mid;  // mid works; it MIGHT be the answer, so keep it (no -1)
        } else {
            lo = mid + 1;  // mid is too slow, and so is everything below it
        }
    }
    return lo;  // lo == hi: the first speed that works
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Same search with prints.
void Trace() {
    const std::vector<int> fields = {3, 6, 7, 11};
    const int hours = 8;
    int lo = 1, hi = 11;
    while (lo < hi) {
        const int mid = lo + (hi - lo) / 2;
        const long long needed = HoursNeeded(fields, mid);
        std::cout << "  lo=" << lo << " hi=" << hi << " mid=" << mid << "  needs " << needed
                  << " hours -> "
                  << (needed <= hours ? "works, hi = mid" : "too slow, lo = mid + 1") << "\n";
        if (needed <= hours)
            hi = mid;
        else
            lo = mid + 1;
    }
    std::cout << "  answer: " << lo << " kg/hour\n";
}

}  // namespace

bool Run() {
    CHECK(MinHarvestSpeed({3, 6, 7, 11}, 8) == 4);
    CHECK(MinHarvestSpeed({30, 11, 23, 4, 20}, 5) == 30);  // hours == fields: biggest pile
    CHECK(MinHarvestSpeed({30, 11, 23, 4, 20}, 6) == 23);
    CHECK(MinHarvestSpeed({7}, 7) == 1);
    CHECK(MinHarvestSpeed({7}, 1) == 7);
    CHECK(MinHarvestSpeed({1, 1, 1, 1}, 100) == 1);
    CHECK(MinHarvestSpeed({1000000000, 1000000000}, 2) == 1000000000);
    // Overflow check: each field may take at most 1e9 / 1e5 = 1e4 hours, so
    // the speed must be at least 1e9 / 1e4 = 100000.
    CHECK(MinHarvestSpeed(std::vector<int>(100000, 1000000000), 1000000000) == 100000);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace min_harvest_speed
}  // namespace refresher
