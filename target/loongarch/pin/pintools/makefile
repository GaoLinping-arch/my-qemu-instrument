# -*- Mode: makefile -*-
#
# This Makefile example is fairly independent from the main makefile
# so users can take and adapt it for their build. We only really
# include config-host.mak so we don't have to repeat probing for
# cflags that the main configure has already done for us.
#

BUILD_DIR := $(CURDIR)/../../../..

include $(BUILD_DIR)/config-host.mak

VPATH += $(SRC_PATH)/target/loongarch/pin/pintools
NAMES :=
NAMES += template
NAMES += ins_count
NAMES += bbl_count
NAMES += hotblock
NAMES += inline_add
NAMES += strace
NAMES += routine
NAMES += bcc
NAMES += reg_frequency
NAMES += time_in_code_cache
NAMES += branch_count
NAMES += isampling
NAMES += branch_offset
NAMES += mem_trace
NAMES += champsim
NAMES += champsim_full
NAMES += champsim_slice
NAMES += champsim_multi
NAMES += mybranch_count
NAMES += ins_ismov
NAMES += ins_opndismem
NAMES += ins_eawidth
NAMES += ins_memopidxtoidx
NAMES += ins_opndisimplicit
NAMES += ifcall
NAMES += predicated
NAMES += if_thenpre
NAMES += ifpre_then
NAMES += ifpre_thenpre
#NAMES += original
NAMES += procedurecall
NAMES += reg_is_implicit
NAMES += change_reg
NAMES += proccount
NAMES += img_rtn
NAMES += rtn_insertcall
NAMES += bbl_trace
NAMES += trace_rtn
NAMES += trace_bbl_count
NAMES += trace_ifcall
NAMES += trace_bbl_cx
NAMES += inlinetest
NAMES += bbl_ctrlFlow
NAMES += bbl_info
NAMES += bbl_time_record
NAMES += call_trace
NAMES += data_cache_sim
NAMES += dynamic_ins_mix
NAMES += edge_cnt
NAMES += emuload
NAMES += jump_mix
NAMES += ldstmix
NAMES += malloc_trace
NAMES += pinatrace
NAMES += regmix
NAMES += regval
NAMES += top_routine
NAMES += topopcode
NAMES += trace_info

SONAMES := $(addsuffix .so,$(addprefix lib,$(NAMES)))

# The main QEMU uses Glib extensively so it's perfectly fine to use it
# in plugins (which many example do).
CFLAGS = $(GLIB_CFLAGS)
CFLAGS += -fPIC -Wall $(filter -W%, $(QEMU_CFLAGS))
CFLAGS += $(if $(findstring no-psabi,$(QEMU_CFLAGS)),-Wpsabi)
CFLAGS += -I$(SRC_PATH)/include
CFLAGS += -O2

# 照着CFLAGS抄的
CXXFLAGS = $(GLIB_CFLAGS)
CXXFLAGS += -fPIC -Wall $(filter -W%, $(QEMU_CXXFLAGS))
CXXFLAGS += $(if $(findstring no-psabi,$(QEMU_CXXFLAGS)),-Wpsabi)
CXXFLAGS += -I$(SRC_PATH)/include
CXXFLAGS += -std=gnu++11
CXXFLAGS += -O2

all: $(SONAMES)

%.o: %cpp
	c++ $(CXXFLAGS) -c -o $@ $<

%.o: %.c
	c++ $(CFLAGS) -c -o $@ $<

lib%.so: %.o
	c++ -shared -Wl,-soname,$@ -o $@ $^ $(LDLIBS)

clean:
	rm -f *.o *.so *.d
	rm -Rf .libs

.PHONY: all clean

