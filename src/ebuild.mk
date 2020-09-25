solibs             := libkarn.so

libkarn.so-objs    := farr.o
libkarn.so-objs    += $(call kconf_enabled,KARN_SLIST,slist.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_DLIST,dlist.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_FBNR_HEAP,fbnr_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_LCRS,lcrs.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_SBNM_HEAP,sbnm_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_DBNM_HEAP,dbnm_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_SPAIR_HEAP,spair_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_FBMP,fbmp.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_FWK_HEAP,fwk_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_PBNM_HEAP,pbnm_heap.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_FALLOC,falloc.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_AVL,avl.o)
libkarn.so-objs    += $(call kconf_enabled,KARN_PAVL,pavl.o)

libkarn.so-cflags  := -I$(TOPDIR)/include \
                      $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE -DPIC -fpic

libkarn.so-ldflags := $(EXTRA_LDFLAGS) -shared -fpic -Wl,-soname,libkarn.so
libkarn.so-ldflags += $(call kconf_enabled,KARN_BTRACE,-rdynamic)
