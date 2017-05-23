//===-- CJGMCAsmInfo.h - CJG asm properties --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the CJGMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef CJGTARGETASMINFO_H
#define CJGTARGETASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class CJGMCAsmInfo : public MCAsmInfoELF {
  virtual void anchor();

public:
  explicit CJGMCAsmInfo(const Triple &TT);
};

} // namespace llvm

#endif
