#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"


/* INS */
static UINT64 trace_count = 0;

static VOID show_ins(UINT64 pc, UINT32 opcode)
{
    /* print ins */
    char msg[128];
    Ins ins;
    la_disasm(opcode, &ins);
    sprint_ins(&ins, msg);
    fprintf(stderr, "pc: 0x%lx\t%s\n", pc, msg);
}

static VOID Trace(TRACE trace, VOID* v)
{
    trace_count ++ ;
    fprintf(stderr,"The %ld trace is:\n",trace_count);
    int bbl_num = 0;
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        bbl_num ++ ;
        fprintf(stderr,"The %d bbl is:\n",bbl_num);
        for(INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            show_ins(ins->pc, ins->opcode);
        }
        //BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)bbl_statistic, IARG_UINT64, BBL_NumIns(bbl), IARG_END);
    }
    fprintf(stderr,"\n");
}

static VOID Init(VOID)
{
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
    if (PIN_Init(argc, argv)) return Usage();
 
    Init();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);
 
    return 0;
}
