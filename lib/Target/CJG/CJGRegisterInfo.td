//===-- CJGRegisterInfo.td - CJG Register defs -------------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the CJG register file 
//===----------------------------------------------------------------------===//

// General register definition
class CJGReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "CJG";
}

// Define all CPU registers

// Special purpose registers
def SR : CJGReg<0, "r0">;
def PC : CJGReg<1, "r1">;
def SP : CJGReg<2, "r2">;

// General purpose registers
foreach i = 3-31 in {
  def R#i : CJGReg< #i, "r"##i>;
}

def StatusReg : RegisterClass<"CJG", [i32], 32, (add
	SR // R0
)>;

def ProgramCounterReg : RegisterClass<"CJG", [i32], 32, (add
	PC //R1
)>;

// General purpose registers class
def GPRegs : RegisterClass<"CJG", [i32], 32, (add
	(sequence "R%u", 4, 31), SP, R3
)>;
