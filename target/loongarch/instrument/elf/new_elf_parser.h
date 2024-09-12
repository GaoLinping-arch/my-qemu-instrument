#ifndef NEW_ELF_PARSER_H
#define NEW_ELF_PARSER_H

#include <stdint.h>
#include "symbols.h"
void parse_elf_symbol(const char* pathname, uint64_t map_base, IMG* pp_img);
void parse_elf_symbol_with_fd(int fd, uint64_t map_base, IMG* pp_img);

#endif
