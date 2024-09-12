// 此工具统计控制流变化的次数

#include "pintool.h"
#include "../ins_inspection.h"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <map>
#include <unistd.h>

using std::cerr;
using std::endl;
using std::setw;
using std::string;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */
// 在此修改输出文件路径
string OutputFilePath = "/home/glp/pintool_out/control_flow/jump_mix.txt";
/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

static INT32 Usage()
{
    cerr << "This pin tool collects a profile of jump/branch/call instructions for an application\n";

    cerr << endl;
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

class COUNTER
{
  public:
    UINT64 _call;
    UINT64 _call_indirect;
    UINT64 _return;
    UINT64 _syscall;
    UINT64 _branch;
    UINT64 _branch_indirect;

    COUNTER() : _call(0), _call_indirect(0), _return(0), _branch(0), _branch_indirect(0) {}

    UINT64 Total() { return _call + _call_indirect + _return + _syscall + _branch + _branch_indirect; }
};

COUNTER CountSeen;
COUNTER CountTaken;

#define INC(what)                     \
    VOID inc##what(INT32 taken)       \
    {                                 \
        CountSeen.what++;             \
        if (taken) CountTaken.what++; \
    }

INC(_call)
INC(_call_indirect)
INC(_branch)
INC(_branch_indirect)
INC(_syscall)
INC(_return)

/* ===================================================================== */

VOID Instruction(INS ins, void* v)
{
    if (INS_IsRet(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_return, IARG_BRANCH_TAKEN, IARG_END);
    }
    else if (INS_IsSyscall(ins))
    {
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_syscall, IARG_BRANCH_TAKEN, IARG_END);
    }
    else if (INS_IsDirectControlFlow(ins))
    {
        if (INS_IsCall(ins))
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_call, IARG_BRANCH_TAKEN, IARG_END);
        else
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_branch, IARG_BRANCH_TAKEN, IARG_END);
    }
    else if (INS_IsIndirectControlFlow(ins))
    {
        if (INS_IsCall(ins))
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_call_indirect, IARG_BRANCH_TAKEN, IARG_END);
        else
            INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)inc_branch_indirect, IARG_BRANCH_TAKEN, IARG_END);
    }
}

/* ===================================================================== */

#define OUT(n, a, b) *out << n << " " << a << setw(12) << CountSeen.b << " " << setw(16) << CountTaken.b << endl

static std::ofstream* out = 0;

VOID Fini(int n, void* v)
{
    *out << "# JUMPMIX\n";
    *out << "#\n";
    *out << "# $dynamic-counts\n";
    *out << "#\n";
    *out << "                       jump_count       jump_taken\n";
    *out << "0 *total      " << setw(16) << CountSeen.Total() << " " << setw(16) << CountTaken.Total() << endl;

    OUT(1, "call            ", _call);
    OUT(2, "indirect-call   ", _call_indirect);
    OUT(3, "branch          ", _branch);
    OUT(4, "indirect-branch ", _branch_indirect);
    OUT(5, "syscall         ", _syscall);
    OUT(6, "return          ", _return);

    *out << "#\n";
    *out << "# eof\n";
    out->close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    out = new std::ofstream(OutputFilePath);

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    // PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
