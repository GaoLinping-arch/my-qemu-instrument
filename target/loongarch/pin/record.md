# 修改配置文件meson.build
loongarch_tcg_ss.add(files(
  'ins_inspection.c',
  'ins_instrumentation.c',
  'pin_state.c',
  'thread.c',
  'symbols.c',   /*symbol.cpp->symbols.c*/
  'reg.c',
  'loader.c',
))

# 删除ins_instrument.c中的函数：
（1）static void _RTN_InsertCall(INS INS, ANALYSIS_CALL *cb)
（2）VOID RTN_InsertCall(RTN rtn, IPOINT action, AFUNPTR funptr, ...);
（3）VOID RTN_instrument(TRACE trace);

# 修改symbol.cpp为symbols.c
# 修改symbol.h为symbols.h
# 将types.h中引的头文件"../instrument/elf/symbol.h"改为../instrument/elf/symbols.h

