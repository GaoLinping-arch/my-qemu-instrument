// 此工具对读写内存的指令进行统计

/* ===================================================================== */
#include "pintool.h"
#include <list>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdlib.h>
#include <string.h>

#include "../../instrument/util/error.h"
#include "../string_convert.h"
#include "../ins_inspection.h"
using std::cerr;
using std::endl;
using std::list;
using std::ostream;
using std::showpoint;
using std::string;

/* ===================================================================== */
/* Options */
/* ===================================================================== */
// 在此修改输出文件路径
string OutFilePath = "/home/glp/pintool_out/ins_mix/ldstmix.txt";

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a dynamic register/memory pattern mix profile\n"
            "\n";

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
UINT32 MaxNumThreads = 1;

typedef UINT64 COUNTER;

typedef enum
{
    PATTERN_INVALID,
    PATTERN_MEM_RW,
    PATTERN_MEM_R,
    PATTERN_MEM_W,
    PATTERN_OTHERS,
    PATTERN_MOV,
    PATTERN_LAST
} pattern_t;

char const* pattern_t2str(pattern_t x)
{
    switch (x)
    {
        case PATTERN_INVALID:
            return "INVALID";
        case PATTERN_MEM_RW:
            return "memory_read_and_write";
        case PATTERN_MEM_R:
            return "memory_read_only";
        case PATTERN_MEM_W:
            return "memory_write_only";
        case PATTERN_OTHERS:
            return "others";
        case PATTERN_MOV:
            return "mov";
        case PATTERN_LAST:
            return "LAST";
    }
    lsassert(0);
    /* NOTREACHED */
    return 0;
}

typedef struct
{
    COUNTER pattern[PATTERN_LAST];
} STATS;

STATS GlobalStats;

class BBLSTATS
{
  public:
    BBLSTATS(UINT16* stats) : _stats(stats), _counter(0) {};

    //array of uint16, one per instr in the block, 0 terminated
    const UINT16* _stats;

    // one ctr per thread to avoid runtime locking at the expense of memory
    COUNTER _counter;
};

list< const BBLSTATS* > statsList;

//////////////////////////////////////////////////////////

/* ===================================================================== */

VOID ComputeGlobalStats()
{
    for (UINT32 i = 0; i < PATTERN_LAST; i++)
        GlobalStats.pattern[i] = 0;

    // We have the count for each bbl and its stats, compute the summary
    for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
        for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
                GlobalStats.pattern[*stats] += (*bi)->_counter;
}

/* ===================================================================== */

/* ===================================================================== */

VOID docount(COUNTER* counter) { (*counter)++; }

INT32 RecordRegisters(BBL bbl, UINT16* stats)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        bool rmem   = INS_IsMemoryRead(ins);
        bool wmem   = INS_IsMemoryWrite(ins);
        bool rw_mem = rmem & wmem;
        if (rw_mem)
            stats[count++] = PATTERN_MEM_RW;
        else if (rmem)
            stats[count++] = PATTERN_MEM_R;
        else if (wmem)
            stats[count++] = PATTERN_MEM_W;
        else if (INS_IsMov(ins))
            stats[count++] = PATTERN_MOV;
        else
            stats[count++] = PATTERN_OTHERS;
    }

    stats[count++] = 0;

    return count;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        // Record the registers into a dummy buffer so we can count them
        UINT16 *stats = new UINT16[BBL_NumIns(bbl) + 1];
        RecordRegisters(bbl, stats);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}


static std::ofstream* out = 0;

VOID Fini(int, VOID* v)
{
    ComputeGlobalStats();

    *out << "#\n"
            "#pattern-type count percent\n"
            "#\n";

    *out << "All Threads" << endl;
    COUNTER total = 0;
    for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
        total += GlobalStats.pattern[i];

    *out << std::setprecision(4) << showpoint;
    for (int i = PATTERN_INVALID + 1; i < PATTERN_LAST; i++)
        *out << ljstr(pattern_t2str(static_cast< pattern_t >(i)), 25) << decstr(GlobalStats.pattern[i], 12) << "\t"
             << std::setw(10) << 100.0 * GlobalStats.pattern[i] / total << std::endl;

    *out << endl;

    *out << "# eof" << endl;

    out->close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, CHAR* argv[])
{
    PIN_InitSymbols();

    if (PIN_Init(argc, argv))
    {
        return Usage();
    }

    out = new std::ofstream(OutFilePath);

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
