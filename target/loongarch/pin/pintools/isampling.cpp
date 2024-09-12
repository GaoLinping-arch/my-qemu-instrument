#include "pintool.h"
/* #include "../ins_inspection.h" */
#include <stdio.h>
#include <stdlib.h>

FILE *trace;

const INT32 N = 2000;
const INT32 M = 500;

INT32 icount = N;

/*
VOID IpSample(VOID *ip)
{
    --icount;
    if (icount == 0)
    {
        fprintf(trace, "%p\n", ip);
        icount = N + rand() % M;
    }
}
*/

inline ADDRINT CountDown()
{
    --icount;
    return (icount == 0);
}

// The IP of the current instruction will be printed and
// the icount will be reset to a random number between N and N+M.
VOID PrintIp(VOID* ip)
{

    fprintf(trace, "%p\n", ip);
    
    // Prepare for next period
    icount = N + rand()%M ;
}


VOID Instruction(INS ins, VOID* v)
{
    //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)IpSample, IARG_END);

    INS_InsertIfCall(ins, IPOINT_BEFORE, (AFUNPTR)CountDown, IARG_END);

    INS_InsertThenCall(ins, IPOINT_BEFORE, (AFUNPTR)PrintIp, IARG_INST_PTR, IARG_END);
}

// This function is called when the application exits
VOID Fini(INT32 code, VOID* v)
{
    fprintf(trace, "#eof\n");
    fclose(trace);
}

INT32 Usage()
{
    return -1;
}

int main(int argc, char* argv[])
{
    trace = fopen("isampling.out", "w");

    if (PIN_Init(argc, argv)) return Usage();

    INS_AddInstrumentFunction(Instruction, 0);

    PIN_AddFiniFunction(Fini, 0);


    return 0;
}

