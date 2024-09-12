// 此工具实时记录执行次数最多的指令

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm> // for sort
#include <vector>
#include "pintool.h"
#include "../../instrument/util/error.h"
#include "../string_convert.h"
#include "../ins_inspection.h"

using std::cerr;
using std::endl;
using std::flush;
using std::setw;
using std::string;
using std::vector;

/* ===================================================================== */
/* Global arguments */
/* ===================================================================== */


string OutFilePath = "/home/myb/pintool_out/opcodetop.txt";
// print histogram after every n BBLs
UINT64 OutThreshold = 1000;
// minimum count for opcode to show
UINT64 Cutoff = 10;
// max number of output lines
UINT64 MaxLines = 24;
// detach after n screen updates (unused)
UINT64 DetachUpdates = 0;
// the factor from the last accounting result, default 0;
FLT64 DecayFactor = 0.0;

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This pin tool provides a real time  opcode mix profile\n"
            "\n";

    cerr << endl;

    return -1;
}

/* ===================================================================== */
/* INDEX HELPERS */
/* ===================================================================== */

const UINT32 MAX_INDEX     = 4096;
const UINT32 INDEX_SPECIAL = 3000;
const UINT32 MAX_MEM_SIZE  = 512;

const UINT32 INDEX_TOTAL          = INDEX_SPECIAL + 0;
const UINT32 INDEX_MEM_ATOMIC     = INDEX_SPECIAL + 1;
/*
const UINT32 INDEX_STACK_READ     = INDEX_SPECIAL + 2;
const UINT32 INDEX_STACK_WRITE    = INDEX_SPECIAL + 3;
const UINT32 INDEX_IPREL_READ     = INDEX_SPECIAL + 4;
const UINT32 INDEX_IPREL_WRITE    = INDEX_SPECIAL + 5;
*/
const UINT32 INDEX_RETURN         = INDEX_SPECIAL + 2;
const UINT32 INDEX_MOV            = INDEX_SPECIAL + 3;
const UINT32 INDEX_DIRECT_CFLOW   = INDEX_SPECIAL + 4;
const UINT32 INDEX_INDIRECT_CFLOW = INDEX_SPECIAL + 5;
const UINT32 INDEX_MEM_READ_SIZE  = INDEX_SPECIAL + 6;
const UINT32 INDEX_MEM_WRITE_SIZE = INDEX_SPECIAL + 6 + MAX_MEM_SIZE;
const UINT32 INDEX_SPECIAL_END    = INDEX_SPECIAL + 6 + MAX_MEM_SIZE + MAX_MEM_SIZE;

BOOL IsMemReadIndex(UINT32 i) { return (INDEX_MEM_READ_SIZE <= i && i < INDEX_MEM_READ_SIZE + MAX_MEM_SIZE); }

BOOL IsMemWriteIndex(UINT32 i) { return (INDEX_MEM_WRITE_SIZE <= i && i < INDEX_MEM_WRITE_SIZE + MAX_MEM_SIZE); }

/* ===================================================================== */

static UINT32 INS_GetIndex(INS ins) { return INS_Opcode(ins); }

/* ===================================================================== */

static UINT32 IndexStringLength(BBL bbl, BOOL memory_acess_profile)
{
    UINT32 count = 0;

    for (INS ins = BBL_InsHead(bbl); INS_Valid(ins); ins = INS_Next(ins))
    {
        count++;
        if (memory_acess_profile)
        {
            if (INS_IsMemoryRead(ins)) count++; // for size
            if (INS_IsMemoryWrite(ins)) count++; // for size
            if (INS_IsAtomicUpdate(ins)) count++;
            if (INS_IsRet(ins)) count++;
            if (INS_IsMov(ins)) count++;
            if (INS_IsDirectControlFlow(ins)) count++;
            if (INS_IsIndirectControlFlow(ins)) count++;
        }
    }

    return count;
}

/* ===================================================================== */
static UINT32 MemsizeToIndex(UINT32 size, BOOL write) { return (write ? INDEX_MEM_WRITE_SIZE : INDEX_MEM_READ_SIZE) + size; }

/* ===================================================================== */
static UINT16* INS_GenerateIndexString(INS ins, UINT16* stats, BOOL memory_acess_profile)
{
    *stats++ = INS_GetIndex(ins);

    if (memory_acess_profile)
    {
        UINT32 readSize = 0, writeSize = 0;
        UINT32 readOperandCount = 0, writeOperandCount = 0;
        for (UINT32 opIdx = 0; opIdx < INS_MemoryOperandCount(ins); opIdx++)
        {
            if (INS_MemoryOperandIsRead(ins, opIdx))
            {
                readSize = INS_MemoryOperandSize(ins, opIdx);
                readOperandCount++;
            }
            if (INS_MemoryOperandIsWritten(ins, opIdx))
            {
                writeSize = INS_MemoryOperandSize(ins, opIdx);
                writeOperandCount++;
            }
        }

        if (readOperandCount > 0) *stats++ = MemsizeToIndex(readSize, 0);
        if (writeOperandCount > 0) *stats++ = MemsizeToIndex(writeSize, 1);

        if (INS_IsAtomicUpdate(ins)) *stats++ = INDEX_MEM_ATOMIC;

        if (INS_IsRet(ins)) *stats++ = INDEX_RETURN;
        if (INS_IsMov(ins)) *stats++ = INDEX_MOV;

        if (INS_IsDirectControlFlow(ins)) *stats++ = INDEX_DIRECT_CFLOW;
        if (INS_IsIndirectControlFlow(ins)) *stats++ = INDEX_INDIRECT_CFLOW;
    }

    return stats;
}

/* ===================================================================== */

static string IndexToOpcodeString(UINT32 index)
{
    if (INDEX_SPECIAL <= index && index < INDEX_SPECIAL_END)
    {
        if (index == INDEX_TOTAL)
            return "*total";
        else if (IsMemReadIndex(index))
            return "*mem-read-" + decstr(index - INDEX_MEM_READ_SIZE);
        else if (IsMemWriteIndex(index))
            return "*mem-write-" + decstr(index - INDEX_MEM_WRITE_SIZE);
        else if (index == INDEX_MEM_ATOMIC)
            return "*mem-atomic";
        else if (index == INDEX_RETURN)
            return "*return";
        else if (index == INDEX_MOV)
            return "*mov";
        else if (index == INDEX_DIRECT_CFLOW)
            return "*direct-controlflow";
        else if (index == INDEX_INDIRECT_CFLOW)
            return "*indirect-controlflow";

        else
        {
            lsassert(0);
            return "";
        }
    }
    else
    {
        return OPCODE_StringShort(index);
    }
}

/* ===================================================================== */
/* ===================================================================== */
typedef UINT64 COUNTER;

/* zero initialized */

class STATS
{
  public:
    COUNTER unpredicated[MAX_INDEX];

    VOID Clear(FLT64 factor)
    {
        for (UINT32 i = 0; i < MAX_INDEX; i++)
        {
            unpredicated[i] = COUNTER(unpredicated[i] * factor);
        }
    }
};

STATS GlobalStats;

class BBLSTATS
{
  public:
    COUNTER _counter;
    const UINT16* const _stats;

  public:
    BBLSTATS(UINT16* stats) : _counter(0), _stats(stats) {};
};

static vector< BBLSTATS* > statsList;

/* ===================================================================== */

static UINT64 bbl_counter = 1000;
// static UINT64 updates     = 0;

/* ===================================================================== */

typedef std::pair< UINT32, UINT64 > PAIR;
typedef vector< PAIR > VEC;

/* ===================================================================== */

static BOOL CompareLess(PAIR s1, PAIR s2) { return s1.second > s2.second; }

/* ===================================================================== */
VOID DumpHistogram(std::ostream& out)
{
    const UINT64 cutoff   = Cutoff;
    const UINT64 maxlines = MaxLines;

    out << ljstr("OPCODE", 25) << " " << setw(16) << "COUNT";
    out << endl;

    for (vector< BBLSTATS* >::iterator bi = statsList.begin(); bi != statsList.end(); bi++)
    {
        BBLSTATS* b = (*bi);

        for (const UINT16* stats = b->_stats; *stats; stats++)
        {
            GlobalStats.unpredicated[*stats] += b->_counter;
        }
        b->_counter = 0;
    }

    COUNTER total = 0;
    VEC CountMap;

    for (UINT32 index = 0; index <= INDEX_SPECIAL_END; index++)
    {
        total += GlobalStats.unpredicated[index];
        if (GlobalStats.unpredicated[index] < cutoff) continue;

        CountMap.push_back(PAIR(index, GlobalStats.unpredicated[index]));
    }
    CountMap.push_back(PAIR(INDEX_TOTAL, total));

    sort(CountMap.begin(), CountMap.end(), CompareLess);
    UINT32 lines = 0;
    for (VEC::iterator bi = CountMap.begin(); bi != CountMap.end(); bi++)
    {
        UINT32 i = bi->first;
        //        out << setw(4) << i << " " <<  ljstr(IndexToOpcodeString(i),15) << " " <<
        out << ljstr(IndexToOpcodeString(i), 25) << " " << setw(16) << bi->second << endl;
        lines++;
        if (lines >= maxlines) break;
    }

    out << endl;
}
/* ===================================================================== */

std::ofstream Out;

/* ===================================================================== */

VOID docount(COUNTER* counter)
{
    (*counter)++;

    if (bbl_counter == 0)
    {
        DumpHistogram(Out);
        Out << flush;

        FLT64 factor = DecayFactor;
        GlobalStats.Clear(factor);

        bbl_counter = OutThreshold;
        /*
        updates++;
        if (updates == DetachUpdates)
        {
            // PIN_Detach();
            Out.close();
        }
        */
    }

    bbl_counter--;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        const INS head = BBL_InsHead(bbl);
        if (!INS_Valid(head)) continue;

        // Summarize the stats for the bbl in a 0 terminated list
        // This is done at instrumentation time
        const UINT32 n = IndexStringLength(bbl, 1);

        UINT16* const stats     = new UINT16[n + 1];
        UINT16* const stats_end = stats + (n + 1);
        UINT16* curr            = stats;

        for (INS ins = head; INS_Valid(ins); ins = INS_Next(ins))
        {
            curr = INS_GenerateIndexString(ins, curr, 1);
        }

        // string terminator
        *curr++ = 0;

        lsassert(curr == stats_end);

        // Insert instrumentation to count the number of times the bbl is executed
        BBLSTATS* bblstats = new BBLSTATS(stats);
        INS_InsertCall(head, IPOINT_BEFORE, AFUNPTR(docount), IARG_PTR, &(bblstats->_counter), IARG_END);

        // Remember the counter and stats so we can compute a summary at the end
        statsList.push_back(bblstats);
    }
}

VOID Fini(int, VOID* v){

    Out.close();

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

    Out.open(OutFilePath);
    Out << "This stores the ins ranking real time" << endl;
    TRACE_AddInstrumentFunction(Trace, 0);
    PIN_AddFiniFunction(Fini, 0);
    // Never returns

    // PIN_StartProgram();

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
