// 此工具记录基本块地址及其跳转指令

#include "pintool.h"
#include "../ins_inspection.h"
#include <stdio.h>

//FILE *checkBblApi;

static VOID WriteBBLTailInsInfo(ADDRINT bblAddress, ADDRINT insAddress, const char* insName)
{
    fprintf(stderr, "BBL Address: %ld, INS Address: %ld, INS Name: %s\n", bblAddress, insAddress, insName);
}

static VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)) {
       for (INS curIns = BBL_InsHead(bbl); INS_Valid(curIns); curIns = INS_Next(curIns)) {
	    if (INS_IsBranch(curIns)) {
                const char* InsName = INS_Mnemonic(curIns);
                BBL_InsertCall(bbl, IPOINT_BEFORE,
                           (AFUNPTR)WriteBBLTailInsInfo,
                           IARG_ADDRINT, BBL_Address(bbl),
                           IARG_ADDRINT, INS_Address(curIns),
                           IARG_PTR, InsName,
                            IARG_END);
	    }
        }
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    // fclose(checkBblApi);
}

static INT32 Usage(void)
{
    return -1;
}

int main(int argc, char* argv[])
{
    // checkBblApi = fopen("/home/dq/my_code/pintool_out/bbl_mix/bbl_info.txt", "w");

    if (PIN_Init(argc, argv)) return Usage();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    return 0;
}
