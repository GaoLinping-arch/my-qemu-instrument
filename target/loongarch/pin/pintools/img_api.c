#include "pintool.h"
#include "../symbols.h"
#include "../../instrument/elf/symbols.h"

static VOID Image(IMG img, VOID* v)
{
    if(IMG_Valid(img))
        fprintf(stderr,"The %d IMG is %s\n", IMG_Id(img), IMG_Name(img));
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
