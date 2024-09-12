// 此文件记录程序读写通用寄存器的次数

#include "pintool.h"
#include "../string_convert.h"
#include "../ins_inspection.h"
#include "../reg.h"
#include <list>
#include <iostream>
#include <fstream>
using std::cerr;
using std::endl;
using std::list;
using std::string;

/* ===================================================================== */
// 修改输出文件路径
string OutFilePath = "/home/glp/pintool_out/reg_mix/reg_mix.txt";
/* ===================================================================== */

#define reg_rw_w(rw) (unsigned char)(rw & 0xff)
#define reg_rw_r1(rw) (unsigned char)((rw >> 8) & 0xff)
#define reg_rw_r2(rw) (unsigned char)((rw >> 16) & 0xff)
#define reg_rw_r3(rw) (unsigned char)((rw >> 24) & 0xff)

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool computes a dynamic register usage mix profile\n"
            "\n";

    // cerr << KNOB_BASE::StringKnobSummary();

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

const UINT16 MAX_REG = 4096;

typedef UINT64 COUNTER;

struct GLOBALSTATS
{
    COUNTER reg_r[MAX_REG];
    COUNTER reg_w[MAX_REG];
} GlobalStats;

class BBLSTATS
{
  public:
    BBLSTATS(UINT16* stats) : _stats(stats), _counter(0) {};

    const UINT16* _stats;
    COUNTER _counter;
};

list< const BBLSTATS* > statsList;

/* ===================================================================== */

VOID ComputeGlobalStats()
{
    // We have the count for each bbl and its stats, compute the summary
    for (list< const BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        for (const UINT16* stats = (*bi)->_stats; *stats; stats++)
        {
            GlobalStats.reg_r[*stats] += (*bi)->_counter;
        }
    }
}

/* ===================================================================== */
// 可以修改此函数，从而记录更多类型的寄存器
UINT64 GetInsGPR(INS ins) {
    // 需要根据 ISA 的寄存器相关知识进一步修改
    UINT64 destination_registers = 0;
    for (int i = 0; i < ins->origin_ins->opnd_count; ++i) {
        LISA_REG_ACCESS_TYPE access_type = get_reg_access_type(ins->origin_ins, i);
        if (access_type == GPR_WRITE || access_type == GPR_READWRITE) {
            destination_registers = ins->origin_ins->opnd[i].val;
            break;
        }
    }
    int read_index = 0;
    UINT64 source_registers[4] = {0};
    for (int i = 0; i < ins->origin_ins->opnd_count; ++i) {
        LISA_REG_ACCESS_TYPE access_type = get_reg_access_type(ins->origin_ins, i);
        if (access_type == GPR_READ || access_type == GPR_READWRITE) {
            source_registers[read_index] = ins->origin_ins->opnd[i].val;
            read_index ++;
        }
    }
    return destination_registers | source_registers[0] << 8 | source_registers[1] << 16 | source_registers[2] << 24;
}

/* ===================================================================== */

UINT16 REG_GetStatsIndex(REG reg, BOOL is_write)
{
    if (is_write)
        return MAX_REG + reg;
    else
        return reg;
}

/* ===================================================================== */

VOID docount(COUNTER* counter) { (*counter)++; }

INT32 RecordRegisters(BBL bbl, UINT16* stats)
{
    INT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        UINT64 AllRegIndex = GetInsGPR(ins);
        UINT64 RegWriteIndex = reg_rw_w(AllRegIndex);
        UINT64 RegRead1Index = reg_rw_r1(AllRegIndex);
        UINT64 RegRead2Index = reg_rw_r2(AllRegIndex);
        UINT64 RegRead3Index = reg_rw_r3(AllRegIndex);
        if (RegWriteIndex > 0)
            stats[count++] = REG_GetStatsIndex(gpr_to_REG(RegWriteIndex), true);
        if (RegRead1Index > 0)
            stats[count++] = REG_GetStatsIndex(gpr_to_REG(RegRead1Index), false);
        if (RegRead2Index > 0)
            stats[count++] = REG_GetStatsIndex(gpr_to_REG(RegRead2Index), false);
        if (RegRead3Index > 0)
            stats[count++] = REG_GetStatsIndex(gpr_to_REG(RegRead3Index), false);
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
        UINT16 buffer[128 * 1024];
        INT32 count = RecordRegisters(bbl, buffer);
        lsassert(count < 128 * 1024);

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        UINT16* stats = new UINT16[count];

        RecordRegisters(bbl, stats);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(BBL_InsHead(bbl), IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

/* ===================================================================== */
static std::ofstream* out = 0;
VOID Fini(int, VOID* v)
{
    ComputeGlobalStats();

    *out << "#\n"
            "#num reg  count-read  count-written\n"
            "#\n";

    for (UINT32 i = 0; i < MAX_REG; i++)
    {
        if (GlobalStats.reg_w[i] == 0 && GlobalStats.reg_r[i] == 0) continue;

        *out << decstr(i - 3, 3) << " " << ljstr(REG_StringShort(REG(i)), 6) << decstr(GlobalStats.reg_r[i], 6)
             << decstr(GlobalStats.reg_w[i], 12) << endl;
    }

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

    // Never returns

    // PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
