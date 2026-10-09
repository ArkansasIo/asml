// ============================================================================
//  ASMLEX - include/asmlex/nodes/Node.hpp
//  Generic node graph.  Both the disassembly listing and the type editor are
//  modelled as trees of Nodes so selection, collapse state and rendering can
//  be shared across every pane.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <memory>
#include <vector>

namespace asmlex::nodes
{

  enum class NodeKind
  {
    Root,
    Section,
    Function,
    BasicBlock,
    Instruction,
    DataChunk,
    TypeField,
    Annotation
  };

  [[nodiscard]] inline StringView to_string(NodeKind kind) noexcept
  {
    switch (kind)
    {
    case NodeKind::Root:
      return "root";
    case NodeKind::Section:
      return "section";
    case NodeKind::Function:
      return "function";
    case NodeKind::BasicBlock:
      return "block";
    case NodeKind::Instruction:
      return "instruction";
    case NodeKind::DataChunk:
      return "data";
    case NodeKind::TypeField:
      return "field";
    case NodeKind::Annotation:
      return "annotation";
    }
    return "root";
  }

  /// A single entry of the graph.  Children are owned outright; the parent
  /// pointer is a non-owning back reference for upward walks.
  struct Node
  {
    Id id;
    NodeKind kind = NodeKind::Root;
    String label;    ///< Text shown in the tree / listing column.
    String detail;   ///< Mnemonic, rendered bytes, resolved symbol, ...
    u64 address = 0; ///< Virtual address, 0 when not addressable.
    u32 size = 0;    ///< Size in bytes for instructions and data.
    bool collapsed = false;
    bool highlighted = false;
    Node *parent = nullptr;
    std::vector<std::unique_ptr<Node>> children;
  };

  /// Depth-first walk producing every node, parents before children.
  [[nodiscard]] std::vector<const Node *> flatten(const Node &root);

  /// Total node count including `root`.
  [[nodiscard]] usize count(const Node &root);

  /// Depth of the deepest path leaving `root` (root alone == 0).
  [[nodiscard]] u32 depth(const Node &root);

  /// Follow `parent` links up to the owning tree root.
  [[nodiscard]] Node *root_of(Node *node);

  /// Append a child, wiring `parent` and returning a borrowed pointer.
  /// `child` is moved in, so it must not be used afterwards.
  Node *append(Node &parent, Node &&child);

  /// Find the first node whose `id` matches, or nullptr.
  [[nodiscard]] const Node *find_by_id(const Node &root, StringView id);

} // namespace asmlex::nodes
