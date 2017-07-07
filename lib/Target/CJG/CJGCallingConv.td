//===- CJGCallingConv.td - Calling Conventions for CJG -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for CJG architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// CJG Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_CJG : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // i32 are returned in registers R24-R31
  CCIfType<[i32], CCAssignToReg<[R24, R25, R26, R27, R28, R29, R30, R31]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

//===----------------------------------------------------------------------===//
// CJG Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_CJG : CallingConv<[
  // Pass by value if the byval attribute is given
  CCIfByVal<CCPassByVal<4, 4>>,

  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // Regsiters used for parameters
  CCIfType<[i32], CCAssignToReg<[R4, R5, R6, R7, R8]>>,

  // Integer values get stored in stack slots that are 4 bytes in
  // size and 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CC_Save : CalleeSavedRegs<(add SR, PC, SP, R3, R9, R10, R11, R12, R13, R14,
                                R15, R16, R17, R18, R19, R20, R21, R22, R23)>;
