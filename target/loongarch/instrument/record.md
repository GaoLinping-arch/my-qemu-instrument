(1)translate.c
317 //tr_data.is_jmp = TRANS_NORETURN;
/*new code*/
if(op_is_tr_uncon_branch(ins->op))
    {
        tr_data.is_jmp = TRANS_NORETURN;
    }
    else
    {
        tr_data.is_jmp = TRANS_CON_BRANCH;
    }

(2)instrument.c
34-35//int64_t bound = -(tb->pc | TARGET_PAGE_MASK) / 4;
//max_insns = MIN(max_insns, bound);

/*new code*/
101-112
if(tr_data.is_jmp == TRANS_CON_BRANCH)
        {
            TRACE_append_BBL(trace, bbl);
            tr_data.is_jmp == TRANS_NEXT;
            if(trace->nr_bbl >= 3)
                break;
            bbl = BBL_alloc(pc);
        }
        else if (tr_data.is_jmp == TRANS_NORETURN) {
            TRACE_append_BBL(trace, bbl);
            break;
        }

(3)tr_data.h
54 /*new code*/
typedef enum DisasType {
    TRANS_NEXT,
    TRANS_TOO_MANY,
    TRANS_NORETURN,
    TRANS_CON_BRANCH,
} DisasType;

(4)decoder/ins.c
/*new code*/
bool op_is_tr_uncon_branch(LA_OPCODE op)
{
    return ((LISA_JISCR0 <= op && op <= LISA_BL));
}

bool op_is_tr_con_branch(LA_OPCODE op)
{
    return ((LISA_BEQZ <= op && op <= LISA_BCNEZ) || (LISA_BEQ <= op && op <= LISA_BGEU));
}

(5)ecoder/ins.h
/*new code*/
bool op_is_tr_uncon_branch(LA_OPCODE op);
bool op_is_tr_con_branch(LA_OPCODE op);