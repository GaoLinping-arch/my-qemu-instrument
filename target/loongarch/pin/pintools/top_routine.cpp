// 此工具实时记录调用次数的函数

#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm> // for sort
#include <vector>
#include "pintool.h"
#include "../symbols.h"
#include "../ins_inspection.h"
using std::cerr;
using std::endl;
using std::flush;
using std::map;
using std::setw;
using std::string;
using std::vector;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */
// 修改输出文件路径
string OutFilePath = "/home/glp/pintool_out/routinetop.txt";
// print histogram after every n calls
UINT64 OutThreshold = 200;
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
    cerr << "This pin tool \"real time\" tool for showing the currently hostest routines\n"
            "\n";

    cerr << endl;

    return -1;
}

string invalid = "invalid_rtn";

/* ===================================================================== */
const string* Target2String(ADDRINT target)
{
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return &invalid;
    else
        return new string(name);
}

/* ===================================================================== */

typedef map< ADDRINT, UINT64 > ADDR_CNT_MAP;
typedef std::pair< ADDRINT, UINT64 > PAIR;
typedef vector< PAIR > VEC;

static ADDR_CNT_MAP RtnMap;

/* ===================================================================== */

static UINT64 counter = 0;
static UINT64 updates = 0;

std::ofstream Out;

static BOOL CompareLess(PAIR s1, PAIR s2) { return s1.second > s2.second; }

/* ===================================================================== */
VOID DumpHistogram(std::ostream& out)
{
    const UINT64 cutoff   = Cutoff;
    const UINT64 maxlines = MaxLines;
    FLT64 factor          = DecayFactor;

    out << "Functions with at least " << cutoff << " invocations in the last " << OutThreshold << " calls ";
    out << endl;

    VEC CountMap;

    for (ADDR_CNT_MAP::iterator bi = RtnMap.begin(); bi != RtnMap.end(); bi++)
    {
        if (bi->second < cutoff) continue;

        CountMap.push_back(*bi);
#if 0
        out << setw(18) << (void *)(bi->first) << " " <<
            setw(10) << bi->second <<
            "   " << Target2String(bi->first) << endl;
#endif
    }

    sort(CountMap.begin(), CountMap.end(), CompareLess);
    UINT64 lines = 0;
    for (VEC::iterator bi = CountMap.begin(); bi != CountMap.end(); bi++)
    {
        const string* rtn_name = Target2String(bi->first);
        out << setw(18) << (void*)(bi->first) << " " << setw(10) << bi->second << "   " << *rtn_name << endl;
        lines++;
        if (lines >= maxlines) break;
        if (rtn_name != &invalid) delete rtn_name;
    }

    for (ADDR_CNT_MAP::iterator bi = RtnMap.begin(); bi != RtnMap.end(); bi++)
    {
        bi->second = UINT64(bi->second * factor);
    }

    //out << "Total Functions: " << CountMap.size() << endl;
}

/* ===================================================================== */

VOID do_call_indirect(ADDRINT target, BOOL taken)
{
    if (!taken) return;

    if (counter == 0)
    {
        DumpHistogram(Out);
        Out << flush;

        counter = OutThreshold;
        // updates++;
        // if (updates == DetachUpdates)
        // {
        //     PIN_Detach();
        // }
    }

    counter--;

    RtnMap[target]++;
}

/* ===================================================================== */

VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        INS tail = BBL_InsTail(bbl);

        if (INS_IsCall(tail))
        {
            INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect), IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
        }
        /*
        else
        {
            // sometimes code is not in an image
            RTN rtn = TRACE_Rtn(trace);

            // also track stup jumps into share libraries
            if (RTN_Valid(rtn) && !INS_IsDirectControlFlow(tail))
            {
                INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect), IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
                               IARG_END);
            }
        }
        */
    }
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
    TRACE_AddInstrumentFunction(Trace, 0);

    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
