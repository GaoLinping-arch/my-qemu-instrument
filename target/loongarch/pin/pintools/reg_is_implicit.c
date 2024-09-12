#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"

static VOID Instruction(INS ins, VOID* v)
{
    char msg[128];
    Ins myins;
    la_disasm(ins->opcode, &myins);
    sprint_ins(&myins, msg);
    if(INS_RegIsImplicit(ins, REG_RA) || INS_RegIsImplicit(ins, REG_T2))
        fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s\n", PIN_ThreadId(), ins->pc, msg);
    
}

static VOID Fini(INT32 code, VOID* v)
{
}
 
static INT32 Usage(void)
{
    return -1;
}
 
int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);
    
    return 0;
}