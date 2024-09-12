#include "pintool.h"
#include "../symbols.h"
#include "../../instrument/elf/symbols.h"

static VOID Image(IMG img, VOID* v)
{
    fprintf(stderr,"The %d IMG is %s\n", IMG_Id(img), IMG_Name(img));
    RTN rtn = IMG_RtnTail(img);
    const char *rtn_name = RTN_Name(rtn);
    UINT32 rtn_id = RTN_Id(rtn);
    USIZE rtn_size = RTN_Size(rtn);
    UINT32 rtn_numins = RTN_NumIns(rtn);
    uint64_t rtn_addr = RTN_Address(rtn);
    fprintf(stderr,"RTN_id   RTN_name   RTN_size   RTN_numins   RTN_addr\n");
    fprintf(stderr,"   %d        %s         %d         %d         %ld\n", rtn_id, rtn_name, rtn_size, rtn_numins, rtn_addr);
    
    RTN r = RTN_FindByAddress(rtn_addr);
    const char * r_name = RTN_FindNameByAddress(rtn_addr);
    UINT32 r_id = RTN_Id(r);
    fprintf(stderr,"The %d RTN is %s\n", r_id, r_name);
    
    IMG i = RTN_Img(r);
    const char * i_name = IMG_Name(i);
    fprintf(stderr,"The IMG is %s\n", i_name);
    fprintf(stderr,"\n");
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
