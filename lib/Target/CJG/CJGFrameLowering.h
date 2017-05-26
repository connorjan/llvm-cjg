//===-- CJGFrameLowering.h - Frame info for CJG Target ---------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This class implements CJG-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef CJGFRAMEINFO_H
#define CJGFRAMEINFO_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class CJGSubtarget;

class CJGFrameLowering : public TargetFrameLowering {
public:
  CJGFrameLowering(const CJGSubtarget &STI);

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;

  static int stackSlotSize() { return 4; } // 4 Bytes?
};
}

#endif
