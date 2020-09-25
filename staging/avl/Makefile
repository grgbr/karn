include ../utils.mk

EXTRA_CPPFLAGS ?= -DNDEBUG
EXTRA_CFLAGS   ?= -O2

CPPFLAGS     := -D_GNU_SOURCE -I$(BUILDDIR) -I$(SRCDIR)/include $(EXTRA_CPPFLAGS)
bin-cppflags  = $(CPPFLAGS)
lib-cppflags  = -DPIC $(CPPFLAGS)
CFLAGS       := -Wall -Wextra -MD -g $(EXTRA_CFLAGS)
bin-cflags    = $(CFLAGS)
lib-cflags    = -fpic $(CFLAGS)
LDFLAGS      := $(EXTRA_LDFLAGS)
bin-ldflags   = $(LDFLAGS)
lib-ldflags   = -shared -fpic -Wl,-soname,libkarn.so $(LDFLAGS)
LIBS         ?=

utest-objs  :=
lib-objs    :=
lib-headers :=

ifeq ($(CONFIG_KARN_ASSERT),y)
CFLAGS += $(shell $(PKG_CONFIG) --cflags lutils)
LIBS   += $(shell $(PKG_CONFIG) --libs lutils)
endif

ifeq ($(CONFIG_KARN_BTRACE),y)
CFLAGS += $(shell $(PKG_CONFIG) --cflags lbtrace)
endif

ifeq ($(CONFIG_KARN_AVL),y)
ifeq ($(CONFIG_KARN_AVL_TEST),y)
utest-objs  += avl_ut.o
endif
lib-objs    += avl.o
lib-headers += avl.h
endif

ifeq ($(CONFIG_KARN_PAVL),y)
ifeq ($(CONFIG_KARN_PAVL_TEST),y)
utest-objs  += pavl_ut.o
endif
lib-objs    += pavl.o
lib-headers += pavl.h
endif

ptest-objs  := $(if $(utest-objs),perf.o bst_pt.o)

utest-bins  := $(patsubst %.o,karn_%,$(utest-objs))
ptest-bins      := $(patsubst %.o,karn_%,$(filter-out perf.o,$(ptest-objs)))

define config_header_tmpl
#ifndef _KARN_CONFIG_H
#define _KARN_CONFIG_H

$(call config_output_bool,CONFIG_KARN_ASSERT)
$(call config_output_bool,CONFIG_KARN_BTRACE)

$(call config_output_bool,CONFIG_KARN_AVL)
$(call config_output_bool,CONFIG_KARN_AVL_TEST)

$(call config_output_bool,CONFIG_KARN_PAVL)
$(call config_output_bool,CONFIG_KARN_PAVL_TEST)

#endif /* _KARN_CONFIG_H */
endef

.PHONY: config
config: $(BUILDDIR)/config.h

$(BUILDDIR)/config.h: Makefile | $(BUILDDIR)
	$(Q)$(call config_cmd,$(@),$(config_header_tmpl))
	@echo 'Build setup:'
	@echo '    assert              $(call config_show_bool,CONFIG_KARN_ASSERT)'
	@echo '    backtrace           $(call config_show_bool,CONFIG_KARN_BTRACE)'
	@echo '    AVL tree            $(call config_show_bool,CONFIG_KARN_AVL)'
	@echo '    AVL test            $(call config_show_bool,CONFIG_KARN_AVL_TEST)'
	@echo '    "parented" AVL tree $(call config_show_bool,CONFIG_KARN_PAVL)'
	@echo '    "parented" AVL test $(call config_show_bool,CONFIG_KARN_PAVL_TEST)'

.PHONY: all
all: $(BUILDDIR)/libkarn.so \
     $(addprefix $(BUILDDIR)/,$(utest-bins)) \
     $(addprefix $(BUILDDIR)/,$(ptest-bins))

$(BUILDDIR)/libkarn.so: $(addprefix $(BUILDDIR)/,$(lib-objs))
	$(Q)$(call ld_cmd,$(@)) $(lib-ldflags) -o $(@) $(^) $(LIBS)

$(addprefix $(BUILDDIR)/,$(lib-objs)): $(BUILDDIR)/%.o: $(SRCDIR)/%.c \
                                                        $(BUILDDIR)/config.h \
                                                        | $(BUILDDIR)
	$(Q)$(call cc_cmd,$(@)) $(lib-cppflags) $(lib-cflags) -o $(@) -c $(<)

$(addprefix $(BUILDDIR)/,$(utest-bins)): \
	$(BUILDDIR)/karn_%: $(BUILDDIR)/%.o \
	                    $(BUILDDIR)/libkarn.so
	$(Q)$(call ld_cmd,$(@)) $(bin-ldflags) -o $(@) $(<) \
		-lkarn -lutils -lcriterion \
		$(shell $(PKG_CONFIG) --cflags --libs lbtrace)

$(addprefix $(BUILDDIR)/,$(utest-objs)): \
	$(BUILDDIR)/%.o: $(SRCDIR)/test/%.c \
	                 $(BUILDDIR)/config.h \
	                 | $(BUILDDIR)
	$(Q)$(call cc_cmd,$(@)) $(bin-cppflags) $(bin-cflags) -o $(@) -c $(<)


$(addprefix $(BUILDDIR)/,$(ptest-bins)): \
	$(BUILDDIR)/karn_%: $(BUILDDIR)/%.o \
	                    $(BUILDDIR)/perf.o \
	                    $(BUILDDIR)/libkarn.so
	$(Q)$(call ld_cmd,$(@)) $(bin-ldflags) -o $(@) \
		$(filter-out libkarn.so,$(^)) -lkarn -lutils

$(addprefix $(BUILDDIR)/,$(ptest-objs)): $(BUILDDIR)/%.o: $(SRCDIR)/test/%.c \
                                                          $(BUILDDIR)/config.h \
                                                          | $(BUILDDIR)
	$(Q)$(call cc_cmd,$(@)) $(bin-cppflags) $(bin-cflags) -o $(@) -c $(<)

$(BUILDDIR):
	@mkdir -p $@

.PHONY: install
install: $(addprefix $(INCLUDEDIR)/karn/,$(lib-headers)) \
         $(INCLUDEDIR)/karn/config.h \
         $(LIBDIR)/libkarn.so \
         $(PKGCONFIGDIR)/libkarn.pc \
         $(addprefix $(BINDIR)/,$(utest-bins)) \
         $(addprefix $(BINDIR)/,$(ptest-bins))

$(addprefix $(INCLUDEDIR)/karn/,$(lib-headers)): \
	$(INCLUDEDIR)/karn/%.h: $(SRCDIR)/include/karn/%.h
	$(Q)$(call install_cmd,$(@)) --mode 644 -D $(<) $(@)

$(INCLUDEDIR)/karn/config.h: $(BUILDDIR)/config.h
	$(Q)$(call install_cmd,$(@)) --mode 644 -D $(<) $(@)

$(LIBDIR)/libkarn.so: $(BUILDDIR)/libkarn.so
	$(Q)$(call install_cmd,$(@)) --mode 755 -D $(<) $(@)

$(BINDIR)/%: $(BUILDDIR)/%
	$(Q)$(call install_cmd,$(@)) --mode 755 -D $(<) $(@)

define pkgconfig_meta_tmpl
prefix=$(PREFIX)
exec_prefix=$${prefix}
libdir=$${exec_prefix}/lib

Name: libkarn
Description: Karn library
Version: %%PKG_VERSION%%
Requires:
Libs: -L$${libdir} -lkarn
endef

$(PKGCONFIGDIR)/libkarn.pc: Makefile | $(PKGCONFIGDIR)
	$(Q)$(call pkgconfig_cmd,$(@),$(pkgconfig_meta_tmpl))

$(PKGCONFIGDIR):
	$(Q)$(call install_cmd,$(@)) --mode 755 -d $(@)

.PHONY: clean
clean:
	$(Q)$(call rmrf_cmd,$(BUILDDIR))

.PHONY: uninstall
uninstall:
	$(Q)$(call rmrf_cmd,$(INCLUDEDIR)/karn)
	$(Q)$(call rmf_cmd,$(LIBDIR)/libkarn.so)
	$(Q)$(call rmf_cmd,$(PKGCONFIGDIR)/libkarn.pc)
	$(Q)$(call rmf_cmd,$(addprefix $(BINDIR)/,$(utest-bins) $(ptest-bins)))

-include $(wildcard $(BUILDDIR)/*.d)

.SUFFIXES:
