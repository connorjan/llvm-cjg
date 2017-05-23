set(LLVM_TARGET_DEFINITIONS CJG.td)

tablegen(LLVM CJGGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CJGGenInstrInfo.inc -gen-instr-info)

add_public_tablegen_target(CJGCommonTableGen)

add_llvm_target(CJGCodeGen
  CJGTargetMachine.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
