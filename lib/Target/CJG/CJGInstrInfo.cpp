//===-- CJGInstrInfo.cpp - CJG Instruction Information ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CJG implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "CJGInstrInfo.h"
#include "CJG.h"
#include "CJGSubtarget.h"
#include "CJGTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR_DTOR
#include "CJGGenInstrInfo.inc"

using namespace llvm;

// Pin the vtable to this file.
void CJGInstrInfo::anchor() {}

CJGInstrInfo::CJGInstrInfo() : CJGGenInstrInfo(), RI() {}

void CJGInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, unsigned DestReg,
                                  unsigned SrcReg, bool KillSrc) const {

  if (CJG::GPRegsRegClass.contains(DestReg, SrcReg)) {
      BuildMI(MBB, I, DL, get(CJG::CPYrr), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc));
  }
  else {
    llvm_unreachable("Impossible reg-to-reg copy");
  }
}

void CJGInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator MI,
                                       unsigned SrcReg, bool isKill,
                                       int FrameIdx,
                                       const TargetRegisterClass *RC,
                                       const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  BuildMI(MBB, MI, DL, get(CJG::PUSH))
    .addReg(SrcReg);
}

void CJGInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                        MachineBasicBlock::iterator MI,
                                        unsigned DestReg, int FrameIdx,
                                        const TargetRegisterClass *RC,
                                        const TargetRegisterInfo *TRI) const{
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  BuildMI(MBB, MI, DL, get(CJG::POP), DestReg);
}
