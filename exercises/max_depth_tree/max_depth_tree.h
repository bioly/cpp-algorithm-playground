#pragma once

// ============================================================================
// Exercise: Maximum Depth of a Binary Tree  -  "How deep is the category menu?"
// Pattern : recursion on a tree, and the same job level by level  Level: easy
// Source  : LeetCode 104
// ============================================================================
//
// STORY
//   The bookstore's category menu is a tree: the top category has up to two
//   subcategories (a LEFT and a RIGHT one), each of those may have two more, and
//   so on. How many levels deep is the longest chain from the top category down
//   to the deepest one? (Count categories, not arrows: a lone top category is
//   1 level deep.)
//
//            3                 longest chains: 3 -> 20 -> 15
//           / \                                3 -> 20 -> 7
//          9   20              both have 3 nodes, so the answer is 3
//             /  \             (15 and 7 have no children)
//            15   7
//
// HOW THE INTERVIEW GOES
//   1. Clarify.   Depth in NODES or in EDGES? (nodes: one node = depth 1.)
//                 Empty tree? (depth 0.)  How big can the tree get? (decides if
//                 recursion is safe: see the traps.)
//   2. Brute      List every root-to-leaf path, count each, keep the longest.
//      force.     Correct, but it stores lots of paths we never need.
//   3. Insight.   Let the children do the work. A node only needs to know:
//                    my depth = 1 (me) + the deeper of my two children
//                    an empty spot has depth 0   <- this is what stops the asking
//                 Each child answers the same question about its own little
//                 tree. Answers travel from the leaves UP to the root.
//                 Second way: walk the tree row by row with a queue and count
//                 the rows. Same answer, no recursion.
//   4. Complexity. n = nodes, h = height of the tree.
//                 Both versions: time O(n), every node is visited once.
//                 Recursion: extra memory O(h) for the call stack
//                            (about log n if balanced, n for a long chain).
//                 Queue:     extra memory O(widest row).
//
// TRAPS (all covered by tests)
//   * Nodes vs edges: a single node is depth 1, not 0.
//   * Empty tree: return 0 BEFORE touching any pointer.
//   * `+` instead of max: that counts the whole tree, not the longest chain.
//   * A tree shaped like a long chain (100000 nodes): recursion goes 100000
//     calls deep and can overflow the stack. The queue version is safe.
//
// Run:  make run ONLY=max_depth_tree V=1
// ============================================================================

namespace refresher {
namespace max_depth_tree {

// A tree node: a value plus a pointer to the left and right child.
// Same shape as LeetCode's (one constructor with default arguments instead of
// three), so  TreeNode leaf{7};  and  TreeNode parent{20, &leaf, nullptr};  both work.
// The pointers do NOT own anything: whoever creates the nodes keeps them alive.
struct TreeNode {
    int val;
    TreeNode* left;   // nullptr = "no child here"
    TreeNode* right;
    TreeNode(int v = 0, TreeNode* l = nullptr, TreeNode* r = nullptr) : val(v), left(l), right(r) {}
};

// Version 1: recursion. The short, obviously-correct answer.
int MaxDepth(const TreeNode* root);

// Version 2: level by level with a queue. Safe for very deep trees.
int MaxDepthByLevels(const TreeNode* root);

bool Run();

}  // namespace max_depth_tree
}  // namespace refresher
