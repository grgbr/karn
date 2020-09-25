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

BUILDDIR := $(CURDIR)/build
DATADIR  := $(BUILDDIR)/data

pythonenv    := env PYTHONPATH="$(CURDIR)$(if $(PYTHONPATH),:$(PYTHONPATH))" \
                    PYTHONDONTWRITEBYTECODE="true"

intdat_files := $(call mk_intdat_files,fullrev,1,20) \
                $(call mk_intdat_files,rarerev,1,20) \
                $(call mk_intdat_files,even,1,20) \
                $(call mk_intdat_files,rarein,1,20) \
                $(call mk_intdat_files,fullin,1,20) \
                $(call mk_intdat_files,random,1,20)

.PHONY: build
build: $(addprefix $(DATADIR)/,$(addsuffix .dat,$(intdat_files)))

$(DATADIR)/type-int_%.dat: $(CURDIR)/mkintdat.py | $(DATADIR)
	$(pythonenv) $< $(dir $@) $(call dat_keynr,$@) $(call dat_presort,$@)

$(DATADIR):
	@mkdir -p $@
