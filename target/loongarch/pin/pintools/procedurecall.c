#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"

/* INS */
static VOID check(UINT64 pc, UINT32 opcode)
{
    /* print ins */
    char msg[128];
    Ins ins;
    la_disasm(opcode, &ins);
    sprint_ins(&ins, msg);
    fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s\n", PIN_ThreadId(), pc, msg);
}

static VOID Instruction(INS ins, VOID* v)
{
    if(INS_IsProcedureCall(ins))
        INS_InsertCall(ins,IPOINT_BEFORE, (AFUNPTR)check, IARG_UINT64, ins->pc, IARG_UINT64, ins->opcode, IARG_END);
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