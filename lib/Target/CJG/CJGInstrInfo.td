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


def CJGimm16 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= 0 && Imm < 65536;
}]>;

//===----------------------------------------------------------------------===//
// Type Profiles.
//===----------------------------------------------------------------------===//

// Ensure the two operands of compare are of the same type
def SDT_CJGCmp  : SDTypeProfile<0, 2, [SDTCisSameAs<0, 1>]>;

//===----------------------------------------------------------------------===//
// CJG Specific Node Definitions.
//===----------------------------------------------------------------------===//

def CJGretFlag : SDNode<"CJGISD::RET_FLAG", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def CJGcmp     : SDNode<"CJGISD::CMP", SDT_CJGCmp, [SDNPOutGlue]>;

//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

let Defs = [SR] in {

multiclass ALU<bits<5> opcode, string opstr, SDNode opnode> {

  def rr : ALU_Inst_RR<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$ri, $rj, $rk"),
                   [(set GPRegs:$ri, (opnode GPRegs:$rj, GPRegs:$rk)),
                    (implicit SR)]> {
  }
  
  def ri : ALU_Inst_RI<opcode, (outs GPRegs:$ri),
                   (ins GPRegs:$rj, CJGimm16:$const),
                   !strconcat(opstr, "\t$ri, $rj, $const"),
                   [(set GPRegs:$ri, (opnode GPRegs:$rj, CJGimm16:$const)),
                    (implicit SR)]> {
  }
}

defm ADD  : ALU<0b01000, "add", add>;
defm SUB  : ALU<0b01001, "sub", sub>;
defm AND  : ALU<0b01100, "and", and>;
defm OR   : ALU<0b01110, "or",  or>;
defm XOR  : ALU<0b01111, "xor", xor>;

} // Defs = [SR]

//===----------------------------------------------------------------------===//
// Data Transfer Instructions
//===----------------------------------------------------------------------===//

def CPYrr  : DT_Inst_RR<0b00010,
                    (outs GPRegs:$ri), (ins GPRegs:$rj),
                    "cpy\t$ri, $rj",
                    []>;

let isAsCheapAsAMove = 1 in {
  def CPYri  : DT_Inst_RI<0b00010,
                      (outs GPRegs:$ri), (ins CJGimm16:$const),
                      "cpy\t$ri, $const",
                      [(set GPRegs:$ri, CJGimm16:$const)]>;
} // isAsCheapAsAMove = 1

let Defs = [SP], Uses = [SP], hasSideEffects=0 in {

  def PUSH  : DT_Inst_RR<0b00011,
                    (outs), (ins GPRegs:$rj),
                    "push\t$rj",
                    []> {
    let ri = 0b00000; // destination register not used
  }

  def POP   : DT_Inst_RR<0b00100,
                    (outs GPRegs:$ri), (ins),
                    "pop\t$ri",
                    []> {
    let rj = 0b00000; // source register not used
  }

} // Defs = [SP], Uses = [SP], hasSideEffects=0

//===----------------------------------------------------------------------===//
// Comparison Instructions
//===----------------------------------------------------------------------===//

let Defs = [SR], Uses = [SR] in {

multiclass CMP_Inst<bits<5> opcode, string opstr, SDNode opnode> {
  
  def rr : ALU_Inst_RR<opcode, (outs),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$rj, $rk"),
                   [(opnode GPRegs:$rj, GPRegs:$rk),
                   (implicit SR)]> {
    let ri = 0b00000;
  }
  
  def ri : ALU_Inst_RI<opcode, (outs),
                   (ins GPRegs:$rj, CJGimm16:$const),
                   !strconcat(opstr, "\t$rj, $const"),
                   [(opnode GPRegs:$rj, CJGimm16:$const),
                   (implicit SR)]> {
    let ri = 0b00000;
  }
}

defm CMP  : CMP_Inst<0b01010, "cmp", CJGcmp>;

} // Defs = [SR], Uses = [SR]

//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//

//defm LD   : LS_Inst<

//===----------------------------------------------------------------------===//
// Return Instructions
//===----------------------------------------------------------------------===//

let isTerminator = 1, isReturn = 1, isBarrier = 1 in {
  def RET : InstCJG<(outs), (ins),
                    "ret",  [(CJGretFlag)]> {
    let Opcode = 0b00111;
    let Inst{26-0}  = 0b000000000000000000000000000;
  }
}
