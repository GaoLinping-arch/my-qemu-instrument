#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"


/* INS */
static UINT64 icount = 0;
static int i = 0;

int trace_count()
{
    i++;
    if(i%3 == 0) return 1;
    else return 0;
}

static VOID show_trace()
{
    icount++;
    /*
    char msg[128];
    Ins ins;
    la_disasm(opcode, &ins);
    sprint_ins(&ins, msg);
    fprintf(stderr, "[thread %d]\tpc: 0x%lx\t%s\n", PIN_ThreadId(), pc, msg);
    */
}

static VOID Trace(TRACE trace, VOID* v)
{
    TRACE_InsertIfCall(trace, IPOINT_BEFORE, (AFUNPTR)trace_count, IARG_END);
    TRACE_InsertThenCall(trace,IPOINT_BEFORE, (AFUNPTR)show_trace,  IARG_END);
}

static VOID Init(VOID)
{
}

static VOID Fini(INT32 code, VOID* v)
{
    fprintf(stderr, "ifCall_number: %d\n", i);
    fprintf(stderr, "ThenCall_number: %ld\n", icount);
}
 
static INT32 Usage(void)
{
    return -1;
}
 
int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();
 
    Init();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);
 
    return 0;
}