#pragma once

// ============================================================================
// Exercise: Word Break  -  "Can the search box text be cut into known words?"
// Pattern : dynamic programming over positions ("stepping stones")  Level: medium
// ============================================================================
//
// STORY
//   A customer types "lordoftherings" into the bookstore's search box, with no
//   spaces. We have a list of known words ("lord", "of", "the", "rings", ...).
//   Can the text be cut into pieces so that EVERY piece is a known word?
//   A word may be used any number of times.
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Return only true/false, or the actual split? (true/false.)
//                 May a word repeat? (yes.)  Empty text? (we say false: the
//                 task asks for ONE OR MORE words.)  Empty words in the list?
//                 (ignore them.)  How big are the inputs? (decides the approach.)
//   2. Brute      Cut off a first word, then try to split the rest; if that
//      force.     fails, undo and try another first word. Correct, but it asks
//                 the SAME question again and again: on "aaaa...ab" with the
//                 words a, aa, aaa, 25 letters cost over 5 million calls.
//   3. Insight.   Only the POSITION matters, not how we got there. Think of
//                 stepping stones across a river: stone 0 is the start, every
//                 word is a jump as long as the word, and a jump only counts if
//                 the letters under it spell that word. Can we reach the last
//                 stone? Keep one yes/no per stone:
//
//                  letters:    l   e   e   t   c   o   d   e
//                  position:  0   1   2   3   4   5   6   7   8
//                  table:     T   .   .   .   T   .   .   .   T
//                             └──── leet ────┘└──── code ────┘
//
//                 Rule: stone `i` is reachable if SOME word ends exactly at `i`
//                 and the stone where that word STARTS is already reachable.
//                 Go left to right, so every earlier stone is already decided.
//   4. Complexity. n = letters in the text, W = words, L = longest word.
//                 Time  O(n * W * L): per position, per word, one comparison.
//                 Space O(n): one yes/no per stone.
//
// TRAPS (all covered by tests)
//   * The start stone: table[0] must be true, or nothing ever becomes true.
//   * The table has n + 1 entries (stones sit BETWEEN letters), not n.
//   * Unsigned subtraction: `letters_seen - word.size()` wraps around to a huge
//     number when the word is longer than the text seen. Check the size FIRST.
//   * Greedy fails: for "cars" with car, ca, rs, taking the longest first word
//     ("car") leaves "s" and gets stuck; the right split is "ca rs".
//   * Exponential recursion on long inputs (the table version is instant).
//
// Run:  make run ONLY=word_break V=1
// ============================================================================

#include <string>
#include <vector>

namespace refresher {
namespace word_break {

// True if `text` can be cut into one or more words from `dictionary`.
bool CanSplitIntoWords(const std::string& text, const std::vector<std::string>& dictionary);

bool Run();

}  // namespace word_break
}  // namespace refresher
