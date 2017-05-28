BUILD   := $(CURDIR)/build
DATA    := $(CURDIR)/data
DOC     := $(CURDIR)/doc
SRC     := $(CURDIR)/src
TEST    := $(CURDIR)/test

common-cflags := -std=gnu99 -Wall -Wextra -MD -D_GNU_SOURCE \
                 -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 \
                 -fstack-protector-all -fstack-protector-strong \
                 -D_FORTIFY_SOURCE=2
BUILD_CFLAGS  := $(common-cflags) -DNDEBUG -pthread -O2
DEBUG_CFLAGS  := $(common-cflags) -pthread -ggdb3
TEST_CFLAGS   := $(common-cflags) -pthread -O2 -ggdb3
CFLAGS        := $(BUILD_CFLAGS)
#CFLAGS        := $(DEBUG_CFLAGS)
PERF_CFLAGS   := $(common-cflags) -fpic -pie -DNDEBUG -O2 \
                 -Wl,-flto -Wl,--relax -Wl,--sort-common -Wl,--strip-all \
                 -Wl,-z,combreloc -Wl,-z,noexecstack -Wl,-z,now \
                 -Wl,-z,loadfltr -Wl,-z,relro

.PHONY: help
help:
	@echo "Please, have a look at the README file stored in this directory"
	@echo ""
	@echo "Available targets:"
	@echo "    build    - build mapred binary into $(BUILD)"
	@echo "    test     - build unit test binaries into $(BUILD)"
	@echo "    mrproper - cleanup everything"
	@echo "    pkg      - install Ubuntu/Debian tool packages"
	@echo "    clean    - cleanup build objects"
	@echo "    doc      - build source documentation into $(DOC)"
	@echo "    help     - this help message"

.PHONY: build
build: $(BUILD)/mapred_rbtree $(BUILD)/mapred_hattrie

.PHONY: test
test: $(BUILD)/ut_token_store_rbtree \
      $(BUILD)/ut_token_store_hattrie \
      $(BUILD)/ut_slist \
      $(BUILD)/slist_perf_run

.PHONY: doc
doc: | $(DOC)
	env SRC=$(SRC) DOC=$(DOC) doxygen Doxyfile

data_ref := $(addsuffix _ref.txt, \
              $(addprefix $(BUILD)/, \
                $(subst .txt,,$(notdir $(wildcard $(DATA)/*.txt)))))

.PHONY: data
data: $(data_ref)

.PHONY: clean
clean:
	$(RM) -r $(BUILD)/*.[od] $(BUILD)/hattrie $(BUILD)/rbtree \
		$(RM) $(BUILD)/slist_perf_run

.PHONY: dev
dev: | $(BUILD)
	ctags -f $(BUILD)/tags -R
	cd $(BUILD) && cscope -bq $(shell find $(CURDIR)/hat-trie $(SRC) -type f)

.PHONY: mrproper
mrproper:
	$(RM) -r $(BUILD) $(DOC)

.PHONY: pkg
pkg:
	sudo apt-get install html2text wget doxygen check valgrind linux-tools

objs := mapred.o work.o task.o token.o utils.o

$(BUILD)/mapred_hattrie: $(addprefix $(BUILD)/hattrie/, $(objs) token_hattrie.o)
	$(CC) $(CFLAGS) -I/usr/include/hat-trie \
		-include $(SRC)/token_hattrie.h -o $@ $^ -lhat-trie

$(BUILD)/hattrie/%.o: $(SRC)/%.c | $(BUILD)/hattrie
	$(CC) $(CFLAGS) -I/usr/include/hat-trie \
		-include $(SRC)/token_hattrie.h -o $@ -c $<

$(BUILD)/mapred_rbtree: $(addprefix $(BUILD)/rbtree/, $(objs) token_rbtree.o)
	$(CC) $(CFLAGS) -include $(SRC)/token_rbtree.h -o $@ $^

$(BUILD)/rbtree/%.o: $(SRC)/%.c | $(BUILD)/rbtree
	$(CC) $(CFLAGS) -include $(SRC)/token_rbtree.h -o $@ -c $<

$(BUILD)/ut_token_store_rbtree: $(addprefix $(BUILD)/rbtree/, token.o utils.o)

$(BUILD)/ut_%_rbtree: $(TEST)/ut_%.c | $(BUILD)
	$(CC) -I$(SRC) $(TEST_CFLAGS) -include $(SRC)/token_rbtree.h -o $@ \
		$(filter %.c, $^) $(filter %.o, $^) -lcheck -lm -lrt -lsubunit

$(BUILD)/ut_token_store_hattrie: $(addprefix $(BUILD)/rbtree/, token.o utils.o)

$(BUILD)/ut_%_hattrie: $(TEST)/ut_%.c | $(BUILD)
	$(CC) -I$(SRC) $(TEST_CFLAGS) -I/usr/include/hat-trie \
		-include $(SRC)/token_hattrie.h -o $@ \
		$(filter %.c, $^) $(filter %.o, $^) \
		-lcheck -lm -lrt -lhat-trie -lsubunit

$(BUILD)/%_uto.o: $(SRC)/%.c | $(BUILD)
	$(CC) -I$(SRC) $(TEST_CFLAGS) -o $@ -c $<

$(BUILD)/ut_slist: $(TEST)/ut_slist.c $(BUILD)/slist_uto.o
	$(CC) -I$(SRC) $(TEST_CFLAGS) -o $@ \
		$(filter %.c, $^) $(filter %.o, $^) -lcheck -lm -lrt -lsubunit

$(BUILD)/ut_%: $(TEST)/ut_%.c | $(BUILD)
	$(CC) -I$(SRC) $(TEST_CFLAGS) -o $@ $< -lcheck -lm -lrt -lsubunit

$(BUILD)/slist_perf_run: $(TEST)/slist_perf_run.c $(SRC)/slist.c
	$(CC) -I$(SRC) $(PERF_CFLAGS) -DSLIST_PERF_EVENTS -o $@ \
		$(filter %.c, $^)

$(BUILD)/%_ref.txt: $(DATA)/%.txt
	test/mkref.sh $<

$(DATA)/lipsum.txt: | $(BUILD)
	wget -O - http://fr.lipsum.com/feed/html | html2text -o $@

TEST_DATA := $(BUILD)/data

define slist_perf_nr
	$(subst nr,,$(word 2,$(subst _, ,$(basename $(1)))))
endef

define slist_perf_order
	$(subst ord,,$(word 3,$(subst _, ,$(basename $(1)))))
endef

slist_perf_data_samples_nr := 2 4 8 16 32 64 128 256 512 1024 2048 4096 8192 \
                              16384 32768 65536 131072 262144 524288 1048576 \
                              2097152 4194304
slist_perf_data_order_rate := 0 40 49 50 51 60 100
slist_perf_data_files      := $(foreach n,$(slist_perf_data_samples_nr),\
                                $(foreach r,$(slist_perf_data_order_rate),\
                                  int_nr$(n)_ord$(r)))

.PHONY: slist_perf_data
slist_perf_data: $(addsuffix .dat,\
                   $(addprefix $(TEST_DATA)/,$(slist_perf_data_files)))

$(TEST_DATA)/int_%.dat: $(TEST)/mkintdat.py | $(TEST_DATA)
	$< $(dir $@) $(call slist_perf_nr,$@) $(call slist_perf_order,$@)


slist_perf_result_files := $(foreach n,$(slist_perf_data_samples_nr),\
                             $(foreach r,$(slist_perf_data_order_rate),\
                               int_nr$(n)_ord$(r).dat))
slist_perf_loop_nr      := 100

.PHONY: slist_perf_run
slist_perf_run: $(addsuffix _insertion.txt,\
                  $(addprefix $(TEST_DATA)/,$(slist_perf_data_files)))

$(TEST_DATA)/int_%_insertion.txt: $(TEST)/slist_perf_run.py \
                                  $(BUILD)/slist_perf_run \
                                  $(TEST_DATA)/int_%.dat
	$< $(word 2,$^) $(dir $@) $(call slist_perf_nr,$@) \
		$(call slist_perf_order,$@) insertion $(slist_perf_loop_nr) > $@

$(BUILD) $(BUILD)/hattrie $(BUILD)/rbtree $(DOC) $(TEST_DATA):
	mkdir -p $@

-include $(wildcard $(BUILD)/*.d)
