#include "pintool.h"
#include "../symbols.h"
#include "../../instrument/elf/symbols.h"

static VOID Image(IMG img, VOID* v)
{
    const char* r_name = RTN_Name(IMG_RtnTail(img));
    uint64_t r_addr = RTN_Address(IMG_RtnTail(img));
    for(RTN rtn = IMG_RtnHead(img); RTN_Valid(rtn); rtn = RTN_Next(rtn))
    {
        UINT32 rtn_id = RTN_Id(rtn);
        const char *rtn_name = RTN_Name(rtn);
        USIZE rtn_size = RTN_Size(rtn);
        UINT32 rtn_numins = RTN_NumIns(rtn);
        uint64_t rtn_addr = RTN_Address(rtn);
        fprintf(stderr,"RTN_id   RTN_name   RTN_size   RTN_numins   RTN_addr\n");
        fprintf(stderr,"   %d        %s         %d         %d         %ld\n", rtn_id, rtn_name, rtn_size, rtn_numins, rtn_addr);
    }
    RTN r1 = RTN_FindByAddress(r_addr);
    uint64_t check_rtn_addr = RTN_Address(r1);
    RTN r2 = RTN_FindByName(img, r_name);
    const char *check_rtn_name = RTN_Name(r2);
    fprintf(stderr,"%ld : The last RTN is %s\n", check_rtn_addr, check_rtn_name); 
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
