gen = decodetree.process('insns.decode')

loongarch_ss = ss.source_set()
loongarch_ss.add(files(
  'cpu.c',
  'disas.c',
))
loongarch_tcg_ss = ss.source_set()
loongarch_tcg_ss.add(gen)
loongarch_tcg_ss.add(files(
  'fpu_helper.c',
  'op_helper.c',
  'translate.c',
  'gdbstub.c',
))
loongarch_tcg_ss.add(zlib)

subdir('instrument')
subdir('pin')

loongarch_softmmu_ss = ss.source_set()
loongarch_softmmu_ss.add(files(
  'machine.c',
  'constant_timer.c',
  'tlb_helper.c',
  'csr_helper.c',
  'iocsr_helper.c',
))

loongarch_ss.add_all(when: 'CONFIG_TCG', if_true: [loongarch_tcg_ss])

target_arch += {'loongarch': loongarch_ss}
target_softmmu_arch += {'loongarch': loongarch_softmmu_ss}
