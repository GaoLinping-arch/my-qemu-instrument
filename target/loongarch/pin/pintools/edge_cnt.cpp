// 此工具记录控制流变化的详细信息
#include "pintool.h"
#include "../ins_inspection.h"

#include <iostream>
#include <fstream>
#include <map>
#include <unistd.h>
using std::cerr;
using std::endl;
using std::map;
using std::string;

static INT32 Usage()
{
    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
// nibble用于筛选指令类型，默认为否，表示不分析共享库中的控制流变化
const INT32 nibble = -1;
// 修改输出文件路径
std::string OutputPath = "/home/myb/pintool_out/control_flow/edge_count.txt";

class COUNTER
{
  public:
    UINT64 _count; // number of times the edge was traversed

    COUNTER() : _count(0) {}
};

typedef enum
{
    ETYPE_INVALID,
    ETYPE_CALL,
    ETYPE_ICALL,
    ETYPE_BRANCH,
    ETYPE_IBRANCH,
    ETYPE_RETURN,
    ETYPE_SYSCALL,
    ETYPE_LAST
} ETYPE;

class EDGE
{
  public:
    ADDRINT _src;
    ADDRINT _dst;
    ADDRINT _next_ins;
    ETYPE _type; // must be integer to make stl happy

    EDGE(ADDRINT s, ADDRINT d, ADDRINT n, ETYPE t) : _src(s), _dst(d), _next_ins(n), _type(t) {}

    bool operator<(const EDGE& edge) const { return _src < edge._src || (_src == edge._src && _dst < edge._dst); }
};

string StringFromEtype(ETYPE etype)
{
    switch (etype)
    {
        case ETYPE_CALL:
            return "Call";
        case ETYPE_ICALL:
            return "IndirectCall";
        case ETYPE_BRANCH:
            return "Branch";
        case ETYPE_IBRANCH:
            return "IndirectBranch";
        case ETYPE_RETURN:
            return "Return";
        case ETYPE_SYSCALL:
            return "Syscall";
        default:
            return "INVALID";
    }
}

typedef map< EDGE, COUNTER* > EDG_HASH_SET;

static EDG_HASH_SET EdgeSet;

/* ===================================================================== */

/*!
  An Edge might have been previously instrumented, If so reuse the previous entry
  otherwise create a new one.
 */

static COUNTER* Lookup(EDGE edge)
{
    COUNTER*& ref = EdgeSet[edge];

    if (ref == 0)
    {
        ref = new COUNTER();
    }

    return ref;
}

/* ===================================================================== */

VOID docount(COUNTER* pedg) { pedg->_count++; }

/* ===================================================================== */
// for indirect control flow we do not know the edge in advance and
// therefore must look it up

VOID docount2(ADDRINT src, ADDRINT dst, ADDRINT n, ETYPE type, INT32 taken)
{
    if (!taken) return;
    COUNTER* pedg = Lookup(EDGE(src, dst, n, type));
    pedg->_count++;
}

VOID docount3(COUNTER* pedg, INT32 taken)
{
    if(!taken) return;
    pedg->_count++;
}

/* ===================================================================== */

VOID Instruction(INS ins, void* v)
{
    if (INS_IsDirectControlFlow(ins))
    {
        ETYPE type = INS_IsCall(ins) ? ETYPE_CALL : ETYPE_BRANCH;

        // static targets can map here once
        COUNTER* pedg = Lookup(EDGE(INS_Address(ins), INS_DirectControlFlowTargetAddress(ins), INS_NextAddress(ins), type));
        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_ADDRINT, pedg, IARG_BRANCH_TAKEN, IARG_END);
    }
    else if (INS_IsIndirectControlFlow(ins))
    {
        ETYPE type = ETYPE_IBRANCH;

        if (INS_IsRet(ins))
        {
            type = ETYPE_RETURN;
        }
        else if (INS_IsCall(ins))
        {
            type = ETYPE_ICALL;
        }

        INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)docount2, IARG_INST_PTR, IARG_BRANCH_TARGET_ADDR, IARG_ADDRINT,
                       INS_NextAddress(ins), IARG_UINT32, type, IARG_BRANCH_TAKEN, IARG_END);
    }
    else if (INS_IsSyscall(ins))
    {
        COUNTER* pedg = Lookup(EDGE(INS_Address(ins), ADDRINT(~0), INS_NextAddress(ins), ETYPE_SYSCALL));
        INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)docount, IARG_ADDRINT, pedg, IARG_END);
    }
}

/* ===================================================================== */

inline INT32 AddressHighNibble(ADDRINT addr) { return 0xf & (addr >> (sizeof(ADDRINT) * 8 - 4)); }

/* ===================================================================== */

VOID Fini(int n, void* v)
{

    std::ofstream OutFile(OutputPath);

    OutFile << "EDGCOUNT        4.0         0\n"; // profile header, no md5sum
    UINT32 count = 0;

    for (EDG_HASH_SET::const_iterator it = EdgeSet.begin(); it != EdgeSet.end(); it++)
    {
        const std::pair< EDGE, COUNTER* > tuple = *it;
        // skip inter shared lib edges

        if (nibble >= 0 && nibble != AddressHighNibble(tuple.first._dst) && nibble != AddressHighNibble(tuple.first._src))
        {
            continue;
        }

        if (tuple.second->_count == 0) continue;

        count++;
    }

    OutFile << "EDGs " << count << endl;
    OutFile << "# src          dst        type    count     next-ins\n";
    OutFile << "DATA:START" << endl;

    for (EDG_HASH_SET::const_iterator it = EdgeSet.begin(); it != EdgeSet.end(); it++)
    {
        const std::pair< EDGE, COUNTER* > tuple = *it;

        // skip inter shared lib edges

        if (nibble >= 0 && nibble != AddressHighNibble(tuple.first._dst) && nibble != AddressHighNibble(tuple.first._src))
        {
            continue;
        }

        if (tuple.second->_count == 0) continue;

        OutFile << std::hex << tuple.first._src << " " << std::hex << tuple.first._dst << " "
             << StringFromEtype(tuple.first._type) << " " << tuple.second->_count << " "
             << tuple.first._next_ins << endl;
    }

    OutFile << "DATA:END" << endl;
    OutFile << "## eof\n";
    OutFile.close();
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

    INS_AddInstrumentFunction(Instruction, 0);
    PIN_AddFiniFunction(Fini, 0);

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
