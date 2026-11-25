#include "longest_unique_run/longest_unique_run.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "check.h"

namespace refresher {
namespace longest_unique_run {

int LongestUniqueRun(std::string_view row) {
    // last_seen[crop] = index where this crop appeared most recently, or -1.
    // A byte has only 256 values, so a plain vector is enough: lookup is just
    // an array index (no hashing), unlike std::unordered_map.
    //
    // CONTAINER: a lookup table indexed by the character's value (0..255).
    //   Why not std::unordered_map<char, int>: it hashes and allocates nodes;
    //   a direct array index is much cheaper when the keys are a small fixed
    //   range. This trick is called a "direct-address table".
    //   Why not std::map: O(log n) and sorted order we do not need.
    //   Even better: std::array<int, 256>. Its size is a compile-time constant,
    //   so it lives on the stack with no heap allocation. We keep the vector
    //   because `(256, -1)` fills it in one readable line.
    //   Compare all containers: docs/containers.md
    //   Docs: https://en.cppreference.com/w/cpp/container/vector
    //         https://en.cppreference.com/w/cpp/container/array
    std::vector<int> last_seen(256, -1);

    int left = 0;  // the window is row[left .. right] and never has a repeat
    int best = 0;

    for (int right = 0; right < static_cast<int>(row.size()); ++right) {
        // Plain `char` can be negative (bytes >= 128), which would index
        // before the start of the vector. Casting to unsigned char fixes it.
        const auto crop = static_cast<unsigned char>(row[right]);

        // A repeat INSIDE the window? Only then must the window shrink.
        // `>= left` matters: an older sighting before `left` is already
        // outside the window and must be ignored. Without it `left` could
        // move backwards (try "abba").
        if (last_seen[crop] >= left) left = last_seen[crop] + 1;

        last_seen[crop] = right;
        best = std::max(best, right - left + 1);
    }
    return best;
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// Same algorithm with prints. ^ marks the window, R the right edge.
void Trace() {
    const std::string row = "abcbdea";
    std::vector<int> last_seen(256, -1);
    int left = 0;
    for (int right = 0; right < static_cast<int>(row.size()); ++right) {
        const auto crop = static_cast<unsigned char>(row[right]);
        std::string note = "grows";
        if (last_seen[crop] >= left) {
            left = last_seen[crop] + 1;
            note = "repeat inside window: left jumps to " + std::to_string(left);
        } else if (last_seen[crop] >= 0) {
            note = "seen before, but outside the window: ignore";
        }
        last_seen[crop] = right;
        std::cout << "  " << row << "   ";
        for (int i = 0; i < static_cast<int>(row.size()); ++i)
            std::cout << (i == right ? 'R' : (i >= left && i < right ? '^' : ' '));
        std::cout << "  window \"" << row.substr(left, right - left + 1) << "\"  " << note << "\n";
    }
}

}  // namespace

bool Run() {
    CHECK(LongestUniqueRun("") == 0);
    CHECK(LongestUniqueRun("a") == 1);
    CHECK(LongestUniqueRun("aaaa") == 1);
    CHECK(LongestUniqueRun("abcdef") == 6);
    CHECK(LongestUniqueRun("abcabcbb") == 3);
    CHECK(LongestUniqueRun("pwwkew") == 3);
    // TRAP: the earlier 'a' is outside the window, so it is not a repeat.
    CHECK(LongestUniqueRun("abcbdea") == 5);
    // TRAP: left must never move backwards.
    CHECK(LongestUniqueRun("abba") == 2);
    CHECK(LongestUniqueRun("\xC3\xA9\xC3\xA8") == 3);  // bytes >= 128
    CHECK(LongestUniqueRun(std::string(100000, 'x')) == 1);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace longest_unique_run
}  // namespace refresher
