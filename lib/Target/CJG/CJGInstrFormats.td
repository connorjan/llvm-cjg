//===-- CJGInstrFormats.td - CJG Instruction Formats -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//
class InstCJG<dag outs, dag ins, string asmstr, list<dag> pattern>
    : Instruction {
  field bits<32> Inst;

  let Namespace = "CJG";
  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  let Pattern = pattern;
  let Size = 4;

  // define Opcode in base class because all instrutions have the same
  // bit-size and bit-location for the Opcode
  bits<5> Opcode = 0;
  let Inst{31-27} = Opcode; // set upper 5 bits to opcode
}

// CJG pseudo instructions format
class CJGPseudoInst<dag outs, dag ins, string asmstr, list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

//===----------------------------------------------------------------------===//
// ALU Instructions
//===----------------------------------------------------------------------===//

// ALU register-register instruction
class ALU_Inst_RR<bits<5> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri; // destination register
  bits<5> rj; // source 1 register
  bits<5> rk; // source 2 register

  let Opcode = opcode;
  let Inst{26-22} = ri;
  let Inst{21-17} = rj;
  let Inst{16-12} = rk;
  let Inst{11-1} = 0;
  let Inst{0} = 0b0; // control-bit for immediate mode
}

// ALU register-immediate instruction
class ALU_Inst_RI<bits<5> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri; // destination register
  bits<5> rj; // source 1 register
  bits<16> const; // constant/immediate value

  let Opcode = opcode;
  let Inst{26-22} = ri;
  let Inst{21-17} = rj;
  let Inst{16-1} = const;
  let Inst{0} = 0b1; // control-bit for immediate mode
}

//===----------------------------------------------------------------------===//
// Shift/Rotate Instructions
//===----------------------------------------------------------------------===//

// register-register
class RotShift_Inst_RR<bits<3> rs_op, dag outs, dag ins,
                  string asmstr, list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {
  bits<5> ri; // destination register
  bits<5> rj; // source 1 register (data)
  bits<5> rk; // source 2 register (shift/rotate amount)

  let Opcode = 0b10000;
  let Inst{26-22} = ri;
  let Inst{21-17} = rj;
  let Inst{16-12} = rk;
  let Inst{11-4} = 0b00000000; // unused
  let Inst{3-1} = rs_op; // which rotate/shift operation to perform
  let Inst{0} = 0b0; // Control bit to use rk instead of immediate
}

// register-immediate
class RotShift_Inst_RI<bits<3> rs_op, dag outs, dag ins,
                  string asmstr, list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {
  bits<5> ri; // destination register
  bits<5> rj; // source 1 register (data)
  bits<6> constant; // immediate value for shift/rotate amount

  let Opcode = 0b10000;
  let Inst{26-22} = ri;
  let Inst{21-17} = rj;
  let Inst{16-11} = constant;
  let Inst{10-4} = 0b0000000; // unused
  let Inst{3-1} = rs_op; // which rotate/shift operation to perform
  let Inst{0} = 0b1; // Control bit to use rk instead of immediate
}

//===----------------------------------------------------------------------===//
// Data Transfer Instructions
//===----------------------------------------------------------------------===//

// Data Transfer register-register instruction
class DT_Inst_RR<bits<5> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri; // destination register
  bits<5> rj; // source 1 register

  let Opcode = opcode;
  let Inst{26-22} = ri;
  let Inst{21-17} = rj;
  let Inst{16} = 0b0; // control-bit for immediate mode
  let Inst{15-0} = 0; // immediate value not used
}

// Data Transfer register-immediate instruction
class DT_Inst_RI<bits<5> opcode, dag outs, dag ins, string asmstr,
                  list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri; // destination register
  bits<16> const; // constant/immediate value

  let Opcode = opcode;
  let Inst{26-22} = ri;
  let Inst{21-17} = 0; // rj is not used for immediate mode
  let Inst{16} = 0b1; // control-bit for immediate mode
  let Inst{15-0} = const;
}

//===----------------------------------------------------------------------===//
// Load/Store Instructions
//===----------------------------------------------------------------------===//

class LS_Inst<bits<5> opcode, dag outs, dag ins, string asmstr,
                list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri;
  // bits<5> rj; // This is combined into the memsrc addr operand
  bits<1> control;
  bits<21> addr;

  let Opcode = opcode;
  let Inst{26-22} = ri;
  let Inst{21-17} = addr{20-16}; // rj
  let Inst{16} = control;
  let Inst{15-0} = addr{15-0};
}

//===----------------------------------------------------------------------===//
// Flow Control Instructions
//===----------------------------------------------------------------------===//

class FC_Inst<bits<5> opcode, dag outs, dag ins, string asmstr,
                list<dag> pattern>
    : InstCJG<outs, ins, asmstr, pattern> {

  bits<5> ri;
  bits<4> condition;
  bits<1> control;
  bits<16> addr;

  let Opcode = opcode;
  let Inst{26-22} = ri; // for addressing mode control
  let Inst{21-18} = condition; // cnvz
  let Inst{17} = 0b0; // unused
  let Inst{16} = control; // for addressing mode control
  let Inst{15-0} = addr; // target address
}
