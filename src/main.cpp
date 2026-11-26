// The runner: a TABLE of exercises and a loop that calls each one.
//
// To skip an exercise, comment out its row below. To add one, add its
// #include and a row. The Makefile finds the new .cpp file by itself.
#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "check.h"

// ---- 1. Exercise headers ----------------------------------------------------
#include "busiest_fields/busiest_fields.h"
#include "count_fields/count_fields.h"
#include "longest_unique_run/longest_unique_run.h"
#include "lru_cache/lru_cache.h"
#include "maximal_square/maximal_square.h"
#include "merge_schedules/merge_schedules.h"
#include "min_harvest_speed/min_harvest_speed.h"
#include "rate_limiter/rate_limiter.h"
#include "consistent_hashing/consistent_hashing.h" 
#include "house_robber_circle/house_robber_circle.h" 
#include "max_depth_tree/max_depth_tree.h"
#include "max_path_sum/max_path_sum.h"  

// ---- 2. The table. Study order = row order. ---------------------------------
// `bool (*run)()` is a pointer to a function that takes nothing and returns
// bool. `&refresher::lru_cache::Run` is the address of that function.
struct Exercise {
    const char* name;
    bool (*run)();
};

static const Exercise kExercises[] = {
    {"maximal_square", &refresher::maximal_square::Run},
    {"count_fields", &refresher::count_fields::Run},
    {"longest_unique_run", &refresher::longest_unique_run::Run},
    {"merge_schedules", &refresher::merge_schedules::Run},
    {"min_harvest_speed", &refresher::min_harvest_speed::Run},
    {"busiest_fields", &refresher::busiest_fields::Run},
    {"lru_cache", &refresher::lru_cache::Run},
    {"rate_limiter", &refresher::rate_limiter::Run}, 
    {"consistent_hashing", &refresher::consistent_hashing::Run},
    {"house_robber_circle", &refresher::house_robber_circle::Run},
    {"max_depth_tree", &refresher::max_depth_tree::Run}, 
    {"max_path_sum", &refresher::max_path_sum::Run},
    
    
    // {"two_sum", &refresher::two_sum::Run},
};

int main(int argc, char** argv) {
    // Command line: -v (traces), --list, and any number of exercise names.
    std::vector<std::string> only;
    bool list_only = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-v")
            refresher::g_verbose = true;
        else if (arg == "--list")
            list_only = true;
        else
            only.push_back(arg);
    }

    // A lambda: a small unnamed function defined right where it is used.
    // `[&]` means "it may use the variables around it (here: `only`)".
    const auto is_selected = [&](const std::string& name) {
        return only.empty() || std::find(only.begin(), only.end(), name) != only.end();
    };

    // Reject names that are not in the table, instead of silently doing nothing.
    for (const std::string& wanted : only) {
        const bool known = std::any_of(std::begin(kExercises), std::end(kExercises),
                                       [&](const Exercise& e) { return wanted == e.name; });
        if (!known) {
            std::cerr << "unknown exercise '" << wanted << "' (try --list)\n";
            return 2;
        }
    }

    int passed = 0, failed = 0;
    // Range-based for (C++11): walks the whole array; `const auto&` = no copy.
    for (const auto& exercise : kExercises) {
        if (list_only) {
            std::cout << exercise.name << "\n";
            continue;
        }
        if (!is_selected(exercise.name)) continue;

        refresher::g_checks = refresher::g_failures = 0;
        bool ok = false;
        if (refresher::Verbose()) std::cout << "=== " << exercise.name << " ===\n";
        try {
            ok = exercise.run();  // call through the function pointer
        } catch (const std::exception& e) {
            // One broken exercise must not stop the others.
            std::cerr << "  EXCEPTION in " << exercise.name << ": " << e.what() << "\n";
        }
        ok = ok && refresher::AllChecksPassed();
        std::cout << (ok ? "PASS  " : "FAIL  ") << exercise.name << "  (" << refresher::g_checks
                  << " checks)\n";
        ++(ok ? passed : failed);
    }

    if (!list_only) std::cout << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;  // non-zero exit code makes `make` and CI fail
}
