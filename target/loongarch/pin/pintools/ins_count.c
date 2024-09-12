#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"


/* INS */
static UINT64 icount = 0;

static VOID docount()
{
    ++icount;
}

static VOID Instruction(INS ins, VOID* v)
{
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount,IARG_END);
}

static VOID Init(VOID)
{
}

static VOID Fini(INT32 code, VOID* v)
{
    fprintf(stderr, "Total Ins Count: %lu\n", icount);
}
 
static INT32 Usage(void)
{
    return -1;
}
 
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
 
    Init();

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);
 
    return 0;
}
