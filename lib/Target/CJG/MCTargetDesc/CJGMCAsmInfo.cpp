//===-- CJGMCAsmInfo.cpp - CJG asm properties -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "CJGMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
using namespace llvm;

void CJGMCAsmInfo::anchor() {}

CJGMCAsmInfo::CJGMCAsmInfo(const Triple &TT) {
  CommentString = "//";
  SupportsDebugInformation = true;
  PrivateLabelPrefix = "";
}
