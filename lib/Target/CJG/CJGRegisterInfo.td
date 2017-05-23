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
foreach i = 0-31 in {
  def R#i : CJGReg< #i, "r#i">;
}

def StatusReg : RegisterClass<"CJG", [i32], 32, (add
	R0
)>; 

// General purpose registers class
def GPRegs : RegisterClass<"CJG", [i32], 32, (add
	(sequence "R%u", 1, 31)
)>;
