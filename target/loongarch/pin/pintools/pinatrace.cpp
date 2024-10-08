// 此工具记录程序中读写内存的指令

#include "./pintool.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include "../ins_inspection.h"
// #include <vector>
using std::cerr;
using std::dec;
using std::endl;
using std::hex;
using std::ios;
using std::setw;
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

std::ofstream TraceFile;
// 修改输出文件路径
std::string OutputFilePath = "/home/myb/pintool_out/mem_access/pinatrace.txt";
// 判断是否记录读取或写入内存的值
BOOL OutputMemVal = 1;
// std::vector<std::string> ins_info;
std::unordered_map<UINT64, std::string> ins_info;
/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

static INT32 Usage()
{
    cerr << "This tool produces a memory address trace.\n"
            "For each (dynamic) instruction reading or writing to memory the the ip and ea are recorded\n"
            "\n";

   // cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

static VOID EmitMem(VOID* ea, INT32 size)
{
    if (!OutputMemVal) return;

    switch (size)
    {
        case 0:
            TraceFile << setw(1);
            break;

        case 1:
            TraceFile << static_cast< UINT32 >(*static_cast< UINT8* >(ea));
            break;

        case 2:
            TraceFile << *static_cast< UINT16* >(ea);
            break;

        case 4:
            TraceFile << *static_cast< UINT32* >(ea);
            break;

        case 8:
            TraceFile << *static_cast< UINT64* >(ea);
            break;

        default:
            TraceFile.unsetf(ios::showbase);
            TraceFile << setw(1) << "0x";
            for (INT32 i = 0; i < size; i++)
            {
                TraceFile << static_cast< UINT32 >(static_cast< UINT8* >(ea)[i]);
            }
            TraceFile.setf(ios::showbase);
            break;
    }
}

static VOID RecordMem(ADDRINT ip, CHAR r, VOID* addr, INT32 size, BOOL isPrefetch)
{
    TraceFile << hex << ip << ": " << r << " " << setw(2 + 2 * sizeof(ADDRINT)) << addr << " " << dec << setw(2) << size << " " << hex
              << setw(2 + 2 * sizeof(ADDRINT));
    if (!isPrefetch) EmitMem(addr, size);
    TraceFile << " " << ins_info[ip] << endl;
    // InsInfoFileForAnalysis << iind << " " << ins_info[ip] << endl;
}

static VOID* WriteAddr;
static INT32 WriteSize;

static VOID RecordWriteAddrSize(VOID* addr, INT32 size)
{
    WriteAddr = addr;
    WriteSize = size;
}

static VOID RecordMemWrite(ADDRINT ip){ RecordMem(ip, 'W', WriteAddr, WriteSize, false); }

static VOID RecordMemWriteForTB(ADDRINT ip, BOOL taken_branch)
{
    if (taken_branch)
        RecordMem(ip, 'W', WriteAddr, WriteSize, false);
}
VOID Instruction(INS ins, VOID* v)
{
    // instruments loads using a predicated call, i.e.
    // the call happens iff the load will be actually executed
    // std::string iname = INS_Mnemonic(ins);
    std::string iname = INS_Disassemble(ins);
    ADDRINT iaddr = INS_Address(ins);
    ins_info.insert(std::unordered_map<UINT64, std::string>::value_type(iaddr, iname));

    if (INS_IsMemoryRead(ins))
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMem, IARG_ADDRINT, iaddr, IARG_UINT32, 'R', IARG_MEMORYREAD_EA,
                                 IARG_MEMORYREAD_SIZE, IARG_BOOL, INS_IsPrefetch(ins), IARG_END);
    }
    // instruments stores using a predicated call, i.e.
    // the call happens iff the store will be actually executed

    if (INS_IsMemoryWrite(ins))
    {
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordWriteAddrSize, IARG_MEMORYWRITE_EA, IARG_MEMORYWRITE_SIZE,
                                 IARG_END);

        if (INS_IsValidForIpointAfter(ins))
        {
            INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)RecordMemWrite, IARG_ADDRINT, iaddr, IARG_END);
        }
        if (INS_IsControlFlow(ins))
        {
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)RecordMemWriteForTB, IARG_INST_PTR, IARG_BRANCH_TAKEN, IARG_ADDRINT, iaddr, IARG_END);
        }

    }
}

/* ===================================================================== */

VOID Fini(INT32 code, VOID* v)
{
    TraceFile << "#eof" << endl;

    TraceFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    std::string trace_header = std::string("#\n"
                                 "# Memory Access Trace Generated By Pin\n"
                                 "#\n");

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    TraceFile.open(OutputFilePath);
    TraceFile.write(trace_header.c_str(), trace_header.size());
    TraceFile.setf(ios::showbase);

    RecordMemWrite(0);
    RecordWriteAddrSize(0, 0);
    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    // PIN_StartProgram();


    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
