//===-- CJGISelLowering.cpp - CJG DAG Lowering Implementation  ------------===//
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

#include "CJGISelLowering.h"
#include "CJG.h"
#include "CJGRegisterInfo.h"
#include "CJGSubtarget.h"
#include "CJGTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "cjg-lower"

const char *CJGTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch ((CJGISD::NodeType)Opcode) {
    case CJGISD::FIRST_NUMBER:            break;
    case CJGISD::CMP:                     return "CJGISD::CMP";
    case CJGISD::RET_FLAG:                return "CJGISD::RET_FLAG";
    case CJGISD::BR_CC:                   return "CJGISD::BR_CC";
    case CJGISD::SELECT_CC:               return "CJGISD::SELECT_CC";
    case CJGISD::Wrapper:                 return "CJGISD::Wrapper";
  }
  return nullptr;
}

CJGTargetLowering::CJGTargetLowering(const CJGTargetMachine &TM,
                                     const CJGSubtarget &STI)
    : TargetLowering(TM) {

  // Set up the register classes.
  addRegisterClass(MVT::i32, &CJG::GPRegsRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties(STI.getRegisterInfo());

  setStackPointerRegisterToSaveRestore(CJG::SP);

  // TODO: add all necessary setOperationAction calls

  setBooleanContents(ZeroOrOneBooleanContent);

  setOperationAction(ISD::BR_JT,            MVT::Other,   Expand);
  setOperationAction(ISD::BR_CC,            MVT::i32,     Custom);
  setOperationAction(ISD::BRCOND,           MVT::Other,   Expand);
  setOperationAction(ISD::SELECT,           MVT::i32,     Expand);
  setOperationAction(ISD::SELECT_CC,        MVT::i32,     Custom);
  setOperationAction(ISD::GlobalAddress,    MVT::i32,     Custom);
  setOperationAction(ISD::ExternalSymbol,   MVT::i32,     Custom);
}

SDValue CJGTargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
    case ISD::BR_CC:            return LowerBR_CC(Op, DAG);
    case ISD::SELECT_CC:        return LowerSELECT_CC(Op, DAG);
    case ISD::GlobalAddress:    return LowerGlobalAddress(Op, DAG);
    case ISD::ExternalSymbol:   return LowerExternalSymbol(Op, DAG);
    default:
      llvm_unreachable("unimplemented operand");
  }
}

// Calling Convention Implementation
#include "CJGGenCallingConv.inc"

// Transform physical registers into virtual registers
SDValue CJGTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
    break;
  }

  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  if (IsVarArg) {
    llvm_unreachable("VarArg not supported");
  }

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, IsVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_CJG);

  for (auto &VA : ArgLocs) {
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) {
      case MVT::i32: {
        const unsigned VReg =
            RegInfo.createVirtualRegister(&CJG::GPRegsRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgIn = DAG.getCopyFromReg(Chain, DL, VReg, RegVT);

        InVals.push_back(ArgIn);
        break;
      }
      default:
        DEBUG(dbgs() << "LowerFormalArguments Unhandled argument type: "
                     << RegVT.getEVTString() << "\n");
        llvm_unreachable("unhandled argument type");
      }
    } else {
      llvm_unreachable("Defined with too many args");
    }
  }
  return Chain;
}

SDValue
CJGTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &DL, SelectionDAG &DAG) const {
  if (IsVarArg) {
    report_fatal_error("VarArg not supported");
  }

  // Stores the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // Info about the registers and stack slot
  CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_CJG);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0, e = RVLocs.size(); i < e; ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode()) {
    RetOps.push_back(Flag);
  }

  return DAG.getNode(CJGISD::RET_FLAG, DL, MVT::Other, RetOps);
}

static SDValue EmitCMP(SDValue &LHS, SDValue &RHS, SDValue &TargetCC,
                       ISD::CondCode CC, const SDLoc &dl, SelectionDAG &DAG) {

  assert(!LHS.getValueType().isFloatingPoint() && "We don't handle FP yet");

  CJGCC::CondCodes TCC = CJGCC::COND_INVALID;
  switch (CC) {
  default: llvm_unreachable("Invalid integer condition!");
  case ISD::SETEQ:
    TCC = CJGCC::COND_Z;     // aka COND_Z
    // Minor optimization: if LHS is a constant, swap operands, then the
    // constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant) {
      std::swap(LHS, RHS);
    }
    break;
  case ISD::SETNE:
    TCC = CJGCC::COND_NZ;    // aka COND_NZ
    // Minor optimization: if LHS is a constant, swap operands, then the
    // constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant) {
      std::swap(LHS, RHS);
    }
    break;
  case ISD::SETULE:
    std::swap(LHS, RHS);
    LLVM_FALLTHROUGH;
  case ISD::SETUGE:
    // Turn lhs u>= rhs with lhs constant into rhs u< lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, dl, C->getValueType(0));
      TCC = CJGCC::COND_NC;
      break;
    }
    TCC = CJGCC::COND_C;    // aka COND_C
    break;
  case ISD::SETUGT:
    std::swap(LHS, RHS);
    LLVM_FALLTHROUGH;
  case ISD::SETULT:
    // Turn lhs u< rhs with lhs constant into rhs u>= lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, dl, C->getValueType(0));
      TCC = CJGCC::COND_C;
      break;
    }
    TCC = CJGCC::COND_NC;    // aka COND_NC
    break;
  case ISD::SETLE:
    std::swap(LHS, RHS);
    LLVM_FALLTHROUGH;
  case ISD::SETGE:
    // Turn lhs >= rhs with lhs constant into rhs < lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, dl, C->getValueType(0));
      TCC = CJGCC::COND_L;
      break;
    }
    TCC = CJGCC::COND_GE;
    break;
  case ISD::SETGT:
    std::swap(LHS, RHS);
    LLVM_FALLTHROUGH;
  case ISD::SETLT:
    // Turn lhs < rhs with lhs constant into rhs >= lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, dl, C->getValueType(0));
      TCC = CJGCC::COND_GE;
      break;
    }
    TCC = CJGCC::COND_L;
    break;
  }

  TargetCC = DAG.getConstant(TCC, dl, MVT::i32);
  return DAG.getNode(CJGISD::CMP, dl, MVT::Glue, LHS, RHS);
}

SDValue CJGTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS   = Op.getOperand(2);
  SDValue RHS   = Op.getOperand(3);
  SDValue Dest  = Op.getOperand(4);
  SDLoc dl  (Op);

  SDValue TargetCC;
  SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

  return DAG.getNode(CJGISD::BR_CC, dl, Op.getValueType(),
                     Chain, Dest, TargetCC, Flag);
}

SDValue CJGTargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDValue LHS    = Op.getOperand(0);
  SDValue RHS    = Op.getOperand(1);
  SDValue TrueV  = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc dl   (Op);

  SDValue TargetCC;
  SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SDValue Ops[] = {TrueV, FalseV, TargetCC, Flag};

  return DAG.getNode(CJGISD::SELECT_CC, dl, VTs, Ops);
}

SDValue CJGTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();
  auto PtrVT = getPointerTy(DAG.getDataLayout());

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op), PtrVT, Offset);
  return DAG.getNode(CJGISD::Wrapper, SDLoc(Op), PtrVT, Result);
}

SDValue CJGTargetLowering::LowerExternalSymbol(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDLoc dl(Op);
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  auto PtrVT = getPointerTy(DAG.getDataLayout());
  SDValue Result = DAG.getTargetExternalSymbol(Sym, PtrVT);

  return DAG.getNode(CJGISD::Wrapper, dl, PtrVT, Result);
}