/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch ipi interrupt header files
 *
 * Copyright (C) 2021 Loongson Technology Corporation Limited
 */

#ifndef HW_LOONGARCH_IPI_H
#define HW_LOONGARCH_IPI_H

#include "hw/sysbus.h"

/* Mainy used by iocsr read and write */
#define SMP_IPI_MAILBOX      0x1000ULL
#define CORE_STATUS_OFF       0x0
#define CORE_EN_OFF           0x4
#define CORE_SET_OFF          0x8
#define CORE_CLEAR_OFF        0xc
#define CORE_BUF_20           0x20
#define CORE_BUF_28           0x28
#define CORE_BUF_30           0x30
#define CORE_BUF_38           0x38
#define IOCSR_IPI_SEND        0x40

#define MAX_IPI_CORE_NUM      16
#define MAX_IPI_MBX_NUM       4

#define TYPE_LOONGARCH_IPI "loongarch_ipi"
OBJECT_DECLARE_SIMPLE_TYPE(LoongArchIPI, LOONGARCH_IPI)

typedef struct IPICore {
    uint32_t status;
    uint32_t en;
    uint32_t set;
    uint32_t clear;
    /* 64bit buf divide into 2 32bit buf */
    uint32_t buf[MAX_IPI_MBX_NUM * 2];
    qemu_irq irq;
} IPICore;

typedef struct LoongArchIPI {
    SysBusDevice parent_obj;
    IPICore core[MAX_IPI_CORE_NUM];
    MemoryRegion ipi_mmio[MAX_IPI_CORE_NUM];
} LoongArchIPI;

#endif
