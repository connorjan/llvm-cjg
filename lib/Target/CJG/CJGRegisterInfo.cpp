//===-- CJGRegisterInfo.cpp - CJG Register Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CJG implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "CJGRegisterInfo.h"
#include "CJG.h"
#include "CJGSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "CJGGenRegisterInfo.inc"

using namespace llvm;

CJGRegisterInfo::CJGRegisterInfo() : CJGGenRegisterInfo(CJG::SR) {}

const MCPhysReg *
CJGRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CC_Save_SaveList;
}

BitVector CJGRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(CJG::SR); // status regsiter
  Reserved.set(CJG::PC); // status regsiter
  Reserved.set(CJG::SP); // status regsiter
  
  return Reserved;
}

void CJGRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  llvm_unreachable("Subroutines not supported yet");
}

unsigned CJGRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return CJG::SP;
}
