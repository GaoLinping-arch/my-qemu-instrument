#include "symbols.h"
#include <string.h>
#include "../util/error.h"
//#define MAX_PIN_IMG_NR 50

int IMG_id = 0 ;
int RTN_id = 0 ;

struct pin_img IMG_array[MAX_IMG_NR];
struct pin_rtn RTN_array[MAX_IMG_NR][MAX_IMG_RTN_NR];

IMG IMG_alloc(const char *path, uintptr_t load_base)
{
    //IMG img = (IMG)malloc(sizeof(struct pin_img));
    //if (img == NULL) return NULL;
    lsassertm(IMG_id +1 < MAX_IMG_NR, "too many IMGS\n");
    IMG img = &(IMG_array[++IMG_id]);
    img->path = path;
    img->load_base = load_base;
    img->id = IMG_id;
    img->img_rtn = NULL;
    if(IMG_id <= 1){
        img->prev = NULL;
    }
    else{
        img->prev = &(IMG_array[IMG_id - 1]);
        img->prev->next = img;
    }
    img->next = NULL;
    return img;
}

RTN RTN_alloc(IMG image, const char *name, uint64_t addr, uint64_t size)
{
    //RTN rtn = (RTN)malloc(sizeof(struct pin_rtn));
    //if (rtn == NULL) return NULL;
    lsassertm(image->id < MAX_IMG_NR, "The IMG does not exist\n");
    lsassertm(image->rtn_num+1 < MAX_IMG_RTN_NR, "too many RTNS\n");
    RTN rtn = &(RTN_array[image->id][++image->rtn_num]);
    rtn->name = name;
    rtn->addr = addr;
    rtn->size = size;
    RTN_id += 1;
    rtn->id = RTN_id;
    //rtn->image = image;
    rtn->ins_head = NULL;
    rtn->ins_tail = NULL;
    
    rtn->entry_cbs_num = 0;
    rtn->exit_cbs_num = 0;
    
    if(image->rtn_num <= 1){
        rtn->prev = NULL;
    }
    else{
        rtn->prev = &(RTN_array[image->id][image->rtn_num - 1]);
        rtn->prev->next = rtn;
    }
    rtn->next = NULL;
    return rtn;
}

void image_add_symbol(IMG image, const char * name, uint64_t addr, uint64_t size)
{
   RTN rtn = RTN_alloc(image, name, addr, size);
   if (rtn->addr == 0 || rtn->size == 0) {
        lsdebug("Find a symbol with invalid addr or size. name: %s, addr: 0x%lx, size: 0x%lx\n", name, addr, size);
    }
    if(image->img_rtn == NULL){
        image->img_rtn = RTN_array[image->id];
    }
}

IMG get_img_by_id(UINT32 id)
{
    IMG img;
    for (int i = 1; i <= IMG_id; i++)
    {
        if(IMG_array[i].id == id)
        {
            img = &(IMG_array[i]);
            return img;
        }
    }
    return NULL;
}

IMG get_img_by_rtn(RTN rtn)
{
    IMG img;
    for(int i = 1; i <= IMG_id; i++)
        for(int j = 1; j <= IMG_array[i].rtn_num; j++)
        {
            if(IMG_array[i].img_rtn[j].id == rtn->id)
            {
                img = &(IMG_array[i]);
                return img;
            }
        }
    return NULL;
}

/*It may find many rtns, just return the first one*/
RTN image_get_symbol_by_name(IMG image, const char *name)
{
    RTN rtn;
    for(int i = 1; i<= image->rtn_num; i++)
    {
        if (strcmp(RTN_array[image->id][i].name, name) == 0)
        {
            rtn = &(RTN_array[image->id][i]);
            return rtn;
        }
    }
    return NULL;
}

RTN get_symbol_by_pc(uint64_t pc)
{
    RTN rtn;
    for(int i = 1; i <= IMG_id; i++)
        for(int j = 1; j <= IMG_array[IMG_id].rtn_num; j++)
        {
            if (RTN_array[i][j].addr <= pc && pc < RTN_array[i][j].addr + RTN_array[i][j].size)
            {
                rtn = &(RTN_array[i][j]);
                return rtn;
            }
        }
    return NULL;
}

const char *get_symbol_name_by_pc(uint64_t pc)
{
    const char *name = "";
    RTN rtn = get_symbol_by_pc(pc);
    if(rtn != NULL) name = rtn->name;
    return name;
}

void print_collected_symbols(void)
{
    for(int i = 1; i <= IMG_id; i++)
    {
        printf("ID      ADDR      SIZE      NAME\n");
        for(int j = 1; j <= IMG_array[IMG_id].rtn_num; j++)
        {
            struct pin_rtn rtn = RTN_array[i][j];
            printf("%-16d     0x%-16lx    0x%-16lx    %s\n", rtn.id, rtn.addr, rtn.size, rtn.name);
        }
        printf("The %d Image: %s (%d symbols)\n", IMG_array[i].id, IMG_array[i].path, IMG_array[i].rtn_num);
    }
}

