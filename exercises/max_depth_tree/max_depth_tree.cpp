#include "max_depth_tree/max_depth_tree.h"

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <vector>

#include "check.h"

namespace refresher {
namespace max_depth_tree {

// ---------------------------------------------------------------------------
// Version 1: recursion
// ---------------------------------------------------------------------------
// C++ NOTE: `const TreeNode*` = "a pointer to a node I promise not to change".
// We only READ the tree, so const is the honest signature.
int MaxDepth(const TreeNode* node) {
    // The stopping rule: an empty spot (nullptr) is depth 0. Without this line
    // we would follow a null pointer and crash.
    // C++ NOTE: use nullptr, not NULL or 0. It has its own type, so it cannot be
    // mistaken for an integer.
    if (node == nullptr) return 0;

    // Ask each child the same question about its own little tree. Each call
    // waits for its children, so leaves answer FIRST and the root answers LAST.
    const int left_depth = MaxDepth(node->left);
    const int right_depth = MaxDepth(node->right);

    // Me (1) plus the deeper side. max, NOT +: we want the longest single chain.
    // C++ NOTE: std::max(a, b) needs both arguments to be the same type.
    return 1 + std::max(left_depth, right_depth);
}

// ---------------------------------------------------------------------------
// Version 2: level by level (no recursion, so a very deep tree cannot crash it)
// ---------------------------------------------------------------------------
int MaxDepthByLevels(const TreeNode* root) {
    if (root == nullptr) return 0;

    // CONTAINER: std::queue<const TreeNode*>, the nodes WAITING to be visited.
    //   First-in-first-out: push at the back, take the oldest from the front, so
    //   the rows come out in order: the top, then its children, then THEIR
    //   children. It is an adapter (over std::deque by default) that offers only
    //   push, pop, front, size, empty.
    //   Why not std::vector: removing the front shifts everything, O(n).
    //   Why not std::stack: last-in-first-out is a different visiting order.
    //   Compare all containers: docs/containers.md
    //   Docs: https://en.cppreference.com/w/cpp/container/queue
    std::queue<const TreeNode*> waiting;
    waiting.push(root);
    int levels = 0;

    while (!waiting.empty()) {
        ++levels;  // someone is waiting, so one more row exists

        // THE TRICK: remember how many nodes are in THIS row before we add any
        // children. The children we push belong to the NEXT row.
        const size_t nodes_in_this_level = waiting.size();
        for (size_t i = 0; i < nodes_in_this_level; ++i) {
            const TreeNode* node = waiting.front();  // look at the oldest waiting node
            waiting.pop();                           // pop() only removes, it returns nothing
            if (node->left != nullptr) waiting.push(node->left);
            if (node->right != nullptr) waiting.push(node->right);
        }
    }
    return levels;
}

// ---------------------------------------------------------------------------
// Tests: small trees written by hand, like on a whiteboard
// ---------------------------------------------------------------------------
namespace {

// Both versions must always agree, so every test checks both.
bool DepthIs(const TreeNode* root, int expected) {
    return MaxDepth(root) == expected && MaxDepthByLevels(root) == expected;
}

void Trace(const TreeNode* root);  // narrated version, at the bottom of the file

}  // namespace

bool Run() {
    CHECK(DepthIs(nullptr, 0));  // empty tree: no pointer may be followed

    TreeNode only{5};
    CHECK(DepthIs(&only, 1));    // TRAP: one node is depth 1, not 0

    // Example 1 from the task (drawn in the header). Build the LEAVES first, so
    // a parent can point at its children:   TreeNode{value, &left, &right}
    TreeNode nine{9}, fifteen{15}, seven{7};
    TreeNode twenty{20, &fifteen, &seven};
    TreeNode root{3, &nine, &twenty};
    CHECK(DepthIs(&root, 3));    // TRAP: `+` instead of max would answer 5 (all nodes)

    // Example 2: node 1 with only a RIGHT child, 2.
    TreeNode two{2};
    TreeNode one{1, nullptr, &two};
    CHECK(DepthIs(&one, 2));

    // A chain down the left side: 1 -> 2 -> 3 -> 4.
    TreeNode d{4}, c{3, &d}, b{2, &c}, a{1, &b};
    CHECK(DepthIs(&a, 4));

    // A "wide" tree: 1 has children 2 and 3; 2 has only a right child (4); 3 has
    // two children (5, 6). Rows now hold SEVERAL nodes, which is what the
    // `nodes_in_this_level` snapshot protects against. Without it the queue
    // version spills into the next row early and answers 4 instead of 3.
    TreeNode four{4}, five{5}, six{6};
    TreeNode left{2, nullptr, &four}, right{3, &five, &six};
    TreeNode wide{1, &left, &right};
    CHECK(DepthIs(&wide, 3));

    // A chain of 100000 nodes (each the left child of the next). The queue
    // version copes; the recursive one would nest 100000 calls and could
    // overflow the stack. That is why the second version exists.
    // (std::vector<TreeNode> chain(n) creates n nodes; no new/delete needed.)
    std::vector<TreeNode> chain(100000);
    for (size_t i = 0; i + 1 < chain.size(); ++i) chain[i].left = &chain[i + 1];
    CHECK(MaxDepthByLevels(&chain[0]) == 100000);

    if (Verbose()) Trace(&root);
    return AllChecksPassed();
}

// ---------------------------------------------------------------------------
// Bonus: the same algorithms, narrated (only printed with V=1)
// ---------------------------------------------------------------------------
namespace {

int TraceDepth(const TreeNode* node, const char* label, int indent) {
    const std::string pad(static_cast<size_t>(indent) * 2, ' ');
    if (node == nullptr) {
        std::cout << "    " << pad << label << ": empty -> 0\n";
        return 0;
    }
    std::cout << "    " << pad << label << ": node " << node->val << ", asking children...\n";
    const int left_depth = TraceDepth(node->left, "left ", indent + 1);
    const int right_depth = TraceDepth(node->right, "right", indent + 1);
    const int depth = 1 + std::max(left_depth, right_depth);
    std::cout << "    " << pad << "node " << node->val << ": 1 + max(" << left_depth << ", "
              << right_depth << ") = " << depth << "\n";
    return depth;
}

void Trace(const TreeNode* root) {
    std::cout << "\n            3\n           / \\\n          9   20\n"
              << "             /  \\\n            15   7\n\n"
              << "  RECURSION: every node asks its children, then reports upwards\n";
    const int depth = TraceDepth(root, "root ", 0);  // prints while it runs, so call it first
    std::cout << "  answer: " << depth << "\n\n"
              << "  LEVEL BY LEVEL: the queue holds the row we are visiting\n";
    std::queue<const TreeNode*> waiting;
    waiting.push(root);
    int levels = 0;
    while (!waiting.empty()) {
        std::cout << "    level " << ++levels << ": [";
        const size_t nodes_in_this_level = waiting.size();
        for (size_t i = 0; i < nodes_in_this_level; ++i) {
            const TreeNode* node = waiting.front();
            waiting.pop();
            std::cout << (i ? ", " : "") << node->val;
            if (node->left != nullptr) waiting.push(node->left);
            if (node->right != nullptr) waiting.push(node->right);
        }
        std::cout << "]\n";
    }
    std::cout << "  answer: " << levels << " levels\n";
}

}  // namespace

}  // namespace max_depth_tree
}  // namespace refresher
