#include "pintool.h"
#include "../symbols.h"
#include "../../instrument/elf/symbols.h"

static VOID Image(IMG img, VOID* v)
{
    const char* NAME = "malloc";

    RTN rtn = IMG_RtnHead(img);
    rtn = RTN_Next(RTN_Next(rtn));
    int rtn_id = RTN_Id(rtn);
    const char *rtn_name = RTN_Name(rtn);
    fprintf(stderr, "First: The name of the %d RTN is %s\n", rtn_id, rtn_name);

    rtn = RTN_FindByName(img, NAME);
    int id = RTN_Id(rtn);
    const char *name = RTN_Name(rtn);
    fprintf(stderr, "Second: The name of the %d RTN is %s\n", id, name);
}

static VOID Fini(INT32 code, VOID* v) {
    /* print_collected_symbols(); */
}

static INT32 Usage(void)
{
    return -1;
}
 
int main(int argc, char* argv[])
{
    // Initialize pin & symbol manager
    PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();
    // Register Image to be called to instrument functions.
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);
    return 0;
}