// 此工具记录不同类型指令的执行次数

#include "pintool.h"
#include "../string_convert.h"
#include "../ins_inspection.h"
#include <list>
#include <iostream>
#include <fstream>
#include <stdlib.h>

using std::cerr;
using std::endl;
using std::list;
using std::ofstream;
using std::string;


/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes the dynamic instruction regimentation mix profile\n"
            "\n";

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

const UINT32 MAX_INDEX = 64;

typedef UINT64 COUNTER;

/* zero initialized */

typedef struct
{
    COUNTER unpredicated[MAX_INDEX];
    COUNTER predicated[MAX_INDEX];
    COUNTER predicated_true[MAX_INDEX];
} STATS;

STATS GlobalStatsDynamic;

class BBLSTATS
{
  public:
    BBLSTATS(UINT16* stats) : _stats(stats), _counter(0) {};

    const UINT16* _stats;
    COUNTER _counter;
};

list< const BBLSTATS* > statsList;

string InsVariety(UINT16 index)
{
    string result = "";
    switch (index)
    {
    case 1:
        result = "Return";
        break;
    case 2:
        result = "Interrupt";
        break;
    case 3:
        result = "Direct ControlFlow";
        break;
    case 4:
        result = "Indirect ControlFlow";
        break;
    case 5:
        result = "Memory Read";
        break;
    case 6:
        result = "Memory Write";
        break;
    case 7:
        result = "Atomic Update";
        break;
    case 8:
	result = "Mov data to regs";
	break;
    case 9:
	result = "Other instructions";
	break;
    default:
        break;
    }
    return result;
}
/* ===================================================================== */
// 可以在此增加对指令类型的筛选，相应的，需要修改函数InsVariety
UINT16 NewIndex(INS ins)
{
    if (INS_IsRet(ins))
        return 1;
    else if (INS_IsInterrupt(ins))
        return 2;
    else if (INS_IsDirectControlFlow(ins))
        return 3;
    else if (INS_IsIndirectControlFlow(ins))
        return 4;
    else if (INS_IsMemoryRead(ins))
        return 5;
    else if (INS_IsMemoryWrite(ins))
        return 6;
    else if (INS_IsAtomicUpdate(ins))
        return 7;
    else if (INS_IsMov(ins))
        return 8;
    else
        return 9;
}

BOOL INS_IsPredicated(INS ins)
{
    return INS_GetPredicate(ins) != PREDICATE_ALWAYS_TRUE;
}
VOID ComputeGlobalStats()
{
    // We have the count for each bbl and its stats, compute the summary
    for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        // _stats ends with number 0
        for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
        {
            GlobalStatsDynamic.unpredicated[*stats] += (*bi)->_counter;
        }
    }
}

/* ===================================================================== */
UINT16 INS_GetStatsIndex(INS ins)
{
    if (INS_IsPredicated(ins))
        return MAX_INDEX + NewIndex(ins);
    else
        return NewIndex(ins);
}

/* ===================================================================== */

VOID docount(COUNTER* counter) { (*counter)++; }

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        UINT16* stats = new UINT16[BBL_NumIns(bbl) + 1];

        INT32 index = 0;
        for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
        {
            // Count the number of times a predicated instruction is actually executed
            // this is expensive and hence disabled by default
            if (INS_IsPredicated(ins))
            {
                INS_InsertPredicatedCall(ins, IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR,
                                         &(GlobalStatsDynamic.predicated_true[NewIndex(ins)]), IARG_END);
            }

            stats[index++] = INS_GetStatsIndex(ins);
        }
        stats[index] = 0;

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
VOID DumpStats(ofstream& out, STATS& stats, const string& title)
{
    out << "#\n"
           "# "
        << title
        << "\n"
           "#\n"
           "#num       regimentation count-unpredicated count-predicated count-predicated-true\n"
           "#\n";

    UINT64 total_instructions = 0;
    for (UINT32 i = 0; i < MAX_INDEX; i++)
    {
        if (stats.unpredicated[i] == 0 && stats.predicated[i] == 0) continue;

        out << decstr(i, 3) << " " << ljstr(InsVariety(i), 20) << decstr(stats.unpredicated[i], 12)
            << decstr(stats.predicated[i], 15) << decstr(stats.predicated_true[i], 19) << endl;

        total_instructions += stats.unpredicated[i];
    }
    out << "the number of instructions unpredicated: " << total_instructions << endl;
}

/* ===================================================================== */
static std::ofstream* out = 0;

VOID Fini(int, VOID* v)
{
    ComputeGlobalStats();

    DumpStats(*out, GlobalStatsDynamic, "dynamic counts");

    *out << "# eof" << endl;

    out->close();
}


/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    if (PIN_Init(argc, argv))
    {
        return Usage();
    }
    // 此处修改输出文件路径
    string OutputFilePath = "/home/glp/pintool_out/ins_mix/dynamic_ins_mix.txt";

    out = new std::ofstream(OutputFilePath);

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);
    // Never returns

    // PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
