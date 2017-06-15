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
// CJG Operand Definitions.
//===----------------------------------------------------------------------===//

// Immediate/Constant operand for ALU/data-transfer instructions
def CJGimm16 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= -32768 && Imm < 32768;
}]>;

// Immediate/Constant operand for rotate/shift instructions
def CJGimm6 : Operand<i32>, ImmLeaf<i32, [{
  return Imm >= 0 && Imm < 64;
}]>;

// Address operand
def memsrc : Operand<i32> {
  let PrintMethod = "printMemSrcOperand";
  let MIOperandInfo = (ops GPRegs, CJGimm16);
}

//===----------------------------------------------------------------------===//
// CJG Complex Pattern Definitions.
//===----------------------------------------------------------------------===//

def addr : ComplexPattern<iPTR, 2, "SelectAddr", [], []>;

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

// Instructions that need their own dag pattern

def NOTrr : ALU_Inst_RR<0b01011,
                    (outs GPRegs:$ri),
                    (ins GPRegs:$rj),
                    "not\t$ri, $rj",
                    [(set GPRegs:$ri, (not GPRegs:$rj)),
                     (implicit SR)]> {
  let rk = 0b00000; // rk not used
}

def BICrr : ALU_Inst_RR<0b01101,
                    (outs GPRegs:$ri),
                    (ins GPRegs:$rj, GPRegs:$rk),
                    "bic\t$ri, $rj, $rk",
                    [(set GPRegs:$ri, (and GPRegs:$rj, (not GPRegs:$rk))),
                     (implicit SR)]> {}

def BICri : ALU_Inst_RR<0b01101,
                    (outs GPRegs:$ri),
                    (ins GPRegs:$rj, CJGimm16:$const),
                    "bic\t$ri, $rj, $const",
                    [(set GPRegs:$ri, (and GPRegs:$rj, (not CJGimm16:$const))),
                     (implicit SR)]> {}

} // Defs = [SR]

//===----------------------------------------------------------------------===//
// Shift/Rotate Instructions
//===----------------------------------------------------------------------===//

let Defs = [SR] in {

multiclass RotShift<bits<3> rs_op, string opstr, SDNode opnode> {

  def rr : RotShift_Inst_RR<rs_op, 
                          (outs GPRegs:$ri),
                          (ins GPRegs:$rj, GPRegs:$rk),
                          !strconcat(opstr, "\t$ri, $rj, $rk"),
                          [(set GPRegs:$ri, (opnode GPRegs:$rj, GPRegs:$rk)),
                          (implicit SR)]> {}

  def ri : RotShift_Inst_RI<rs_op,
                   (outs GPRegs:$ri),
                   (ins GPRegs:$rj, CJGimm6:$const),
                   !strconcat(opstr, "\t$ri, $rj, $const"),
                   [(set GPRegs:$ri, (opnode GPRegs:$rj, CJGimm6:$const)),
                    (implicit SR)]> {}
}

defm SRL  : RotShift<0b000, "srl", srl>; // shift right logical
defm SLL  : RotShift<0b001, "sll", shl>; // shift left logical
defm SRA  : RotShift<0b010, "sra", sra>; // shift left arithmetic
defm RTR  : RotShift<0b100, "rtr", rotr>; // rotate right
defm RTL  : RotShift<0b101, "rtl", rotl>; // rotate left

let Uses = [SR] in {
  //defm RRC  : RotShift<0b110, "rrc">; // rotate right through carry
  //defm RLC  : RotShift<0b111, "rlc">; // rotate left through carry
} // Uses = [SR]
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

let Defs = [SP], hasSideEffects=0 in {

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

} // Defs = [SP], hasSideEffects=0

//===----------------------------------------------------------------------===//
// Comparison Instructions
//===----------------------------------------------------------------------===//

let Defs = [SR] in {

multiclass CMP_Inst<bits<5> opcode, string opstr> {
  
  def rr : ALU_Inst_RR<opcode, (outs),
                   (ins GPRegs:$rj, GPRegs:$rk),
                   !strconcat(opstr, "\t$rj, $rk"),
                   [(CJGcmp GPRegs:$rj, GPRegs:$rk),
                   (implicit SR)]> {
    let ri = 0b00000;
  }
  
  def ri : ALU_Inst_RI<opcode, (outs),
                   (ins GPRegs:$rj, CJGimm16:$const),
                   !strconcat(opstr, "\t$rj, $const"),
                   [(CJGcmp GPRegs:$rj, CJGimm16:$const),
                   (implicit SR)]> {
    let ri = 0b00000;
  }
}

defm CMP  : CMP_Inst<0b01010, "cmp">;

} // Defs = [SR]

//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//

def LD  : LS_Inst<0b00000,
                  (outs GPRegs:$ri), (ins memsrc:$addr),
                  "ld\t$ri, $addr",
                  [(set GPRegs:$ri, (load addr:$addr))]> {
  let rj = 0b00000;   // For now set rj and control to 0
  let control = 0b1;  // This forces absolute addressing mode
}

def ST  : LS_Inst<0b00001,
                  (outs), (ins memsrc:$addr, GPRegs:$ri),
                  "st\t$addr, $ri",
                  [(store GPRegs:$ri, addr:$addr)]> {
  let rj = 0b00000;   // For now set rj and control to 0
  let control = 0b1;  // This forces absolute addressing mode
}

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
