config-in := Config.in
config-h  := karn/config.h

subdirs    = src

subdirs   += $(call kconf_enabled,KARN_TEST,test)
test-deps := src

HEADERDIR := $(CURDIR)/include
headers    = karn/common.h karn/farr.h karn/fabs_tree.h
headers   += $(call kconf_enabled,KARN_SLIST,karn/slist.h)
headers   += $(call kconf_enabled,KARN_DLIST,karn/dlist.h)
headers   += $(call kconf_enabled,KARN_FBNR_HEAP,karn/fbnr_heap.h)
headers   += $(call kconf_enabled,KARN_LCRS,karn/lcrs.h)
headers   += $(call kconf_enabled,KARN_SBNM_HEAP,karn/sbnm_heap.h)
headers   += $(call kconf_enabled,KARN_DBNM_HEAP,karn/dbnm_heap.h)
headers   += $(call kconf_enabled,KARN_SPAIR_HEAP,karn/spair_heap.h)
headers   += $(call kconf_enabled,KARN_FBMP,karn/fbmp.h)
headers   += $(call kconf_enabled,KARN_FWK_HEAP,karn/fwk_heap.h)
headers   += $(call kconf_enabled,KARN_PBNM_HEAP,karn/pbnm_heap.h)
headers   += $(call kconf_enabled,KARN_FALLOC,karn/falloc.h)
headers   += $(call kconf_enabled,KARN_AVL,karn/avl.h)
headers   += $(call kconf_enabled,KARN_PAVL,karn/pavl.h)

define libkarn_pkgconf_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib
includedir=$${prefix}/include

Name: libkarn
Description: Karn data structure library
Version: %%PKG_VERSION%%
Requires: libutils
Cflags: -I$${includedir}
Libs: -L$${libdir} -lkarn
endef

pkgconfigs      := libkarn.pc
libkarn.pc-tmpl := libkarn_pkgconf_tmpl
