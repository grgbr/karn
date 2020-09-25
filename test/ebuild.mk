################################################################################
# Unit tests
################################################################################

bins               := karn_ut
karn_ut-cflags     := -I$(TOPDIR)/include \
                      $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE \
                      -ftest-coverage -fprofile-arcs
karn_ut-ldflags    := $(EXTRA_LDFLAGS) -lkarn -lgcov
karn_ut-pkgconf    := libcute libutils
karn_ut-objs        = test/karn_ut.o test/utils_ut.o
karn_ut-objs       += $(call kconf_enabled,KARN_SLIST,slist_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_DLIST,dlist_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_FBNR_HEAP,fbnr_heap_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_SBNM_HEAP,sbnm_heap_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_DBNM_HEAP,dbnm_heap_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_SPAIR_HEAP,spair_heap_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_FWK_HEAP,fwk_heap_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_LCRS,lcrs_ut.o)
karn_ut-objs       += $(call kconf_enabled,KARN_PBNM_HEAP,pbnm_heap_ut.o)
ifeq ($(strip $(or $(CONFIG_KARN_FARR_BUBBLE_SORT), \
                   $(CONFIG_KARN_FARR_SELECTION_SORT), \
                   $(CONFIG_KARN_FARR_INSERTION_SORT), \
                   $(CONFIG_KARN_FARR_QUICK_SORT))),y)
karn_ut-objs       += farr_ut.o
endif

################################################################################
# Performance test
################################################################################

ifeq ($(CONFIG_KARN_PERF),y)

KARN_PT_CFLAGS      := -I$(TOPDIR)/include \
                       $(EXTRA_CFLAGS) -Wall -Wextra -D_GNU_SOURCE
KARN_PT_LDFLAGS     := $(EXTRA_LDFLAGS) -lkarn
KARN_PT_PKGCONF     := libutils

arlibs              := libkarn_pt.a
libkarn_pt.a-objs   := karn_pt.o
libkarn_pt.a-cflags := $(KARN_PT_CFLAGS)

bins               += farr_pt
farr_pt-cflags     := $(KARN_PT_CFLAGS)
farr_pt-ldflags    := $(KARN_PT_LDFLAGS) -lkarn_pt
farr_pt-pkgconf    := $(KARN_PT_PKGCONF)
farr_pt-objs       := farr_pt.o

bins               += $(call kconf_enabled,KARN_SLIST_PERF_EVENTS,slist_pt)
slist_pt-cflags    := $(KARN_PT_CFLAGS)
slist_pt-ldflags   := $(KARN_PT_LDFLAGS) -lkarn_pt
slist_pt-pkgconf   := $(KARN_PT_PKGCONF)
slist_pt-objs      := slist_pt.o

ifeq ($(or $(CONFIG_KARN_FBNR_HEAP), \
           $(CONFIG_KARN_FWK_HEAP), \
           $(CONFIG_KARN_SBNM_HEAP), \
           $(CONFIG_KARN_DBNM_HEAP), \
           $(CONFIG_KARN_SPAIR_HEAP), \
           $(CONFIG_KARN_PBNM_HEAP))),y)

bins              += heap_pt
heap_pt-cflags    := $(KARN_PT_CFLAGS)
heap_pt-ldflags   := $(KARN_PT_LDFLAGS) -lkarn_pt
heap_pt-pkgconf   := $(KARN_PT_PKGCONF)
heap_pt-objs      := heap_pt.o

endif # ifeq ($(or $(CONFIG_KARN_FBNR_HEAP), \
      #            $(CONFIG_KARN_FWK_HEAP), \
      #            $(CONFIG_KARN_SBNM_HEAP), \
      #            $(CONFIG_KARN_DBNM_HEAP), \
      #            $(CONFIG_KARN_SPAIR_HEAP), \
      #            $(CONFIG_KARN_PBNM_HEAP))),y)

endif # ($(CONFIG_KARN_PERF),y)
