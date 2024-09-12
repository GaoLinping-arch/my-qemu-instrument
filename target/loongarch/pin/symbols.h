#ifndef PIN_SYMBOLS_H
#define PIN_SYMBOLS_H

#include "types.h"
#include "../instrument/elf/symbols.h"
#include "instrumentation_arguements.h"

#ifdef __cplusplus
extern "C" {
#endif
    /*API of IMG*/
    IMG IMG_Next(IMG img);
    IMG IMG_Prev(IMG img);
    IMG IMG_Invalid(void);
    BOOL IMG_Valid(IMG img);
    RTN IMG_RtnHead(IMG img);
    RTN IMG_RtnTail(IMG img);
    const char * IMG_Name(IMG img);
    UINT32 IMG_Id(IMG img); //start from 1
    IMG IMG_FindImgById(UINT32);


    /*API of RTN*/
    /* In Pin, RTN_FindNameByAddress return std::string */
    IMG RTN_Img(RTN rtn);
    RTN RTN_Next(RTN rtn);
    RTN RTN_Prev(RTN rtn);
    RTN RTN_Invaild(void);
    BOOL RTN_Valid(RTN rtn);
    const char * RTN_Name(RTN rtn);
    UINT32 RTN_Id(RTN rtn); //start from 1
    USIZE RTN_Size(RTN rtn);
    const CHAR *RTN_FindNameByAddress(ADDRINT address);
    RTN RTN_FindByAddress(ADDRINT address);
    RTN RTN_FindByName(IMG img, const CHAR *name);
    VOID RTN_Open(RTN rtn);
    VOID RTN_Close(RTN rtn);
    UINT32 RTN_NumIns(RTN rtn);
    ADDRINT RTN_Address(RTN rtn);
    


    /* === 下面为内部实现所需接口 === */
    //VOID RTN_add_entry_cb(RTN rtn, ANALYSIS_CALL *cb);
    //ANALYSIS_CALL *RTN_get_entry_cbs(uintptr_t pc, int *cnt);
    //VOID RTN_add_exit_cb(RTN rtn, ANALYSIS_CALL *cb);
    //ANALYSIS_CALL *RTN_get_exit_cbs(uintptr_t pc, int *cnt);
#ifdef __cplusplus
}
#endif

#endif
