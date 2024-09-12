// 此工具记录基本块的控制流变化
#include "pintool.h"
#include "../ins_inspection.h"
#include "../symbol.h"

#include <unordered_map>
#include <vector>
#include <algorithm>

struct bbl_info {
    bbl_info(uint64_t pc) : pc(pc), cnt(0) {}
    uint64_t pc;
    uint64_t next_pc;
    uint64_t cnt;
    const char* tail_ins_name;
};

std::unordered_map<uint64_t, bbl_info *> m;
std::vector<bbl_info *> vec;
//FILE* bbl_info_out;

VOID updateMap(ADDRINT pc, ADDRINT next_pc)
{
    m[pc]->next_pc = next_pc;
}

static VOID Trace(TRACE trace, VOID* v)
{
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        uint64_t pc = bbl->pc;
        INS tailIns = bbl->ins_tail;
        if (m.count(pc) == 0) {
            bbl_info *p = new bbl_info(pc);
            p->tail_ins_name = INS_Mnemonic(tailIns);
            if(INS_IsControlFlow(tailIns)) {
		INS_InsertCall(tailIns,
			       IPOINT_BEFORE,
			       (AFUNPTR)updateMap,
			       IARG_ADDRINT, pc,
			       IARG_BRANCH_TARGET_ADDR,
			       IARG_END);
            } else {
                p->next_pc = INS_NextAddress(tailIns);
            }
            vec.push_back(p);
            m[pc] = p;
        }
        BBL_InsertInlineAdd(bbl, IPOINT_BEFORE, &(m[pc]->cnt), 1, false);
    }
}

static VOID Fini(INT32 code, VOID* v)
{
    std::sort(vec.begin(), vec.end(), [](bbl_info *a, bbl_info * b) { return a->cnt > b->cnt; });
    for (bbl_info * info : vec) {
        fprintf(stderr, "0x%08lx\t%lu\t0x%08lx\t%s      %s      %s\n", info->pc, info->cnt, info->next_pc, info->tail_ins_name, RTN_FindNameByAddress(info->pc), RTN_FindNameByAddress(info->next_pc));
    }
    // fclose(bbl_info_out);
}

static INT32 Usage(void)
{
    return -1;
}

int main(int argc, char* argv[])
{
    // 此处修改输出文件路径
    // bbl_info_out = fopen("/home/dq/my_code/pintool_out/bbl_mix/bbl_ctrlFlow.txt", "w");

    PIN_InitSymbols();

    if (PIN_Init(argc, argv)) return Usage();

    TRACE_AddInstrumentFunction(Trace, 0);

    PIN_AddFiniFunction(Fini, 0);

    return 0;
}
