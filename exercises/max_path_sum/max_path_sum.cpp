#include "max_path_sum/max_path_sum.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <string>

#include "check.h"

namespace refresher {
namespace max_path_sum {

namespace {

// The length of the best ARM starting at `node`: a chain that goes DOWN one side.
// Along the way it also records the best "V" shape into `best_so_far`.
//
// C++ NOTE: `int& best_so_far` is a REFERENCE: another name for the caller's
// variable, not a copy. Every recursive call updates the SAME number, so when
// the recursion ends the caller can read the final answer. (Without the &, each
// call would change its own private copy and the result would be lost.)
int ArmLength(const TreeNode* node, int& best_so_far) {
    if (node == nullptr) return 0;  // no box, no arm

    // Best arm on each side. max(0, ...) means "skip an arm that only loses
    // money": we simply do not walk that way.
    const int left_arm = std::max(0, ArmLength(node->left, best_so_far));
    const int right_arm = std::max(0, ArmLength(node->right, best_so_far));

    // The V shape with ME as the highest box: both arms. This is only a
    // CANDIDATE answer; it can never be handed upward (a chain cannot fork).
    best_so_far = std::max(best_so_far, node->val + left_arm + right_arm);

    // What my parent gets: me plus ONE arm, the better one.
    return node->val + std::max(left_arm, right_arm);
}

}  // namespace

int MaxPathSum(const TreeNode* root) {
    if (root == nullptr) return 0;

    // Start at the SMALLEST int, not 0: if every value is negative the answer is
    // negative too, and a start of 0 would wrongly win.
    // C++ NOTE: std::numeric_limits<int>::min() is the smallest value an int can
    // hold. Docs: https://en.cppreference.com/w/cpp/types/numeric_limits
    int best_so_far = std::numeric_limits<int>::min();
    ArmLength(root, best_so_far);  // we ignore the returned arm; the answer is in best_so_far
    return best_so_far;
}

// ---------------------------------------------------------------------------
// Tests: small trees written by hand. Build the LEAVES first, so a parent can
// point at its children:   TreeNode{value, &left_child, &right_child}
// ---------------------------------------------------------------------------
namespace {
void Trace(const TreeNode* root);  // narrated version, at the bottom of the file
}

bool Run() {
    CHECK(MaxPathSum(nullptr) == 0);

    // The example from the story: the best chain 15 -> 20 -> 7 skips the root.
    TreeNode nine{9}, fifteen{15}, seven{7};
    TreeNode twenty{20, &fifteen, &seven};
    TreeNode root{-10, &nine, &twenty};
    CHECK(MaxPathSum(&root) == 42);

    TreeNode two{2}, three{3};
    TreeNode one{1, &two, &three};
    CHECK(MaxPathSum(&one) == 6);  // 2 -> 1 -> 3

    // TRAP: skip negative arms. Taking the -1 would give 1.
    TreeNode minus_one{-1};
    TreeNode two_with_loss{2, &minus_one};
    CHECK(MaxPathSum(&two_with_loss) == 2);
    TreeNode two_with_right_loss{2, nullptr, &minus_one};  // the same on the RIGHT side
    CHECK(MaxPathSum(&two_with_right_loss) == 2);

    // TRAP: every value negative. The answer is the least-bad single node.
    TreeNode minus_three{-3};
    CHECK(MaxPathSum(&minus_three) == -3);
    TreeNode minus_one_child{-1};
    TreeNode minus_two{-2, &minus_one_child};
    CHECK(MaxPathSum(&minus_two) == -1);

    // TRAP: a chain cannot fork. Box 2 has two leaves of 10: the best chain is
    // 10 -> 2 -> 10 = 22. If box 2 offered BOTH arms to the root (2 + 10 + 10 = 22
    // going up), the root would build 1 + 22 = 23: a chain that does not exist.
    TreeNode ten_a{10}, ten_b{10};
    TreeNode two_with_leaves{2, &ten_a, &ten_b};
    TreeNode small_root{1, &two_with_leaves};
    CHECK(MaxPathSum(&small_root) == 22);

    // A bigger tree with a known answer: 7 -> 11 -> 4 -> 5 -> 8 -> 13 = 48.
    TreeNode n7{7}, n2{2}, n13{13}, n1{1};
    TreeNode n11{11, &n7, &n2};
    TreeNode n4a{4, &n11};
    TreeNode n4b{4, nullptr, &n1};
    TreeNode n8{8, &n13, &n4b};
    TreeNode big{5, &n4a, &n8};
    CHECK(MaxPathSum(&big) == 48);

    if (Verbose()) Trace(&root);
    return AllChecksPassed();
}

// ---------------------------------------------------------------------------
// Bonus: the same algorithm, narrated (only printed with V=1)
// ---------------------------------------------------------------------------
namespace {

int TraceArm(const TreeNode* node, const char* label, int indent, int& best_so_far) {
    const std::string pad(static_cast<size_t>(indent) * 2, ' ');
    if (node == nullptr) return 0;
    std::cout << "    " << pad << label << ": node " << node->val << ", asking children...\n";
    const int left_arm = std::max(0, TraceArm(node->left, "left ", indent + 1, best_so_far));
    const int right_arm = std::max(0, TraceArm(node->right, "right", indent + 1, best_so_far));
    const int v_shape = node->val + left_arm + right_arm;
    const bool is_new_best = v_shape > best_so_far;
    best_so_far = std::max(best_so_far, v_shape);
    const int arm_up = node->val + std::max(left_arm, right_arm);
    std::cout << "    " << pad << "node " << node->val << ": V shape = " << node->val << " + " << left_arm
              << " + " << right_arm << " = " << v_shape << (is_new_best ? "  (new best)" : "")
              << ";  hands up " << node->val << " + max(" << left_arm << ", " << right_arm
              << ") = " << arm_up << "\n";
    return arm_up;
}

void Trace(const TreeNode* root) {
    std::cout << "\n           -10\n           /  \\\n          9    20\n"
              << "              /  \\\n             15    7\n\n"
              << "  every node: V shape (both arms) is a candidate answer, one arm goes up\n";
    int best_so_far = std::numeric_limits<int>::min();
    TraceArm(root, "root ", 0, best_so_far);
    std::cout << "  answer: " << best_so_far << "\n";
}

}  // namespace

}  // namespace max_path_sum
}  // namespace refresher
