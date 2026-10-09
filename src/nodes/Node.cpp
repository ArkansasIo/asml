// ============================================================================
//  ASMLEX - src/nodes/Node.cpp
//  Tree helpers shared by the listing and the type editor.
// ============================================================================
#include <asmlex/nodes/Node.hpp>

#include <algorithm>
#include <utility>

namespace asmlex::nodes
{

  namespace
  {
    void walk(const Node &node, std::vector<const Node *> &out)
    {
      out.push_back(&node);
      for (const auto &child : node.children)
      {
        if (child)
          walk(*child, out);
      }
    }

    usize count_recursive(const Node &node)
    {
      usize total = 1;
      for (const auto &child : node.children)
      {
        if (child)
          total += count_recursive(*child);
      }
      return total;
    }

    u32 depth_recursive(const Node &node)
    {
      u32 best = 0;
      for (const auto &child : node.children)
      {
        if (child)
          best = std::max(best, depth_recursive(*child) + 1);
      }
      return best;
    }
  } // namespace

  std::vector<const Node *> flatten(const Node &root)
  {
    std::vector<const Node *> out;
    out.reserve(count(root));
    walk(root, out);
    return out;
  }

  usize count(const Node &root) { return count_recursive(root); }

  u32 depth(const Node &root) { return depth_recursive(root); }

  Node *root_of(Node *node)
  {
    if (node == nullptr)
      return nullptr;
    while (node->parent != nullptr)
      node = node->parent;
    return node;
  }

  Node *append(Node &parent, Node &&child)
  {
    child.parent = &parent;
    parent.children.push_back(std::make_unique<Node>(std::move(child)));
    return parent.children.back().get();
  }

  const Node *find_by_id(const Node &root, StringView id)
  {
    if (root.id == id)
      return &root;
    for (const auto &child : root.children)
    {
      if (!child)
        continue;
      if (const Node *hit = find_by_id(*child, id))
        return hit;
    }
    return nullptr;
  }

} // namespace asmlex::nodes
