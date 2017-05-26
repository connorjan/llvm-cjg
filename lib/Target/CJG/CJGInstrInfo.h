//===-- CJGInstrInfo.h - CJG Instruction Information ------------*- C++ -*-===//
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

#ifndef CJGINSTRUCTIONINFO_H
#define CJGINSTRUCTIONINFO_H

#include "CJGRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "CJGGenInstrInfo.inc"

namespace llvm {

class CJGInstrInfo : public CJGGenInstrInfo {
  const CJGRegisterInfo RI;
  virtual void anchor();

public:
  CJGInstrInfo();

  const CJGRegisterInfo &getRegisterInfo() const { return RI; }
};
}

#endif
