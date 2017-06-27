//===-- CJGISelLowering.h - CJG DAG Lowering Interface ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that CJG uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef CJGISELLOWERING_H
#define CJGISELLOWERING_H

#include "CJG.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {

class CJGSubtarget;

namespace CJGISD {
enum NodeType {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,

  // Return with a flag operand. Operand 0 is the chain operand.
  RET_FLAG,

  // The compare instruction
  CMP,

  // Branch conditional, condition-code
  BR_CC,

  // Select the condition code
  SELECT_CC,
};
}

//===----------------------------------------------------------------------===//
// TargetLowering Implementation
//===----------------------------------------------------------------------===//

class CJGTargetLowering : public TargetLowering {
public:
  explicit CJGTargetLowering(const CJGTargetMachine &TM,
                             const CJGSubtarget &STI);

  // Provide custom lowering hooks for some operations.
  virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;

  // This method returns the name of a target specific DAG node.
  virtual const char *getTargetNodeName(unsigned Opcode) const override;

private:
  // Lower incoming arguments, copy physregs into vregs
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;
  
  bool shouldConvertConstantLoadToIntImm(const APInt &Imm,
                                         Type *Ty) const override {
    return true;
  }
};
}

#endif
