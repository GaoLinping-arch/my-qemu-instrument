#include "pintool.h"
#include "../ins_inspection.h"
#include <assert.h>

/* BBL */
static uint64_t trace_exec_nr = 0;
static uint64_t bbl_exec_nr = 0;
static uint64_t ins_exec_nr = 0;

static UINT64 tr_c = 0;
static UINT64 bbl_c = 0;

static VOID docount(uint64_t ins_nr) {
    trace_exec_nr ++;
    ins_exec_nr += ins_nr;
}

static VOID count()
{
    bbl_exec_nr ++;
}

static VOID Trace(TRACE trace, VOID* v)
{

        TRACE_InsertCall(trace, IPOINT_BEFORE, (AFUNPTR)docount, IARG_UINT64, TRACE_NumIns(trace), IARG_END);
        tr_c++;
        for(BBL bbl=TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
        {
                BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)count, IARG_END);
                bbl_c++;
        }

}


static VOID Fini(INT32 code, VOID* v)
{
    fprintf(stderr, "EXEC: TRACE: %ld, BBL: %ld, INS: %ld\n", trace_exec_nr, bbl_exec_nr, ins_exec_nr);

    fprintf(stderr, "TRACE: %ld, BBL: %ld\n", tr_c, bbl_c);
}

static INT32 Usage(void)
{
    return -1;
}

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv)) return Usage();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    return 0;
}