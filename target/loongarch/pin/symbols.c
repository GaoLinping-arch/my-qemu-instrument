#include "symbols.h"
#include <stdio.h>
#include <string.h>
#include "../instrument/elf/symbols.h"
#include "../instrument/util/error.h"

/* API of IMG */
IMG IMG_Next(IMG img)
{
    if(IMG_Valid(img)) return img->next;
    return NULL;
}

IMG IMG_Prev(IMG img)
{
    if(IMG_Valid(img)) return img->prev;
    return NULL;
}

IMG IMG_Invalid(void)
{
    return NULL;
}

BOOL IMG_Valid(IMG img)
{
    if(img != NULL) return true;
    return false;
}

RTN IMG_RtnHead(IMG img)
{
    if(IMG_Valid(img)) return &(img->img_rtn[1]);
    return NULL;
}

RTN IMG_RtnTail(IMG img)
{
    if(IMG_Valid(img)) return &(img->img_rtn[img->rtn_num]);
    return NULL;
}

const char *IMG_Name(IMG img)
{
    const char *name = "";
    if(IMG_Valid(img)) name = img->path;
    return name;
}

UINT32 IMG_Id(IMG img)
{
    if(IMG_Valid(img)) return img->id;
    return 0;
}

IMG IMG_FindImgById(UINT32 id)
{
    IMG img = get_img_by_id(id);
    return img;
}

/* API of RTN */
IMG RTN_Img(RTN rtn)
{
    IMG img = get_img_by_rtn(rtn);
    return img;
}

RTN RTN_Next(RTN rtn)
{
    if(rtn) return rtn->next;
    return NULL;
}

RTN RTN_Prev(RTN rtn)
{
    if(rtn) return rtn->prev;
    return NULL;
}

RTN RTN_Invaild(void)
{
    return NULL;
}

BOOL RTN_Valid(RTN rtn)
{
    if(rtn != NULL) return true;
    else return false;
}

const char *RTN_Name(RTN rtn)
{
    const char *name = "";
    if(rtn) return rtn->name;
    return name;
}

UINT32 RTN_Id(RTN rtn)
{
    if(rtn) return rtn->id;
    return 0;
}

USIZE RTN_Size(RTN rtn)
{
    if(rtn) return rtn->size;
    return 0;
}

const CHAR *RTN_FindNameByAddress(ADDRINT address)
{
    return get_symbol_name_by_pc(address);
}

RTN RTN_FindByAddress(ADDRINT address)
{
    RTN sym = get_symbol_by_pc(address);
    return sym;
}

RTN RTN_FindByName(IMG img, const CHAR *name)
{
    RTN sym = image_get_symbol_by_name(img, name);
    return sym;
}

VOID RTN_Open(RTN rtn)
{

}

VOID RTN_Close(RTN rtn)
{

}

UINT32 RTN_NumIns(RTN rtn)
{
    UINT32 ins_num = 0;
    if(rtn) ins_num = rtn->size / 4;
    return ins_num;
}

ADDRINT RTN_Address(RTN rtn)
{
    if(rtn) return rtn->addr;
    return 0;
}





