#pragma once

// A tiny test helper, so the project needs no test library.
//
// Why not <cassert>? assert() vanishes when NDEBUG is defined and aborts at
// the first failure. CHECK counts every check, keeps going after a failure
// and prints the file, line and the expression, so one run shows everything.
//
//   CHECK(x == y);     passes when the condition is true
#include <iostream>

namespace refresher {

// C++17 `inline` variables: a header may DEFINE a global and every .cpp that
// includes it shares one instance. (Before C++17 this needed a separate .cpp
// file or `extern`, otherwise the linker reports duplicate definitions.)
inline int g_checks = 0;
inline int g_failures = 0;
inline bool g_verbose = false;

inline bool Verbose() {
    return g_verbose;
}  // true with `make run V=1`
inline bool AllChecksPassed() {
    return g_failures == 0;
}

}  // namespace refresher

// A macro is the right tool here: only a macro can print the TEXT of the
// expression (#cond) and the file/line. `do { } while (0)` makes the macro
// behave like one statement, so it is safe inside an if/else without braces.
#define CHECK(cond)                                                                              \
    do {                                                                                         \
        ++refresher::g_checks;                                                                   \
        if (!(cond)) {                                                                           \
            ++refresher::g_failures;                                                             \
            std::cerr << "  CHECK FAILED " << __FILE__ << ":" << __LINE__ << "  " #cond << "\n"; \
        }                                                                                        \
    } while (0)
