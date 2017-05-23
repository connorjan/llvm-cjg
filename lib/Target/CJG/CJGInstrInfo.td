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

def const16 : Operand<i32>; // Type of the constant/immediate value

//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

multiclass ALU<bits<5> opcode, string opstr> {

  def rr : ALU_Inst_RR<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$ri, $rj, $rk"),
                   [/*(set i32:$ri, (opnode i32:$rj, i32:$rk))*/]> {
  }
  
  def ri : ALU_Inst_RI<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, const16:$const),
                   !strconcat(opstr, "\t$ri, $rj, $const"),
                   [/*(set i32:$ri, (opnode i32:$rj, const16:$const))*/]> {
  }
}

defm ADD  : ALU<0b01000, "add">;
defm SUB  : ALU<0b01001, "sub">;
defm AND  : ALU<0b01100, "and">;
defm OR   : ALU<0b01110, "or">;
defm XOR  : ALU<0b01111, "xor">;

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
                   (ins const16:$const),
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
                   (ins GPRegs:$rj, const16:$const),
                   !strconcat(opstr, "\t$rj, $const"), []> {
  }
}

defm CMP  : CMP_Inst<0b01010, "cmp">;
