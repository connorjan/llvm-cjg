//===-- CJGFixupKinds.h - CJG Specific Fixup Entries ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef CJGFIXUPKINDS_H
#define CJGFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
namespace CJG {

  // This table *must* be in the same order of
  // MCFixupKindInfo Infos[CJG::NumTargetFixupKinds]
  // in CJGAsmBackend.cpp.
  enum Fixups {
    fixup_CJG_NONE = FirstTargetFixupKind,

    // Pure 32 but data fixup
    fixup_CJG_32,

    // Marker
    LastTargetFixupKind,
    NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
  };
} // namespace CJG
} // namespace llvm


#endif
