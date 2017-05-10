//===-- LEGSubtarget.h - Define Subtarget for the LEG -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the LEG specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LEGSUBTARGET_H
#define LEGSUBTARGET_H

#include "LEG.h"
#include "LEGFrameLowering.h"
#include "LEGISelLowering.h"
#include "LEGInstrInfo.h"
#include "LEGSelectionDAGInfo.h"
#include "LEGSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include <string>

#define GET_SUBTARGETINFO_HEADER
#include "LEGGenSubtargetInfo.inc"

namespace llvm {
class StringRef;

class LEGSubtarget : public LEGGenSubtargetInfo {
  virtual void anchor();

private:
  const DataLayout DL;       // Calculates type size & alignment.
  LEGInstrInfo InstrInfo;
  LEGTargetLowering TLInfo;
  LEGSelectionDAGInfo TSInfo;
  LEGFrameLowering FrameLowering;
  InstrItineraryData InstrItins;

public:
  /// This constructor initializes the data members to match that
  /// of the specified triple.
  ///
  LEGSubtarget(const Triple &TT, StringRef CPU,
               StringRef FS, LEGTargetMachine &TM);

  /// getInstrItins - Return the instruction itineraries based on subtarget
  /// selection.
  const InstrItineraryData *getInstrItineraryData() const override {
    return &InstrItins;
  }
  const LEGInstrInfo *getInstrInfo() const override { return &InstrInfo; }
  const LEGRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
  const LEGTargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const LEGFrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const LEGSelectionDAGInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }

  /// ParseSubtargetFeatures - Parses features string setting specified
  /// subtarget options.  Definition of function is auto generated by tblgen.
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);
};
} // End llvm namespace

#endif

