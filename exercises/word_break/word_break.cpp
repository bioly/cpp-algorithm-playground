#include "word_break/word_break.h"

#include <iostream>
#include <random>

#include "check.h"

namespace refresher {
namespace word_break {

bool CanSplitIntoWords(const std::string& text, const std::vector<std::string>& dictionary) {
    // "One or more words": an empty text has nothing to cut, so the answer is no.
    if (text.empty()) return false;

    const size_t text_length = text.size();

    // CONTAINER: std::vector<char>, one yes/no per stone: 0 .. text_length.
    //   can_split_first[count] == 1  means  "the first `count` letters can be
    //   split into words". We use char (0/1) and NOT std::vector<bool>: that one
    //   is a special packed type with surprises (see docs/containers.md).
    //   `(n, 0)` builds n zeros in one go. The size is only known at run time,
    //   so a plain array or std::array would not do.
    //   Docs: https://en.cppreference.com/w/cpp/container/vector
    std::vector<char> can_split_first(text_length + 1, 0);
    can_split_first[0] = 1;  // zero letters: nothing left to split, so it counts as done

    // OUTER LOOP: one position (stone) at a time, left to right. When we get to
    // `letters_seen`, every smaller position is already decided.
    // C++ NOTE: size_t is an UNSIGNED integer type, the type of .size().
    for (size_t letters_seen = 1; letters_seen <= text_length; ++letters_seen) {
        // INNER LOOP: "could this word be the LAST piece of these letters?"
        // C++ NOTE: `const std::string&` in a range-for means "look at each word
        // in place". Without the & every word would be COPIED on every pass.
        for (const std::string& word : dictionary) {
            // Check 1: does the word fit? Do this BEFORE subtracting: size_t
            // cannot go negative, 3 - 4 would wrap to a gigantic number.
            if (word.size() > letters_seen) continue;

            const size_t word_start = letters_seen - word.size();  // where the word would begin

            // Check 2: is everything BEFORE the word already splittable?
            // (An empty word has word_start == letters_seen, whose entry is still
            // 0 at this moment, so empty words are skipped here for free.)
            if (!can_split_first[word_start]) continue;

            // Check 3: do the letters under the jump really spell the word?
            // compare(pos, len, other) compares text[pos, pos + len) with `other`
            // WITHOUT building a new string; it returns 0 when they are equal.
            // The expensive check comes last on purpose.
            // Docs: https://en.cppreference.com/w/cpp/string/basic_string/compare
            if (text.compare(word_start, word.size(), word) != 0) continue;

            can_split_first[letters_seen] = 1;  // a word ends here and what is before it is fine
            break;                              // one way is enough: next position
        }
    }
    return can_split_first[text_length] != 0;
}

// ----------------------------------------------------------------------------
// Tests and trace
// ----------------------------------------------------------------------------
namespace {

// The BRUTE FORCE (only for tests on tiny inputs): try every first word, then
// split the rest the same way. Same answer, exponential time.
bool BruteForce(const std::string& text, size_t from, const std::vector<std::string>& dictionary) {
    if (from == text.size()) return true;  // used up every letter
    for (const std::string& word : dictionary) {
        if (word.empty()) continue;  // an empty word would recurse forever
        // (compare() with a word that runs past the end is simply "not equal")
        if (text.compare(from, word.size(), word) == 0 &&
            BruteForce(text, from + word.size(), dictionary)) {
            return true;
        }
    }
    return false;
}

// Draws the three rows from the story: letters, positions, and the table so far.
void DrawTable(const std::string& text, const std::vector<char>& table, size_t known) {
    std::cout << "      letters:  ";
    for (char letter : text) std::cout << letter << "   ";
    std::cout << "\n      position: ";
    for (size_t i = 0; i <= text.size(); ++i) std::cout << i << "   ";
    std::cout << "\n      table:    ";
    for (size_t i = 0; i <= known; ++i) std::cout << (table[i] ? 'T' : '.') << "   ";
    std::cout << "\n";
}

// The same loops as CanSplitIntoWords, but narrating every check.
void Trace() {
    const std::string text = "leetcode";
    const std::vector<std::string> dictionary = {"leet", "code"};
    std::vector<char> can_split_first(text.size() + 1, 0);
    can_split_first[0] = 1;
    std::cout << "\n  text \"leetcode\", words: leet, code   (T = reachable stone, . = not)\n\n";
    DrawTable(text, can_split_first, 0);
    for (size_t letters_seen = 1; letters_seen <= text.size(); ++letters_seen) {
        std::cout << "\n  position " << letters_seen << ": letters so far \""
                  << text.substr(0, letters_seen) << "\"\n";
        for (const std::string& word : dictionary) {
            if (word.size() > letters_seen) {
                std::cout << "    \"" << word << "\": too long for these letters, skip\n";
                continue;
            }
            const size_t word_start = letters_seen - word.size();
            if (!can_split_first[word_start]) {
                std::cout << "    \"" << word << "\": would start at " << word_start
                          << ", but that stone is '.', no\n";
                continue;
            }
            if (text.compare(word_start, word.size(), word) != 0) {
                std::cout << "    \"" << word << "\": starts at " << word_start << " (T), but the letters there are \""
                          << text.substr(word_start, word.size()) << "\", no\n";
                continue;
            }
            std::cout << "    \"" << word << "\": starts at " << word_start
                      << " (T) and the letters match: YES -> stone " << letters_seen << " is T\n";
            can_split_first[letters_seen] = 1;
            break;
        }
        DrawTable(text, can_split_first, letters_seen);
    }
    std::cout << "\n  answer: " << (can_split_first[text.size()] ? "true" : "false") << "\n";
}

}  // namespace

bool Run() {
    // The three examples from the task.
    CHECK(CanSplitIntoWords("leetcode", {"leet", "code"}));
    CHECK(CanSplitIntoWords("applepenapple", {"apple", "pen"}));  // "apple" used twice
    CHECK(!CanSplitIntoWords("catsandog", {"cats", "dog", "sand", "and", "cat"}));

    // Edge cases.
    CHECK(!CanSplitIntoWords("", {"a"}));                // empty text: no words at all
    CHECK(!CanSplitIntoWords("abc", {}));                // empty dictionary
    CHECK(!CanSplitIntoWords("ab", {"abc"}));            // the only word is longer than the text
    CHECK(CanSplitIntoWords("a", {"a"}));                // text IS one word
    CHECK(CanSplitIntoWords("aaaa", {"a"}));             // same word again and again
    CHECK(CanSplitIntoWords("ab", {"", "a", "b"}));      // empty words are ignored
    CHECK(!CanSplitIntoWords("ab", {""}));               // ... and cannot split anything

    // TRAP: greedy (always take the longest word) gets stuck: "car" leaves "s".
    CHECK(CanSplitIntoWords("cars", {"car", "ca", "rs"}));  // the answer is "ca rs"
    CHECK(CanSplitIntoWords("goalspecial", {"go", "goal", "goals", "special"}));

    // TRAP: exponential recursion. These are instant with the table.
    CHECK(!CanSplitIntoWords(std::string(300, 'a') + "b", {"a", "aa", "aaa"}));
    CHECK(CanSplitIntoWords(std::string(300, 'a'), {"a", "aa", "aaa"}));

    // Cross-check against the brute force on many tiny random cases.
    // (Fixed seed: a failing case can be reproduced exactly. We use raw %
    // instead of std::uniform_int_distribution, whose results may differ
    // between standard libraries.)
    std::mt19937 rng(2024);
    int mismatches = 0;
    for (int round = 0; round < 3000; ++round) {
        std::vector<std::string> dictionary;
        const int word_count = 1 + static_cast<int>(rng() % 4);  // 1 .. 4 words
        for (int w = 0; w < word_count; ++w) {
            std::string word;
            const int letters = 1 + static_cast<int>(rng() % 3);  // 1 .. 3 letters
            for (int i = 0; i < letters; ++i) word += static_cast<char>('a' + rng() % 2);
            dictionary.push_back(word);
        }
        std::string text;
        const int text_letters = 1 + static_cast<int>(rng() % 10);  // 1 .. 10 letters
        for (int i = 0; i < text_letters; ++i) text += static_cast<char>('a' + rng() % 2);
        if (CanSplitIntoWords(text, dictionary) != BruteForce(text, 0, dictionary)) ++mismatches;
    }
    CHECK(mismatches == 0);

    if (Verbose()) Trace();
    return AllChecksPassed();
}

}  // namespace word_break
}  // namespace refresher
