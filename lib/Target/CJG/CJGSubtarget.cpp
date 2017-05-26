//===-- CJGSubtarget.cpp - CJG Subtarget Information ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CJG specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "CJG.h"
#include "CJGFrameLowering.h"
#include "CJGSubtarget.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "cjg-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "CJGGenSubtargetInfo.inc"

void CJGSubtarget::anchor() {}

CJGSubtarget::CJGSubtarget(const Triple &TT, StringRef CPU, StringRef FS,
                           CJGTargetMachine &TM)
    : CJGGenSubtargetInfo(TT, CPU, FS), InstrInfo(), FrameLowering(*this),
      TLInfo(TM, *this) {}
