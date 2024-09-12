#include "new_elf_parser.h"
#include "symbols.h"
#include <stdio.h>

static void find_symbol(uint64_t pc)
{
    RTN sym = get_symbol_by_pc(pc);
    if (sym) { printf("0x%lx: is in function %s\n", pc, sym->name);} 
    else { printf("0x%lx: fail to find symbol\n", pc); }
}

int main()
{
    char str[] = "/lib/x86_64-linux-gnu/libc.so.6";
    //char str[] = "hello";
    IMG img;
    parse_elf_symbol(str, 0, &img);
    print_collected_symbols();
    
    RTN sym = image_get_symbol_by_name(img, "getpt");
    find_symbol(sym->addr + sym->size - 4);
    find_symbol(sym->addr + sym->size);
}

