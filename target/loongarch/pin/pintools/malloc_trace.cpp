// 此工具查找程序中对malloc和free的调用，记录参数及其返回值

#include "pintool.h"
#include "../symbol.h"
#include <iostream>
#include <fstream>
using std::cerr;
using std::cout;
using std::endl;
using std::hex;
using std::ios;
using std::string;

/* ===================================================================== */
/* Names of malloc and free */
/* ===================================================================== */
// 可以用来查找其他函数，增加宏定义即可
#if defined(TARGET_MAC)
#define MALLOC "_malloc"
#define FREE "_free"
#else
#define MALLOC "malloc"
#define FREE "free"
#endif

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */
// 设置输出文件路径
string OutFilePath = "/home/myb/pintool_out/func_call/malloc_trace.txt";
/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a trace of calls to malloc.\n"
            "\n";

    cerr << endl;

    return -1;
}

/* ===================================================================== */

VOID Arg1Before(CHAR* name, ADDRINT size) { TraceFile << name << "(" << size << ")" << endl; }

/* ===================================================================== */

VOID MallocAfter(ADDRINT ret) { TraceFile << "  returns " << ret << endl; }

/* ===================================================================== */

VOID Image(IMG img, VOID* v)
{
    RTN mallocRtn = RTN_FindByName(img, MALLOC);
    if (RTN_Valid(mallocRtn))
    {
        RTN_Open(mallocRtn);
        RTN_InsertCall(mallocRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before, IARG_ADDRINT, MALLOC, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_InsertCall(mallocRtn, IPOINT_AFTER, (AFUNPTR)MallocAfter, IARG_FUNCRET_EXITPOINT_VALUE, IARG_END);
        RTN_Close(mallocRtn);
    }

    RTN freeRtn = RTN_FindByName(img, FREE);
    if (RTN_Valid(freeRtn))
    {
        RTN_Open(freeRtn);
        RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before, IARG_ADDRINT, FREE, IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
                       IARG_END);
        RTN_Close(freeRtn);
    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v) { TraceFile.close(); }

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(OutFilePath);

    TraceFile << hex;
    TraceFile.setf(ios::showbase);

    cout << hex;
    cout.setf(ios::showbase);

    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
