#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"

static VOID Instruction(INS ins, VOID* v)
{
    UINT32 idx[5];
    char msg[128];
    Ins myins;
    la_disasm(ins->opcode, &myins);
    sprint_ins(&myins, msg);
    int mem_opnd_count = INS_MemoryOperandCount (ins);
    fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s has %d memory oprand\n", PIN_ThreadId(), ins->pc, msg, mem_opnd_count);
    if(mem_opnd_count == 1) mem_opnd_count +=1;  
        for(int i=0; i< mem_opnd_count; i++)
        {
             idx[i] = INS_MemoryOperandIndexToOperandIndex (ins, i);
             fprintf(stderr,"memidx:%d to idx:%d\n",i,idx[i]);
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