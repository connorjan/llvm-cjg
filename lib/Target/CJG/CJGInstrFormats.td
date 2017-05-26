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
  let Size = 4; // Not sure what this means, 4 bytes maybe?

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
