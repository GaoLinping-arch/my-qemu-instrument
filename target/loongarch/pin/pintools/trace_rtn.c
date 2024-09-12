#include "pintool.h"
#include "../../instrument/decoder/disasm.h"
#include "../../instrument/decoder/la_print.h"
#include "../ins_inspection.h"
#include "../symbols.h"
#include "../../instrument/elf/symbols.h"

static VOID Trace(TRACE trace, VOID* v)
{
    RTN rtn = TRACE_Rtn(trace);
    UINT32 rtn_id = RTN_Id(rtn);
    const char *rtn_name = RTN_Name(rtn);
    fprintf(stderr,"RTN_id:%d   RTN_name:%s \n",rtn_id, rtn_name);
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

    //IMG_AddInstrumentFunction(Image, 0);
    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);
 
    return 0;
}
