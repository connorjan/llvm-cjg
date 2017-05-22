//===-- CJGTargetInfo.cpp - CJG Target Implementation ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/CJGMCTargetDesc.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheCJGTarget() {
  static Target TheCJGTarget;
  return TheCJGTarget;
}

extern "C" void LLVMInitializeCJGTargetInfo() {
  RegisterTarget<Triple::cjg> X(getTheCJGTarget(), "cjg", "CJG");
}
