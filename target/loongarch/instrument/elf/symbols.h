#ifndef _ELF_SYMBOLS_H_
#define _ELF_SYMBOLS_H_
#include "../tr_data.h"
#include "../../pin/instrumentation_arguements.h"

#define MAX_IMG_NR 64
#define MAX_IMG_RTN_NR (512*64)
#define MAX_CB_NUM 8

typedef struct pin_rtn {
    const char *name;
    uint64_t addr;
    uint64_t size;
    uint32_t id;
    INS ins_head;
    INS ins_tail;
    struct pin_rtn *next;
    struct pin_rtn *prev;
    
    ANALYSIS_CALL *rtn_entry_cbs;
    int entry_cbs_num;
    ANALYSIS_CALL *rtn_exit_cbs;
    int exit_cbs_num; 
} *RTN;

typedef struct pin_img {
    const char *path;
    uintptr_t load_base;
    uint32_t id;
    RTN img_rtn;
    struct pin_img *next;
    struct pin_img *prev;
    int rtn_num;
} *IMG;

extern int IMG_id, RTN_id;
extern struct pin_img IMG_array[MAX_IMG_NR];
extern struct pin_rtn RTN_array[MAX_IMG_NR][MAX_IMG_RTN_NR];

#ifdef __cplusplus
extern "C" {
#endif
    IMG IMG_alloc(const char *path, uintptr_t load_base);
    RTN RTN_alloc(IMG image, const char *name, uint64_t addr, uint64_t size);
    void image_add_symbol(IMG image, const char * name, uint64_t addr, uint64_t size);
    IMG get_img_by_id(UINT32 id);
    IMG get_img_by_rtn(RTN rtn);
    RTN image_get_symbol_by_name(IMG image, const char *name);
    RTN get_symbol_by_pc(uint64_t pc);
    const char *get_symbol_name_by_pc(uint64_t pc);
    void print_collected_symbols(void);


#ifdef __cplusplus
}
#endif

#endif
