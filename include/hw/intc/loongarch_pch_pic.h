/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * LoongArch 7A1000 I/O interrupt controller definitions
 *
 * Copyright (c) 2021 Loongson Technology Corporation Limited
 */

#define TYPE_LOONGARCH_PCH_PIC "loongarch_pch_pic"
#define PCH_PIC_NAME(name) TYPE_LOONGARCH_PCH_PIC#name
OBJECT_DECLARE_SIMPLE_TYPE(LoongArchPCHPIC, LOONGARCH_PCH_PIC)

#define PCH_PIC_IRQ_START               0
#define PCH_PIC_IRQ_END                 63
#define PCH_PIC_IRQ_NUM                 64
#define PCH_PIC_INT_ID_VAL              0x7000000UL
#define PCH_PIC_INT_ID_NUM              0x3f0001UL

#define PCH_PIC_INT_ID_LO               0x00
#define PCH_PIC_INT_ID_HI               0x04
#define PCH_PIC_INT_MASK_LO             0x20
#define PCH_PIC_INT_MASK_HI             0x24
#define PCH_PIC_HTMSI_EN_LO             0x40
#define PCH_PIC_HTMSI_EN_HI             0x44
#define PCH_PIC_INT_EDGE_LO             0x60
#define PCH_PIC_INT_EDGE_HI             0x64
#define PCH_PIC_INT_CLEAR_LO            0x80
#define PCH_PIC_INT_CLEAR_HI            0x84
#define PCH_PIC_AUTO_CTRL0_LO           0xc0
#define PCH_PIC_AUTO_CTRL0_HI           0xc4
#define PCH_PIC_AUTO_CTRL1_LO           0xe0
#define PCH_PIC_AUTO_CTRL1_HI           0xe4
#define PCH_PIC_ROUTE_ENTRY_OFFSET      0x100
#define PCH_PIC_ROUTE_ENTRY_END         0x13f
#define PCH_PIC_HTMSI_VEC_OFFSET        0x200
#define PCH_PIC_HTMSI_VEC_END           0x23f
#define PCH_PIC_INT_STATUS_LO           0x3a0
#define PCH_PIC_INT_STATUS_HI           0x3a4
#define PCH_PIC_INT_POL_LO              0x3e0
#define PCH_PIC_INT_POL_HI              0x3e4

#define STATUS_LO_START                 0
#define STATUS_HI_START                 0x4
#define POL_LO_START                    0x40
#define POL_HI_START                    0x44

typedef struct LoongArchPCHPIC {
    SysBusDevice parent_obj;
    qemu_irq parent_irq[64];
    uint32_t int_mask_lo; /*0x020 interrupt mask register*/
    uint32_t int_mask_hi;
    uint32_t htmsi_en_lo; /*0x040 1=msi*/
    uint32_t htmsi_en_hi;
    uint32_t intedge_lo; /*0x060 edge=1 level  =0*/
    uint32_t intedge_hi; /*0x060 edge=1 level  =0*/
    uint32_t intclr_lo; /*0x080 for clean edge int,set 1 clean,set 0 is noused*/
    uint32_t intclr_hi; /*0x080 for clean edge int,set 1 clean,set 0 is noused*/
    uint32_t auto_crtl0_lo; /*0x0c0*/
    uint32_t auto_crtl0_hi; /*0x0c0*/
    uint32_t auto_crtl1_lo; /*0x0e0*/
    uint32_t auto_crtl1_hi; /*0x0e0*/
    uint32_t last_intirr_lo;    /* edge detection */
    uint32_t last_intirr_hi;    /* edge detection */
    uint32_t intirr_lo; /* 0x380 interrupt request register */
    uint32_t intirr_hi; /* 0x380 interrupt request register */
    uint32_t intisr_lo; /* 0x3a0 interrupt service register */
    uint32_t intisr_hi; /* 0x3a0 interrupt service register */
    /*
     * 0x3e0 interrupt level polarity selection
     * register 0 for high level trigger
     */
    uint32_t int_polarity_lo;
    uint32_t int_polarity_hi;

    uint8_t route_entry[64]; /*0x100 - 0x138*/
    uint8_t htmsi_vector[64]; /*0x200 - 0x238*/

    MemoryRegion iomem32_low;
    MemoryRegion iomem32_high;
    MemoryRegion iomem8;
} LoongArchPCHPIC;
