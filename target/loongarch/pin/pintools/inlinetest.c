#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"


// 全局变量
UINT64 count0 = 0;
UINT64 count1 = 0;

// 优化后的 docount 函数
static inline void docount(int taken) {
    //if(taken) count1++;
    //else count0++;
    count1 += taken;
    count0 += (1 - taken);
}

// Pin的回调函数，每次指令被执行时调用
static VOID Instruction(INS ins, VOID *v) {
    // 在这里插入分析函数，在条件跳转指令后插入
    if (INS_IsBranch(ins) || INS_IsCall(ins) ) {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_BRANCH_TAKEN, IARG_END);
    }
}

// This function is called when the application exits
static VOID Fini(INT32 code, VOID* v)
{
    fprintf(stderr, "count1:%ld\n,count0:%ld\n", count1, count0);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    return 0;
}