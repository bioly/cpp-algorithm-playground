#include "house_robber_circle/house_robber_circle.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

#include "check.h"

namespace refresher {
namespace house_robber_circle {

namespace {

// C++ NOTE: an ITERATOR is a "pointer-like" position inside a container.
// begin() points at the first element, end() one PAST the last. A pair
// [first, last) describes a stretch of houses without copying them into a new
// vector. That is how every <algorithm> function takes its input too.
using Houses = std::vector<int>::const_iterator;  // alias: shorter name for a long type

// The EASY problem: houses in a straight line, houses [first, last).
//
// We walk once and remember only two numbers:
//   two_back = best total using houses up to i-2
//   one_back = best total using houses up to i-1
// At house i with `money` there are exactly two choices:
//   skip it -> keep one_back
//   rob it  -> money + two_back  (we may not touch house i-1)
// and we take the larger. Then both memories slide forward by one house.
//
// `long long`: sums of many large amounts can pass what an int can hold.
long long LinearLoot(Houses first, Houses last) {
    long long two_back = 0;  // nothing before the first house
    long long one_back = 0;
    for (Houses it = first; it != last; ++it) {
        // C++ NOTE: *it reads the element the iterator points at. Adding an int
        // to a long long first converts the int, so there is no overflow here.
        const long long robbed = two_back + *it;
        const long long best_now = std::max(one_back, robbed);  // skip vs rob
        two_back = one_back;  // slide the window: i-1 becomes "two back" ...
        one_back = best_now;  // ... and the new answer becomes "one back"
    }
    return one_back;  // best for the whole stretch (0 for an empty stretch)
}

}  // namespace

long long MaxLoot(const std::vector<int>& nums) {
    if (nums.empty()) return 0;
    // TRAP: with ONE house the two worlds below would be the ranges
    // [begin + 1, end) and [begin, end - 1): the first starts AFTER the end of
    // the second, and passing such a backwards range is undefined behaviour.
    // A lone house has no neighbour, so just take it.
    if (nums.size() == 1) return nums[0];

    // World A: skip the LAST house, so the street is houses 0 .. n-2.
    const long long skip_last = LinearLoot(nums.begin(), nums.end() - 1);
    // World B: skip the FIRST house, so the street is houses 1 .. n-1.
    const long long skip_first = LinearLoot(nums.begin() + 1, nums.end());

    // Houses 0 and n-1 touch, so no valid plan uses both: every plan fits in
    // world A, world B, or both. The better of the two is the answer.
    // (Two houses: A = house 0 alone, B = house 1 alone: the richer one.)
    return std::max(skip_last, skip_first);
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// The BRUTE FORCE, used only to double-check the fast solution on small inputs.
// C++ NOTE: a "bitmask" stores a subset in one integer: bit i set = house i
// robbed. 1u << n is 2^n, so the loop visits every subset exactly once.
long long BruteForce(const std::vector<int>& nums) {
    const unsigned n = static_cast<unsigned>(nums.size());
    long long best = 0;
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        // mask & (mask >> 1) has a bit set exactly where two NEIGHBOURING
        // houses are both chosen (bit i and bit i+1).
        bool neighbours = (mask & (mask >> 1)) != 0;
        // The circle: house n-1 and house 0 are neighbours too. (n > 1: a lone
        // house is not its own neighbour.)
        if (n > 1 && (mask & 1u) && ((mask >> (n - 1)) & 1u)) neighbours = true;
        if (neighbours) continue;  // police! this subset is not allowed
        long long total = 0;
        for (unsigned i = 0; i < n; ++i) {
            if (mask & (1u << i)) total += nums[i];
        }
        best = std::max(best, total);
    }
    return best;
}

// ---- Terminal drawing of the algorithm (only used with V=1) ----------------
//   X = robbed in that plan     . = not robbed     - = house is crossed out
//   (not part of this world)    blank = not looked at yet

// One row of glyphs: every house gets a 6-character cell, and the glyph sits at
// position 4 of the cell: right under the last digit of the money box "[  2] ".
std::string PlanRow(const std::vector<int>& plan, size_t n, size_t from, size_t to, size_t reach) {
    std::string row;
    for (size_t c = 0; c < n; ++c) {
        char glyph = ' ';
        if (c < from || c >= to) {
            glyph = '-';  // outside this world: pretend the house does not exist
        } else if (c <= reach) {
            const bool robbed = std::find(plan.begin(), plan.end(), static_cast<int>(c)) != plan.end();
            glyph = robbed ? 'X' : '.';
        }
        row += std::string("    ") + glyph + " ";
    }
    return row;
}

struct World {
    long long total;
    std::vector<int> plan;  // the houses robbed, e.g. {0, 2}
};

// The same loop as LinearLoot, but it also remembers WHICH houses each notebook
// stands for, and draws every step.
World TraceWorld(const char* title, const std::vector<int>& nums, size_t from, size_t to) {
    const size_t n = nums.size();
    std::cout << "\n  " << title << "\n";
    long long two_back = 0, one_back = 0;
    std::vector<int> two_plan, one_plan;  // the plans behind the two notebooks
    for (size_t i = from; i < to; ++i) {
        const long long robbed = two_back + nums[i];
        std::vector<int> robbed_plan = two_plan;  // rob = the plan from TWO back ...
        robbed_plan.push_back(static_cast<int>(i));  // ... plus this house
        const bool rob = robbed > one_back;

        std::string money, index, now;
        for (size_t c = 0; c < n; ++c) {
            std::ostringstream box;
            box << "[" << std::setw(3) << nums[c] << "] ";
            money += box.str();
            index += "    " + std::to_string(c) + " ";
            now += std::string("    ") + (c == i ? 'v' : ' ') + " ";
        }
        std::cout << "\n  step " << (i - from + 1) << ": house " << i << " (money " << nums[i] << ")\n"
                  << "     house  " << index << "\n"
                  << "     money  " << money << "\n"
                  << "     now    " << now << "\n"
                  << "     skip=" << std::setw(2) << one_back << PlanRow(one_plan, n, from, to, i)
                  << (rob ? "" : "  <== wins") << "\n"
                  << "     rob =" << std::setw(2) << robbed << PlanRow(robbed_plan, n, from, to, i)
                  << (rob ? "  <== wins" : "") << "\n";
        two_back = one_back;
        two_plan = one_plan;
        one_back = rob ? robbed : one_back;
        one_plan = rob ? robbed_plan : one_plan;
    }
    std::cout << "\n     RESULT " << std::setw(2) << one_back << PlanRow(one_plan, n, from, to, to - 1) << "\n";
    return {one_back, one_plan};
}

void Trace() {
    const std::vector<int> nums = {2, 7, 9, 3, 1};
    std::cout << "\n  circle of 5 houses: 2 7 9 3 1   (house 4 touches house 0)\n"
              << "  legend: X robbed in this plan, . not robbed, - house crossed out, blank = not reached\n";
    std::cout << "  a straight street alone would answer " << LinearLoot(nums.begin(), nums.end())
              << " = houses {0,2,4}: wrong, 0 and 4 touch!\n";
    const World a = TraceWorld("WORLD A: skip the LAST house (house 4 crossed out)", nums, 0, nums.size() - 1);
    const World b = TraceWorld("WORLD B: skip the FIRST house (house 0 crossed out)", nums, 1, nums.size());
    std::cout << "\n  answer = max(" << a.total << ", " << b.total << ") = " << MaxLoot(nums) << "\n";
}

}  // namespace

bool Run() {
    CHECK(MaxLoot({}) == 0);
    CHECK(MaxLoot({5}) == 5);                 // one house: no neighbour
    CHECK(MaxLoot({2, 3}) == 3);              // two houses touch: take the richer
    // TRAP: a straight street would rob both 2s (total 4), but house 0 and
    // house 2 are neighbours on the circle.
    CHECK(MaxLoot({2, 3, 2}) == 3);
    CHECK(MaxLoot({1, 2, 3, 1}) == 4);        // houses 0 and 2
    CHECK(MaxLoot({1, 2, 3}) == 3);
    CHECK(MaxLoot({6, 1, 1, 7}) == 8);        // 7 + 1, not 6 + 7
    CHECK(MaxLoot({200, 3, 140, 20, 10}) == 340);
    CHECK(MaxLoot({0, 0, 0}) == 0);
    CHECK(MaxLoot({5, 5, 5, 5}) == 10);       // two opposite houses
    CHECK(MaxLoot({1, 100, 1, 1, 100, 1}) == 200);  // the two 100s are not neighbours

    // Overflow: 100000 houses of 1e9 each; the best plan takes every other
    // house, 50000 * 1e9 = 5e13, which does not fit in an int.
    CHECK(MaxLoot(std::vector<int>(100000, 1000000000)) == 50000LL * 1000000000LL);

    // Cross-check against the brute force on many small random streets.
    // C++ NOTE: std::mt19937 is a standard random generator. With a FIXED seed
    // it produces the same numbers on every machine, so a failing case can be
    // reproduced exactly. (We use raw values with %, not std::uniform_int_
    // distribution, whose results may differ between standard libraries.)
    std::mt19937 rng(12345);
    int mismatches = 0;
    for (int round = 0; round < 2000; ++round) {
        const int n = 1 + static_cast<int>(rng() % 12);  // 1 .. 12 houses
        std::vector<int> nums(n);
        for (int& money : nums) money = static_cast<int>(rng() % 50);  // 0 .. 49
        if (MaxLoot(nums) != BruteForce(nums)) ++mismatches;
    }
    CHECK(mismatches == 0);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace house_robber_circle
}  // namespace refresher
