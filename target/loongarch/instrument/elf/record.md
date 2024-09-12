# 修改配置文件meson.build
loongarch_tcg_ss.add(files(
  'new_elf_parser.c',
  'symbols.c',
))

# 修改symbol.cpp为symbols.c
# 修改symbol.h为symbols.h
# 修改elf_parser.c为new_elf_parser.c
# 修改elf_parser.h为new_elf_parser.h

# 增添测试文件test.c