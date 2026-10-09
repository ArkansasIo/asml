// ============================================================================
//  ASMLEX - include/asmlex/asm/Instruction.hpp
//  Instruction representation shared by the assembler, disassembler and the
//  x86-64 execution engine.
// ============================================================================
#pragma once

#include <asmlex/core/Types.hpp>

#include <vector>

namespace asmlex::asm_
{

  /// Operand classification.
  enum class OperandKind
  {
    None,
    Register,
    Immediate,
    Memory, ///< [base + index*scale + disp]
    Label   ///< symbolic reference
  };

  [[nodiscard]] inline StringView to_string(OperandKind k) noexcept
  {
    switch (k)
    {
    case OperandKind::None:
      return "none";
    case OperandKind::Register:
      return "reg";
    case OperandKind::Immediate:
      return "imm";
    case OperandKind::Memory:
      return "mem";
    case OperandKind::Label:
      return "label";
    }
    return "?";
  }

  /// A single operand slot for the built-in instruction profile.
  struct Operand
  {
    OperandKind kind = OperandKind::None;
    i64 value = 0;    ///< immediate / displacement
    String reg;       ///< register name (lower case)
    String base;      ///< memory base register
    String index;     ///< memory index register
    int scale = 1;    ///< memory index scale (1, 2, 4, 8)
    int size = 0;     ///< operand size in bits (0 = infer)
    String symbol;    ///< symbolic target (labels, unresolved names)
    usize indent = 0; ///< source column for pretty printing

    [[nodiscard]] bool valid() const noexcept { return kind != OperandKind::None; }
  };

  /// The instruction form (opcode family) understood by the built-in encoder.
  struct InstructionDesc
  {
    String mnemonic; ///< canonical lower-case name
    u8 opcode = 0;   ///< primary opcode byte
    u8 opcode2 = 0;  ///< 0F-prefixed secondary opcode, 0 if unused
    bool has_modrm = false;
    u8 reg_field = 0; ///< /digit for grouped opcodes (0 = operand reg)
    bool is_jump = false;
    bool is_call = false;
    bool is_ret = false;
    bool is_terminator = false; ///< control does not fall through (jmp, ret)
    usize operand_count = 0;
    String summary;        ///< "MOV r64, r/m64" style description
    String flags_affected; ///< free-form note for the docs pane
  };

  /// A fully decoded instruction with its byte encoding.
  struct DecodedInstruction
  {
    String mnemonic;
    std::vector<Operand> operands;
    String mnemonic_prefix; ///< rep / lock prefixes, verbatim
    u8 opcode = 0;
    u8 opcode2 = 0;
    usize length = 0; ///< encoded length in bytes
    usize offset = 0; ///< offset in the parent buffer
    u64 address = 0;  ///< virtual address
    String raw_bytes; ///< the exact encoded bytes
    String text;      ///< pretty printed disassembly
    bool has_modrm = false;
    u8 modrm = 0;
    u8 sib = 0;
    u8 rex = 0;
    i64 displacement = 0;
    i64 immediate = 0;
    int operand_size = 64; ///< in bits
    String comment;        ///< symbolic annotation added by the disassembler
  };

} // namespace asmlex::asm_
