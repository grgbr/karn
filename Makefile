################################################################################
# Helper macros
################################################################################

define pip_install
	if pip list 2>&1 | grep -q $(1); then \
		pip install --upgrade --user $(2); \
	else \
		pip install --user $(2); \
	fi
endef

define dat_keynr
	$(subst keynr-,,$(word 2,$(subst _, ,$(notdir $(basename $(1))))))
endef

define dat_presort
	$(subst presort-,,$(word 3,$(subst _, ,$(notdir $(basename $(1))))))
endef

define dat_algo
	$(subst algo-,,$(word 4,$(subst _, ,$(notdir $(basename $(1))))))
endef

define pow2_range
	$(shell \
		i=$(1); \
		while [ $$i -le $(2) ]; \
			do echo 2 ^ $$i | bc; \
			i=$$(expr $$i + 1); \
		done)
endef

define mk_intdat_files
	$(foreach n,$(call pow2_range,$(2),$(3)),\
	  type-int_keynr-$(n)_presort-$(1))
endef

# Build a performance file basename from arguments.
# $(1): algorithm name
# $(2): presorted scheme name
# $(3): log base 2 of number of keys to be sorted
#
# Note: $(1) must be declared into $(intpt_algo) variable
#       $(2) must be declared into $(intpt_presort) variable
# This allows to run a make for a particular subset of algorithm(s) and
# presorted scheme(s) passed onto command line:
#     $ make perf intpt_algo="selection merge" intpt_presort="fullin"
define mk_intpt_files
	$(if $(findstring $(2),$(intpt_presort)),\
	  $(if $(findstring $(1),$(intpt_algo)),\
	    $(foreach n,$(call pow2_range,$(3),$(4)),\
	      type-int_keynr-$(n)_presort-$(2)_algo-$(1))))
endef

BUILD    := $(CURDIR)/build
DATA     := $(BUILD)/data
DOC      := $(CURDIR)/doc
SRC      := $(CURDIR)/src
TEST     := $(CURDIR)/test
PERF     := $(BUILD)/perf
SCRIPT   := $(CURDIR)/script

confdir  := $(CURDIR)/config

docbuilddir    := $(BUILD)/doc
sphinxbuilddir := $(docbuilddir)/build
sphinxsrcdir   := $(docbuilddir)/sphinx
doxybuilddir   := $(docbuilddir)/doxygen

AR := gcc-ar
CC := gcc

pythonenv := env PYTHONPATH="$(CURDIR)/script:$(PYTHONPATH)" \
                 PYTHONDONTWRITEBYTECODE="true"

-include $(BUILD)/include/config/auto.conf

common-cflags := -std=gnu99 -Wall -Wextra -MD -D_GNU_SOURCE \
                 -fstrict-aliasing \
                 -fstack-protector-all -fstack-protector-strong \
                 -D_FORTIFY_SOURCE=2 -march=native \
                 -include $(BUILD)/include/generated/autoconf.h \
                 -I$(HOME)/local/include -L$(HOME)/local/lib
BUILD_CFLAGS := $(common-cflags) -flto -fpic -DNDEBUG -O2
DBG_CFLAGS   := $(common-cflags) -fpic -ggdb3
UT_CFLAGS    := $(common-cflags) -flto -O2 -ggdb3 -ftest-coverage -fprofile-arcs
PT_CFLAGS    := $(BUILD_CFLAGS) -flto -pie \
                -Wl,--relax -Wl,--sort-common -Wl,--strip-all \
                -Wl,-z,combreloc -Wl,-z,noexecstack -Wl,-z,now \
                -Wl,-z,loadfltr -Wl,-z,relro

# Testing data sets
intdat_files  = $(call mk_intdat_files,fullrev,1,20) \
                $(call mk_intdat_files,rarerev,1,20) \
                $(call mk_intdat_files,even,1,20) \
                $(call mk_intdat_files,rarein,1,20) \
                $(call mk_intdat_files,fullin,1,20) \
                $(call mk_intdat_files,worstins,1,18) \
                $(call mk_intdat_files,random,1,20)
intpt_loopnr  = 100
intpt_presort = fullrev rarerev even rarein fullin worstins random
intpt_algo    =
intpt_files   =

# Library source files located into $(SRC)
lib_src =
# Unit test source files located into $(TEST)
ut_src  = karn_ut.c utils_ut.c
# Performance test binaries
pt_bin  = array_fixed_pt

ifeq ($(CONFIG_SLIST),y)
lib_src   += slist.c
ut_src    += slist_ut.c
pt_bin    += slist_pt
PT_CFLAGS += -DCONFIG_SLIST_PERF_EVENTS
endif

ifeq ($(CONFIG_SLIST_INSERTION_SORT),y)
intpt_algo  += insertion
intpt_files += $(call mk_intpt_files,insertion,fullrev,1,20) \
               $(call mk_intpt_files,insertion,rarerev,1,20) \
               $(call mk_intpt_files,insertion,even,1,20) \
               $(call mk_intpt_files,insertion,rarein,1,20) \
               $(call mk_intpt_files,insertion,fullin,1,20) \
               $(call mk_intpt_files,insertion,worstins,1,15) \
               $(call mk_intpt_files,insertion,random,12,20)
endif

ifeq ($(CONFIG_SLIST_SELECTION_SORT),y)
intpt_algo  += selection
intpt_files += $(call mk_intpt_files,selection,fullrev,1,12) \
               $(call mk_intpt_files,selection,rarerev,1,12) \
               $(call mk_intpt_files,selection,even,1,12) \
               $(call mk_intpt_files,selection,rarein,1,12) \
               $(call mk_intpt_files,selection,fullin,1,12) \
               $(call mk_intpt_files,selection,worstins,1,12) \
               $(call mk_intpt_files,selection,random,12,12)
endif

ifeq ($(CONFIG_SLIST_BUBBLE_SORT),y)
intpt_algo  += bubble
intpt_files += $(call mk_intpt_files,bubble,fullrev,1,12) \
               $(call mk_intpt_files,bubble,rarerev,1,12) \
               $(call mk_intpt_files,bubble,even,1,12) \
               $(call mk_intpt_files,bubble,rarein,1,12) \
               $(call mk_intpt_files,bubble,fullin,1,12) \
               $(call mk_intpt_files,bubble,worstins,1,12) \
               $(call mk_intpt_files,bubble,random,12,12)
endif

ifeq ($(CONFIG_SLIST_MERGE_SORT),y)
intpt_algo  += merge
intpt_files += $(call mk_intpt_files,merge,fullrev,1,20) \
               $(call mk_intpt_files,merge,rarerev,1,20) \
               $(call mk_intpt_files,merge,even,1,20) \
               $(call mk_intpt_files,merge,rarein,1,20) \
               $(call mk_intpt_files,merge,fullin,1,20) \
               $(call mk_intpt_files,merge,worstins,1,15) \
               $(call mk_intpt_files,merge,random,12,20)
endif

ifeq ($(CONFIG_BHEAP_FIXED),y)
lib_src += bheap.c
ut_src  += bheap_ut.c
endif

# Needed for performance results generation rules (see below)
.SECONDEXPANSION:

.PHONY: help
help:
	@echo "Please, have a look at the README file stored in this directory"
	@echo ""
	@echo "Available targets:"
	@echo "    config     - library build interactive configuration"
	@echo "    defconfig  - select library build configuration scheme from" \
	"$(confdir) directory"
	@echo "    saveconfig - save library build configuration scheme into" \
	"$(confdir) directory"
	@echo "    build      - build library into $(BUILD)"
	@echo "    test       - build unit test binaries into $(BUILD)"
	@echo "    check      - run unit test and output results into" \
	"$(BUILD)/karn_ut.xml"
	@echo "    cov        - display and build coverage statistics into" \
	"$(BUILD)/karn_cov.xml"
	@echo "    data       - build testing data sets into $(DATA)"
	@echo "    perf       - run performance tests and output results into" \
	"$(PERF)"
	@echo "    plot       - plot performance test results into $(PERF)"
	@echo "    doc        - build documentation into $(sphinxbuilddir)"
	@echo "    clean      - cleanup build objects"
	@echo "    mrproper   - cleanup everything"
	@echo "    help       - this help message"

.PHONY: config
config: | $(BUILD)
	cd $(BUILD) && kconfig-mconf $(CURDIR)/Config.in

.PHONY: defconfig-%
defconfig-%: $(confdir)/%.config | $(BUILD)
	cp $< $(BUILD)/.config

.PHONY: saveconfig-%
saveconfig-%: $(BUILD)/.config
	cp $< $(confdir)/$(subst saveconfig-,,$@).config

.PHONY: build
build: $(BUILD)/libkarn.a $(BUILD)/libkarn_dbg.a $(BUILD)/libkarn_ut.a \
       $(BUILD)/libkarn_pt.a

.PHONY: test
test: $(BUILD)/karn_ut $(BUILD)/karn_utdbg \
      $(addprefix $(BUILD)/,$(pt_bin))

.PHONY: check
check: $(BUILD)/karn_ut.xml
	$(BUILD)/karn_ut

.PHONY: cov
cov: $(BUILD)/karn_cov.xml
	$(SCRIPT)/gcov.sh $(BUILD)/ut

.PHONY: data
data: $(addsuffix .dat,$(addprefix $(DATA)/,$(intdat_files)))

.PHONY: perf
perf: $(addsuffix .txt,$(addprefix $(PERF)/,$(intpt_files)))

.PHONY: plot
plot: $(foreach a,$(intpt_algo),$(PERF)/type-int_algo-$(a).png) \
      $(foreach p,$(intpt_presort),$(PERF)/type-int_presort-$(p).png)

.PHONY: doc
doc: plot | $(doxybuilddir) $(sphinxbuilddir) $(sphinxsrcdir)
	env SRC="$(SRC) $(TEST)" DOCBUILD=$(doxybuilddir) doxygen \
		$(DOC)/Doxyfile
	rsync -rlpt --delete $(DOC)/* $(sphinxsrcdir)
	rsync -lpt $(PERF)/*.png $(DATA)/*.png $(sphinxsrcdir)/images
	env DOXYBUILD=$(doxybuilddir)/xml sphinx-build \
		$(sphinxsrcdir) $(sphinxbuilddir)

.PHONY: dev
dev: | $(BUILD)
	ctags -f $(BUILD)/tags -R
	cd $(BUILD) && cscope -bq $(shell find $(SRC) $(TEST) -type f)

.PHONY: clean
clean:
	$(RM) -r $(BUILD)/*.[od] $(BUILD)/dbg $(BUILD)/utdbg $(BUILD)/ut \
		$(BUILD)/pt

.PHONY: mrproper
mrproper:
	$(RM) -r $(BUILD) $(DATA) $(DOCBUILD) $(PERF)

$(BUILD)/karn_ut.xml: $(BUILD)/karn_ut
	$(BUILD)/karn_ut -x > $@
	xmllint --noout -schema $(SCRIPT)/junit-jenkins.xsd $@

$(BUILD)/karn_cov.xml: $(BUILD)/karn_ut.xml $(SCRIPT)/gcov.sh
	$(SCRIPT)/gcov.sh --xml $(BUILD)/ut > $@

################################################################################
# Build configuration header
################################################################################
Makefile: $(BUILD)/include/config/auto.conf

$(BUILD)/.config: $(CURDIR)/Config.in | $(BUILD)
	cd $(BUILD) && kconfig-conf --alldefconfig $< >/dev/null

$(BUILD)/include/generated/autoconf.h \
$(BUILD)/include/config/auto.conf: $(BUILD)/.config | \
                                   $(BUILD)/include/config \
                                   $(BUILD)/include/generated
	cd $(BUILD) && \
		kconfig-conf --silentoldconfig $(CURDIR)/Config.in >/dev/null

################################################################################
# Build unit testing objects with optimization enabled.
################################################################################
cute_cflags := $(shell pkg-config --cflags libcute)
cute_libs   := $(shell pkg-config --libs libcute)

# cd to build directory first to ensure .gcno coverage data files are generated
# into it.
$(BUILD)/karn_ut: $(addprefix $(BUILD)/ut/,$(patsubst %.c,%.o,$(ut_src))) \
                  $(BUILD)/libkarn_ut.a
	cd $(BUILD) && $(CC) $(UT_CFLAGS) $(cute_cflags) -L$(BUILD) \
		-o $@ $(filter %.o,$^) -lkarn_ut $(cute_libs)

$(BUILD)/ut/%_ut.o: $(TEST)/%_ut.c $(BUILD)/include/generated/autoconf.h | \
                    $(BUILD)/ut
	$(CC) -I$(TEST) -I$(SRC) $(UT_CFLAGS) $(cute_cflags) -o $@ -c $<

################################################################################
# Build unit testing objects for debugging purpose.
################################################################################

$(BUILD)/karn_utdbg: $(addprefix $(BUILD)/utdbg/,$(patsubst %.c,%.o,$(ut_src))) \
                     $(BUILD)/libkarn_dbg.a
	$(CC) $(DBG_CFLAGS) $(cute_cflags) -L$(BUILD) \
		-o $@ $(filter %.o,$^) -lkarn_dbg $(cute_libs)

$(BUILD)/utdbg/%_ut.o: $(TEST)/%_ut.c $(BUILD)/include/generated/autoconf.h | \
                       $(BUILD)/utdbg
	$(CC) -I$(TEST) -I$(SRC) $(DBG_CFLAGS) $(cute_cflags) -o $@ -c $<

################################################################################
# Build performance testing objects.
################################################################################
$(BUILD)/slist_pt: $(TEST)/slist_pt.c $(BUILD)/libkarn_pt.a
	$(CC) -I$(SRC) $(PT_CFLAGS) -L$(BUILD) -o $@ $< -lkarn_pt

$(BUILD)/array_fixed_pt: $(TEST)/array_fixed_pt.c
	$(CC) -I$(SRC) $(PT_CFLAGS) -L$(BUILD) -o $@ $<

$(BUILD)/pt/%_pt.o: $(TEST)/%_pt.c $(BUILD)/include/generated/autoconf.h | \
                    $(BUILD)/pt
	$(CC) -I$(SRC) $(PT_CFLAGS) -o $@ -c $<

################################################################################
# Build library objects.
################################################################################
$(BUILD)/libkarn.a: $(addprefix $(BUILD)/,$(lib_src:.c=.o))
$(BUILD)/libkarn_dbg.a: $(addprefix $(BUILD)/dbg/,$(lib_src:.c=.o))
$(BUILD)/libkarn_ut.a: $(addprefix $(BUILD)/ut/,$(lib_src:.c=.o))
$(BUILD)/libkarn_pt.a: $(addprefix $(BUILD)/pt/,$(lib_src:.c=.o))

$(BUILD)/%.a:
	$(AR) crs $@ $(filter-out %.h,$^)

# Build library objects for debug purpose.
$(BUILD)/dbg/%.o: $(SRC)/%.c $(BUILD)/include/generated/autoconf.h | \
                  $(BUILD)/dbg
	$(CC) -I$(SRC) $(DBG_CFLAGS) -o $@ -c $<

# Build library objects for unit testing purpose.
$(BUILD)/ut/%.o: $(SRC)/%.c $(BUILD)/include/generated/autoconf.h | $(BUILD)/ut
	$(CC) -I$(SRC) $(UT_CFLAGS) -o $@ -c $<

# Build library objects for performance testing purpose.
$(BUILD)/pt/%.o: $(SRC)/%.c $(BUILD)/include/generated/autoconf.h | $(BUILD)/pt
	$(CC) -I$(SRC) $(PT_CFLAGS) -o $@ -c $<

# Build library objects.
$(BUILD)/%.o: $(SRC)/%.c $(BUILD)/include/generated/autoconf.h
	$(CC) -I$(SRC) $(BUILD_CFLAGS) -o $@ -c $<

################################################################################
# Build coverage statistics out of unit tests run.
################################################################################
$(BUILD)/karn_ut-cov.rst: $(BUILD)/karn_ut
	$<
	gcovr --object-directory=$(BUILD)/ut --root=$(CURDIR) \
		--gcov-exclude='.*_ut.*' -p --xml -o $@

################################################################################
# Performance testing data sets and ploting.
################################################################################
$(DATA)/type-int_%.dat: $(SCRIPT)/mkintdat.py | $(DATA)
	$(pythonenv) $< $(dir $@) $(call dat_keynr,$@) $(call dat_presort,$@)

# This rule is a bit special: carefully look at the last prerequisite...
# This rule relies upon prerequisite secondary expansion to derive performance
# data file name from target (by removing algorithm name from it).
$(PERF)/type-int_%.txt: $(SCRIPT)/slist_run_pt.py \
                        $(BUILD)/slist_pt \
                        $(DATA)/type-int_$$(firstword $$(subst _algo-, ,%)).dat \
                        | $(PERF)
	$(pythonenv) $< $(word 2,$^) $(DATA) $(call dat_keynr,$@) \
		$(call dat_presort,$@) $(call dat_algo,$@) $(intpt_loopnr) > $@

$(PERF)/type-int_algo-%.png: $(SCRIPT)/slist_plot_algo_pt.py \
                             $(addsuffix .txt,\
                               $(addprefix $(PERF)/,$(intpt_files)))
	$(pythonenv) $< $(PERF) $(subst algo-,,\
	             $(word 2,$(subst _, ,$(notdir $(basename $@)))))

$(PERF)/type-int_presort-%.png: $(SCRIPT)/slist_plot_presort_pt.py \
                                $(addsuffix .txt,\
                                  $(addprefix $(PERF)/,$(intpt_files)))
	$(pythonenv) $< $(PERF) $(subst presort-,,\
	             $(word 2,$(subst _, ,$(notdir $(basename $@)))))

$(BUILD) $(BUILD)/ut $(BUILD)/dbg $(BUILD)/utdbg $(BUILD)/pt \
$(DOCBUILD) $(DATA) $(PERF) $(BUILD)/include/config $(BUILD)/include/generated \
$(doxybuilddir) $(sphinxbuilddir) $(sphinxsrcdir):
	mkdir -p $@

-include $(wildcard $(BUILD)/*.d)
