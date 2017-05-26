//===-- CJGFrameLowering.cpp - CJG Frame Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the CJG implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "CJGFrameLowering.h"
#include "CJGSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

CJGFrameLowering::CJGFrameLowering(const CJGSubtarget &STI)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 
                            /*StackAlignment=*/4,
                            /*LocalAreaOffset=*/0) {}

bool CJGFrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void CJGFrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}

void CJGFrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {}
