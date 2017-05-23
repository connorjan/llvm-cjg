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

#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;

class StringRef;
class raw_ostream;
class raw_pwrite_stream;
class Triple;

Target &getTheCJGTarget();

MCCodeEmitter *createCJGMCCodeEmitter(const MCInstrInfo &MCII,
                                      const MCRegisterInfo &MRI,
                                      MCContext &Ctx);

MCAsmBackend *createCJGAsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                  const Triple &TT, StringRef CPU,
                                  const MCTargetOptions &Options);

MCObjectWriter *createCJGELFObjectWriter(raw_pwrite_stream &OS, uint8_t OSABI);

} // End llvm namespace

// Defines symbolic names for CJG registers.
#define GET_REGINFO_ENUM
#include "CJGGenRegisterInfo.inc"

// Defines symbolic names for CJG instructions.
#define GET_INSTRINFO_ENUM
#include "CJGGenInstrInfo.inc"

#endif
