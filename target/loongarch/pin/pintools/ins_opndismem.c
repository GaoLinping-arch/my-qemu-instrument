#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"

static VOID Instruction(INS ins, VOID* v)
{
    BOOL is_mem[5];
    char msg[128];
    Ins myins;
    la_disasm(ins->opcode, &myins);
    sprint_ins(&myins, msg);
    int opnd_count = INS_OperandCount (ins);
    fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s\t has %d oprand(s)\n", PIN_ThreadId(), ins->pc, msg, opnd_count);
    for(int i=0; i< opnd_count; i++)
    {
        is_mem[i] = INS_OperandIsMemory (ins, i);
        fprintf(stderr,"The result of %d operand is %d\n",i ,is_mem[i]);
    } 
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