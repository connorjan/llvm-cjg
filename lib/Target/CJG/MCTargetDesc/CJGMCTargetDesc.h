//===-- CJGMCTargetDesc.h - CJG Target Descriptions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides CJG specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef CJGMCTARGETDESC_H
#define CJGMCTARGETDESC_H

#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class Target;

Target &getTheCJGTarget();

} // End llvm namespace

#endif
