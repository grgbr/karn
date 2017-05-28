#include "token.h"
#include <stdio.h>
#include <check.h>

/******************************************************************************
 * These are functions needed by token store object: override them with dummy
 * implementation...
 ******************************************************************************/

void
free(void *ptr __unused)
{
}

void
mapred_clear_token_store(struct mapred_token_store *store __unused)
{
}

int
mapred_flatten_token_store(struct mapred_token_store *store __unused)
{
	return 0;
}

int
mapred_register_token(struct mapred_token_store *store __unused,
                      const char                *data __unused,
                      size_t                     length __unused)
{
	return 0;
}

/******************************************************************************
 * Real testing starts here.
 ******************************************************************************/

#define UT_INIT_TOKEN(_data, _rate)              \
	{                                        \
		.tok_data   = _data,             \
		.tok_length = sizeof(_data) - 1, \
		.tok_rate   = _rate              \
	}

static struct mapred_token_store ut_result_token_store;
static struct mapred_token_store ut_source_token_store;

static void
ut_setup_token_stores(struct mapred_token *result_tokens,
                      unsigned int         result_count,
                      struct mapred_token *source_tokens,
                      unsigned int         source_count)
{
	unsigned int c;

	slist_init(&ut_result_token_store.tok_list);
	for (c = 0; c < result_count; c++)
		slist_nqueue(&ut_result_token_store.tok_list,
		             &result_tokens[c].tok_node);
	ut_result_token_store.tok_count = result_count;

	slist_init(&ut_source_token_store.tok_list);
	for (c = 0; c < source_count; c++)
		slist_nqueue(&ut_source_token_store.tok_list,
		             &source_tokens[c].tok_node);
	ut_source_token_store.tok_count = source_count;
}

static void
ut_check_token_store_merge(struct mapred_token       *result_tokens,
                           unsigned int               result_count,
                           struct mapred_token       *source_tokens,
                           unsigned int               source_count,
                           const struct mapred_token *check_tokens,
                           unsigned int               check_count)
{
	struct mapred_token *tok;
	unsigned int         cnt = 0;

	ut_setup_token_stores(result_tokens, result_count,
	                      source_tokens, source_count);

	mapred_merge_token_store(&ut_result_token_store,
	                         &ut_source_token_store);

	slist_foreach_entry(&ut_result_token_store.tok_list, tok, tok_node) {
		ck_assert_str_eq(tok->tok_data, check_tokens[cnt].tok_data);
		cnt++;
	}

	ck_assert_uint_eq(cnt, check_count);
	ck_assert_uint_eq(ut_result_token_store.tok_count, check_count);
	ck_assert_uint_eq(ut_source_token_store.tok_count, 0);
}

START_TEST(ut_test_token_store_merge_00)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("0", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("0", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 2),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_002)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("0", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("2", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 2),
		UT_INIT_TOKEN("2", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_0025)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("5", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("2", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 2),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("5", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_15)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("5", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("1", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("5", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_inorder_012378)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_disorder_012378)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_inorder_0123456789)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("4", 1),
		UT_INIT_TOKEN("6", 1),
		UT_INIT_TOKEN("8", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("5", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("9", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("4", 1),
		UT_INIT_TOKEN("5", 1),
		UT_INIT_TOKEN("6", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
		UT_INIT_TOKEN("9", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

START_TEST(ut_test_token_store_merge_disorder_0123456789)
{
	struct mapred_token result_tokens[] = {
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("5", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("9", 1),
	};
	struct mapred_token source_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("4", 1),
		UT_INIT_TOKEN("6", 1),
		UT_INIT_TOKEN("8", 1),
	};
	const struct mapred_token check_tokens[] = {
		UT_INIT_TOKEN("0", 1),
		UT_INIT_TOKEN("1", 1),
		UT_INIT_TOKEN("2", 1),
		UT_INIT_TOKEN("3", 1),
		UT_INIT_TOKEN("4", 1),
		UT_INIT_TOKEN("5", 1),
		UT_INIT_TOKEN("6", 1),
		UT_INIT_TOKEN("7", 1),
		UT_INIT_TOKEN("8", 1),
		UT_INIT_TOKEN("9", 1),
	};

	ut_check_token_store_merge(result_tokens, array_count(result_tokens),
	                           source_tokens, array_count(source_tokens),
	                           check_tokens,  array_count(check_tokens));
}
END_TEST

static void ut_create_token_store_merge_tcase(Suite* suite)
{
	TCase* tc;

	tc = tcase_create("Merge");

	tcase_add_test(tc, ut_test_token_store_merge_00);
	tcase_add_test(tc, ut_test_token_store_merge_002);
	tcase_add_test(tc, ut_test_token_store_merge_0025);
	tcase_add_test(tc, ut_test_token_store_merge_15);
	tcase_add_test(tc, ut_test_token_store_merge_inorder_012378);
	tcase_add_test(tc, ut_test_token_store_merge_disorder_012378);
	tcase_add_test(tc, ut_test_token_store_merge_inorder_0123456789);
	tcase_add_test(tc, ut_test_token_store_merge_disorder_0123456789);

	suite_add_tcase(suite, tc);
}

static Suite* ut_create_token_store_suite(void)
{
	Suite* s;
	
	s = suite_create("Token store");

	ut_create_token_store_merge_tcase(s);

	return s;
}

int main(int argc, char const* argv[])
{
	Suite*   s;
	SRunner* r;
	int      cnt;
	
	s = ut_create_token_store_suite();
	r = srunner_create(s);
	
	if (argc == 2) {
		srunner_set_fork_status(r, CK_NOFORK);
		srunner_run(r, NULL, argv[1], CK_VERBOSE);
	}
	else if (argc == 1)
		srunner_run_all(r, CK_VERBOSE);
	else
		fprintf(stderr, "invalid number of arguments\n");

	cnt = srunner_ntests_failed(r);
	srunner_free(r);

	return (!cnt) ? EXIT_SUCCESS : EXIT_FAILURE;
}
