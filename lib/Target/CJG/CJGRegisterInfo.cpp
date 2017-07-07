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

CJGRegisterInfo::CJGRegisterInfo() : CJGGenRegisterInfo(CJG::PC) {}

const MCPhysReg *
CJGRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CC_Save_SaveList; // from tablegen
}

BitVector CJGRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(CJG::SR); // status regsiter
  Reserved.set(CJG::PC); // program counter
  Reserved.set(CJG::SP); // stack pointer
  
  return Reserved;
}

const TargetRegisterClass *
CJGRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind)
                                                                         const {
  return &CJG::GPRegsRegClass;
}

void CJGRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                          int SPAdj, unsigned FIOperandNum,
                                          RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  const MachineFunction &MF = *MBB.getParent();
  const MachineFrameInfo MFI = MF.getFrameInfo();
  MachineOperand &FIOp = MI.getOperand(FIOperandNum);
  unsigned FI = FIOp.getIndex();
    
  // Determine if we can eliminate the index from this kind of instruction.
  unsigned ImmOpIdx = 0;
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Instruction not supported");
  case CJG::LD:
  case CJG::ST:
  case CJG::ADDri:
    ImmOpIdx = FIOperandNum + 1;
    break;
  }
    
  // FIXME: check the size of offset.
  MachineOperand &ImmOp = MI.getOperand(ImmOpIdx);
  int Offset = MFI.getObjectOffset(FI) + MFI.getStackSize() + ImmOp.getImm();
  if (Offset % 4) {
    llvm_unreachable("Offset must be aligned to 4 bytes because memory is "
                     "32-bit word addressable only");
  }
  FIOp.ChangeToRegister(CJG::SP, false);
  ImmOp.setImm(Offset);
}

unsigned CJGRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return CJG::SP;
}
