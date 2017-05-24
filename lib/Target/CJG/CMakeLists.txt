set(LLVM_TARGET_DEFINITIONS CJG.td)

tablegen(LLVM CJGGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM CJGGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM CJGGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM CJGGenAsmWriter.inc -gen-asm-writer)

add_public_tablegen_target(CJGCommonTableGen)

add_llvm_target(CJGCodeGen
  CJGTargetMachine.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
