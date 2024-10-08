/*
 * Common System Call Tracing Wrappers for *-user
 *
 * Copyright (c) 2019 Linaro
 * Written by Alex Bennée <alex.bennee@linaro.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SYSCALL_TRACE_H
#define SYSCALL_TRACE_H

#include "trace/trace-root.h"
#ifdef CONFIG_LMJ
#include "target/loongarch/pin/pin_state.h"
#endif

/*
 * These helpers just provide a common place for the various
 * subsystems that want to track syscalls to put their hooks in. We
 * could potentially unify the -strace code here as well.
 */

static inline void record_syscall_start(void *cpu, int num,
                                        abi_long arg1, abi_long arg2,
                                        abi_long arg3, abi_long arg4,
                                        abi_long arg5, abi_long arg6,
                                        abi_long arg7, abi_long arg8)
{
    trace_guest_user_syscall(cpu, num,
                             arg1, arg2, arg3, arg4,
                             arg5, arg6, arg7, arg8);
    qemu_plugin_vcpu_syscall(cpu, num,
                             arg1, arg2, arg3, arg4,
                             arg5, arg6, arg7, arg8);
#ifdef CONFIG_LMJ
    syscall_instrument(cpu);
#endif
}

static inline void record_syscall_return(void *cpu, int num, abi_long ret)
{
    trace_guest_user_syscall_ret(cpu, num, ret);
    qemu_plugin_vcpu_syscall_ret(cpu, num, ret);
/* #ifdef CONFIG_LMJ */
/*     syscall_ret_instrument(cpu, num, ret); */
/* #endif */
}


#endif /* _SYSCALL_TRACE_H_ */
