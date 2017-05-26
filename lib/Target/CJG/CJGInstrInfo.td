//===-- CJGInstrInfo.td - Target Description for CJG -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the CJG instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "CJGInstrFormats.td"

//def const16 : Operand<i32>; // Type of the constant/immediate value

def CJGimm16 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= 0 && Imm < 65536;
}]>;

// Target-independent nodes
def RetFlag : SDNode<"CJGISD::RET_FLAG", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

multiclass ALU<bits<5> opcode, string opstr, SDNode opnode> {

  def rr : ALU_Inst_RR<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$ri, $rj, $rk"),
                   [(set i32:$ri, (opnode i32:$rj, i32:$rk))]> {
  }
  
  def ri : ALU_Inst_RI<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, CJGimm16:$const),
                   !strconcat(opstr, "\t$ri, $rj, $const"),
                   [(set i32:$ri, (opnode i32:$rj, CJGimm16:$const))]> {
  }
}

defm ADD  : ALU<0b01000, "add", add>;
defm SUB  : ALU<0b01001, "sub", sub>;
defm AND  : ALU<0b01100, "and", and>;
defm OR   : ALU<0b01110, "or",  or>;
defm XOR  : ALU<0b01111, "xor", xor>;

//===----------------------------------------------------------------------===//
// Data Transfer Instructions
//===----------------------------------------------------------------------===//

multiclass DT<bits<5> opcode, string opstr> {
  
  def rr : DT_Inst_RR<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj),
                   !strconcat(opstr, "\t$ri, $rj"),
                   [/*(set i32:$ri, (opnode i32:$rj))*/]> {
  }
  
  def ri : DT_Inst_RI<opcode, (outs GPRegs:$ri),
                   (ins CJGimm16:$const),
                   !strconcat(opstr, "\t$ri, $const"),
                   [/*(set i32:$ri, (opnode const16:$const))*/]> {
  }
}

defm CPY  : DT<0b00010, "cpy">;

//===----------------------------------------------------------------------===//
// Comparison Instructions
//===----------------------------------------------------------------------===//

multiclass CMP_Inst<bits<5> opcode, string opstr> {
  
  def rr : ALU_Inst_RR<opcode, (outs),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$rj, $rk"), []> {
  }
  
  def ri : ALU_Inst_RI<opcode, (outs),
                   (ins GPRegs:$rj, CJGimm16:$const),
                   !strconcat(opstr, "\t$rj, $const"), []> {
  }
}

defm CMP  : CMP_Inst<0b01010, "cmp">;
