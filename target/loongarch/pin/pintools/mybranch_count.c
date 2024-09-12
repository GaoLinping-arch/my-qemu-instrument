#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"

/* INS */
static VOID recording(UINT64 pc, UINT32 opcode, ADDRINT ip, ADDRINT addr)
{
    /* print ins */
    char msg[128];
    Ins ins;
    la_disasm(opcode, &ins);
    sprint_ins(&ins, msg);
    fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s\n", PIN_ThreadId(), pc, msg);
    fprintf(stderr,"from %ld to %ld\n", ip, addr);
}

static VOID Instruction(INS ins, VOID* v)
{
    if(INS_IsBranch(ins))
        INS_InsertCall(ins,IPOINT_TAKEN_BRANCH, (AFUNPTR)recording, IARG_UINT64, ins->pc, IARG_UINT64, ins->opcode, IARG_INST_PTR,IARG_BRANCH_TARGET_ADDR,IARG_END);
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