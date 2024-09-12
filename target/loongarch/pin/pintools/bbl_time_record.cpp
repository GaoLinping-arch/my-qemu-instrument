// 这个工具记录基本块的运行时间和运行次数
#include "pintool.h"
#include "../ins_inspection.h"
#include "../symbol.h"
#include <time.h>

#include <unordered_map>
#include <vector>
#include <algorithm>

struct bbl_info {
    bbl_info(uint64_t pc) : pc(pc), cnt(0) {}
    uint64_t pc;
    uint64_t cnt;
    unsigned int exec_time_once;
    unsigned int bbl_size;
};

std::unordered_map<uint64_t, bbl_info *> m;
std::vector<bbl_info *> vec;
static struct timespec tv_enter = {0, 0};
static struct timespec tv_exit = {0, 0};
//FILE* bbl_exec_time_record;

VOID enter_bbl()
{
    clock_gettime(CLOCK_MONOTONIC, &tv_enter);
}
VOID exit_bbl(ADDRINT pc)
{
    if(m[pc]->cnt > 1) {
        return;
    } else {
        clock_gettime(CLOCK_MONOTONIC, &tv_exit);
        // exectime stored in ms
        m[pc]->exec_time_once = tv_exit.tv_nsec - tv_enter.tv_nsec;
    }
}

static VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        uint64_t pc = BBL_Address(bbl);
        if (m.count(pc) == 0) {
            bbl_info *p = new bbl_info(pc);
            p->bbl_size = BBL_Size(bbl);
            vec.push_back(p);
            m[pc] = p;
        }
        BBL_InsertInlineAdd(bbl, IPOINT_BEFORE, &(m[pc]->cnt), 1, false);
        BBL_InsertCall(bbl, IPOINT_BEFORE, (AFUNPTR)enter_bbl, IARG_END);
        INS tailIns = BBL_InsTail(bbl);
        INS_InsertCall(tailIns, IPOINT_BEFORE, (AFUNPTR)exit_bbl, IARG_ADDRINT, BBL_Address(bbl), IARG_END);
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    std::sort(vec.begin(), vec.end(), [](bbl_info *a, bbl_info * b) { return a->cnt > b->cnt; });
    for (bbl_info * info : vec) {
        fprintf(stderr, "0x%08lx\t%lu\t%u\t%u      %s\n",
                info->pc,
                info->cnt,
                info->exec_time_once,
                info->bbl_size,
                RTN_FindNameByAddress(info->pc));
    }
}

static INT32 Usage(void)
{
    return -1;
}

int main(int argc, char* argv[])
{
    //bbl_exec_time_record = fopen("/home/dq/my_code/pintool_out/bbl_mix/bbl_time_record.txt", "w");

    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    return 0;
}
