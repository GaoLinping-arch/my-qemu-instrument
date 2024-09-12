/*#include "pintool.h"
#include "../symbol.h"
#include "../../instrument/elf/symbol.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::ofstream;
using std::setw;
using std::string;

ofstream outFile;

// Holds instruction count for a single procedure
typedef struct RtnCount
{
    string _name;
    //int _id;
    int _size;
    int num_ins;
    //string _sec; //added code
    //string _image;
    ADDRINT _address;
    //RTN _rtn;
    UINT64 _rtnCount;
    //UINT64 _icount;
    struct RtnCount* _next;
} RTN_COUNT;

// Linked list of instruction counts for each routine
RTN_COUNT* RtnList = 0;

// This function is called before every instruction is executed
static VOID docount(UINT64* counter) { (*counter)++; }

static const char* StripPath(const char* path)
{
    const char* file = strrchr(path, '/');
    if (file)
        return file + 1;
    else
        return path;
}

// Pin calls this function every time a new rtn is executed
static VOID Routine(RTN rtn, VOID* v)
{
    fprintf(stderr,"I am doing Routine\n");
    // Allocate a counter for this routine
    RTN_COUNT* rc = new RTN_COUNT;

    // The RTN goes away when the image is unloaded, so save it now
    // because we need it in the fini
    rc->_name     = RTN_Name(rtn);
    //rc->_id       = RTN_Id(rtn);
    rc->_size     = RTN_Size(rtn);
    rc->num_ins   = RTN_NumIns(rtn);
    //rc->_sec      = SEC_Name(RTN_Sec(rtn));
    //rc->_image    = StripPath(IMG_Name(SEC_Img(RTN_Sec(rtn))).c_str());
    rc->_address  = RTN_Address(rtn);
    //rc->_icount   = 0;
    rc->_rtnCount = 0;

    // Add to list of routines
    rc->_next = RtnList;
    RtnList   = rc;

    //RTN_Open(rtn);

    // Insert a call at the entry point of a routine to increment the call count
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_rtnCount), IARG_END);

    // For each instruction of the routine
    //for (INS ins = RTN_InsHead(rtn); INS_Valid(ins); ins = INS_Next(ins))
    //{
        // Insert a call to docount to increment the instruction counter for this rtn
        //INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_PTR, &(rc->_icount), IARG_END);
    //}

    //RTN_Close(rtn);
}

// This function is called when the application exits
// It prints the name and count for each procedure
static VOID Fini(INT32 code, VOID* v)
{
    RTN invalid_rtn = RTN_Invalid();
    RTN_COUNT* invalid_rc = new RTN_COUNT;
    //if(invalid_rtn->name == NULL) invalid_rc->_name = "NULL";
    //invalid_rc->_id = -1;
    invalid_rc->_address  = RTN_Address(invalid_rtn);
    invalid_rc->_icount   = 0;
    invalid_rc->_rtnCount = 0;

    // Add to list of routines
    invalid_rc->_next = RtnList;
    RtnList   = invalid_rc;

    
    int num_rtn = 0;
    for (RTN_COUNT* rc = RtnList; rc; rc = rc->_next)
    {
        num_rtn ++ ;
        outFile << "The" << num_rtn << "RTN is:" << endl;
        //if (rc->_icount > 0)
            outFile << setw(30) << rc->_name << " " << setw(15) << rc->_size << " " << setw(15) << rc->num_ins << " "  << setw(18) << hex << rc->_address << dec
                    << " " << setw(12) << rc->_rtnCount <<endl;
    }
}


static INT32 Usage()
{
    return -1;
}

int main(int argc, char* argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    outFile.open("proccount.txt");

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    fprintf(stderr,"I am doing RTN_AddInstrumentFunction\n");
    // Register Routine to be called to instrument rtn,only execute once
    RTN_AddInstrumentFunction(Routine, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    //PIN_StartProgram();

    return 0;
}
*/