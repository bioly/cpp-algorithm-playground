#pragma once

// ============================================================================
// Exercise: Binary Tree Maximum Path Sum  -  "The most profitable chain"
// Pattern : tree recursion that returns ONE thing and records ANOTHER   Level: hard
// Source  : LeetCode 124
// ============================================================================
//
// STORY
//   The bookstore's categories form a tree, and every category has a profit
//   (it can be NEGATIVE: some categories lose money). Pick ONE connected chain of
//   categories, like a snake, that starts and ends anywhere and visits no
//   category twice. What is the biggest total profit?
//
//             -10
//             /  \             The best chain is 15 -> 20 -> 7 = 42.
//            9    20           It does not need the top box; here the -10
//                /  \          would only hurt.
//               15    7
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Must the chain pass through the root? (no.)  At least one
//                 node? (yes.)  Can every value be negative? (yes: then the
//                 answer is the least-bad single node.)
//   2. Brute      Try every pair of start and end nodes, add up the chain
//      force.     between them. Correct, but about n^2 chains.
//   3. Insight.   Every chain has ONE highest box, like the peak of a hill. So
//                 visit each box and ask: "if I am the highest box, how big can
//                 the chain be?"  =  me + my best arm on the left + my best arm
//                 on the right (an "arm" is a chain going DOWN one side).
//                 Each box does two different things with its arms:
//                    for the ANSWER: use BOTH arms (a V shape)
//                    for my PARENT:  offer only ONE arm (a line), because the
//                                    chain cannot fork when it climbs higher
//                 A negative arm is skipped (counted as 0).
//   4. Complexity. n = nodes, h = height.
//                 Time O(n): every node is visited once.
//                 Extra memory O(h): the recursion stack (n for a long chain).
//
// TRAPS (all covered by tests)
//   * Starting the best answer at 0: a tree with only -3 must answer -3.
//   * Not skipping negative arms: [2, -1] must answer 2, not 1.
//   * Offering BOTH arms to the parent: the chain would fork. (Test: a box with
//     two big leaves under a small root.)
//   * Assuming the chain goes through the root.
//
// Run:  make run ONLY=max_path_sum V=1
// ============================================================================

namespace refresher {
namespace max_path_sum {

// A tree node: a value plus a pointer to the left and right child.
// (Same as in max_depth_tree: nullptr = "no child here"; the pointers own nothing.)
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int v = 0, TreeNode* l = nullptr, TreeNode* r = nullptr) : val(v), left(l), right(r) {}
};

// The biggest sum of any chain of connected nodes. An empty tree has no chain: 0.
int MaxPathSum(const TreeNode* root);

bool Run();

}  // namespace max_path_sum
}  // namespace refresher
