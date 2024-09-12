/* ref: https://stackoverflow.com/questions/34960383/how-read-elf-header-in-c */
#include "new_elf_parser.h"
#include <elf.h>
#include <stdio.h>
#include <string.h>
#include "symbols.h"
#include "../util/error.h"

#include <fcntl.h>
#include <unistd.h>
#include <alloca.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

/* lookup .symtab and .dynsym section */
static bool find_sym_table(Elf64_Shdr *shdr, int shnum, Elf64_Word type, int *sym_idx, int *str_idx)
{
    /* type can be SHT_SYMTAB or SHT_DYNSYM */
    for (int i = 0; i < shnum; ++i) {
        if (shdr[i].sh_type == type) {
            *sym_idx = i;
            *str_idx = shdr[i].sh_link;
            return true;
        }
    }
    return false;
}

void parse_elf_symbol_with_fd(int fd, uint64_t map_base, IMG *pp_img)
{
    char filepath[1024];
    char link[64];

    sprintf(link, "/proc/self/fd/%d", fd);
    int n = readlink(link, filepath, 1024); //将link内容存到filepath中，n是字符串的字符数
    if (n < 0 || n >= 1024) {            
        fprintf(stderr, "get file path failed\n");
        return;
    }
    filepath[n] = 0;    /* readlink不会给字符串结尾加\0 */

    lsdebug("read symbol in %s\n", filepath);
    parse_elf_symbol(filepath, map_base, pp_img);
}

/* parse ELF and print all symbols() */
void parse_elf_symbol(const char* pathname, uint64_t map_base, IMG *pp_img)
{
    Elf64_Ehdr *ehdr = NULL;
    int shnum;    //表示Section Header的个数
    size_t shsz;  //表示Section Header的大小
    Elf64_Shdr *shdr = NULL;
    uint64_t secsz;  //Section的大小
    int sym_idx;     //所有符号所在Section的索引
    int str_idx;     //所有符号名所在Section的索引
    Elf64_Sym *syms = NULL;  //存储所有Symbol的信息
    char *strs = NULL;  //存储所有Symbol的名字
    int nsyms;      //获取Symbol的个数
    *pp_img = NULL;

    int fd = open(pathname, O_RDONLY, 0);
    if (fd < 0) {
        /* printf("Error while loading %s: %s\n", filename, strerror(errno)); */
        perror("open file failed\n");
        return;
    }

    /* 1. read ELF header */
    ehdr = (Elf64_Ehdr *)alloca(sizeof(Elf64_Ehdr));
    if (pread(fd, ehdr, sizeof(Elf64_Ehdr), 0) != sizeof(Elf64_Ehdr)) {
        fprintf(stderr, "read elf header failed\n");
        goto give_up;
    }

    /* 2. check if is an ELF shared object or exec */
    // TODO add more check
    if (!((memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0) && ((ehdr->e_type == ET_DYN) || (ehdr->e_type == ET_EXEC)))) {
        fprintf(stderr, "not a shared object\n");
        goto give_up;
    }

    /* 3. read section headers */
    shnum = ehdr->e_shnum;
    shsz = shnum * ehdr->e_shentsize;
    shdr = (Elf64_Shdr *)alloca(shsz);
    if (pread(fd, shdr, shsz, ehdr->e_shoff) != shsz) {
        fprintf(stderr, "read section headers failed\n");
        goto give_up;
    }

    /* 4. lookup .symtab and .dynsym section */
    /* only need one of both, because dynsym is included in symtab */
    if (!find_sym_table(shdr, shnum, SHT_SYMTAB, &sym_idx, &str_idx)
        && !find_sym_table(shdr, shnum, SHT_DYNSYM, &sym_idx, &str_idx)) {
        fprintf(stderr, "find no symbol table\n");
        goto give_up;
    }

    /* 4.1 read symbol string */
    /* TODO add assert strtab exist */
    secsz = shdr[str_idx].sh_size;
    strs = (char *)malloc(secsz);  //strs中存储所有symbol的名字
    if (!strs || pread(fd, strs, secsz, shdr[str_idx].sh_offset) != secsz) {
        fprintf(stderr, "read .symstr section failed\n");
        goto give_up;
    }

    /* 4.2 read symbol table */
    secsz = shdr[sym_idx].sh_size;
    syms = (Elf64_Sym *)malloc(secsz);  //syms中存储所有symbol的信息
    if (!syms || pread(fd, syms, secsz, shdr[sym_idx].sh_offset) != secsz) {
        fprintf(stderr, "read .symtab section failed\n");
        goto give_up;
    }

    if (secsz / sizeof(Elf64_Sym) > INT_MAX) {
        fprintf(stderr, "Implausibly large symbol table, give up\n");
        goto give_up;
    }

    /* 4.3 scan symbol table, collect functions */
    *pp_img = IMG_alloc(pathname, map_base);
    nsyms = secsz / sizeof(Elf64_Sym);
    for (int i = 0; i < nsyms; ++i) {
        /* Throw away entries which we do not need.  */
        if (syms[i].st_shndx == SHN_UNDEF
            || syms[i].st_shndx >= SHN_LORESERVE
            || ELF64_ST_TYPE(syms[i].st_info) != STT_FUNC) {
            continue;
        }
        /* printf("find symbol: %p: %s\n", (void *)(map_base + syms[i].st_value), strs + syms[i].st_name); */
        image_add_symbol(*pp_img, strs + syms[i].st_name, map_base + syms[i].st_value, syms[i].st_size);
    }
give_up:
    free(strs);
    free(syms);
    close(fd);
}
