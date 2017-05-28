#ifndef _TOKEN_H
#define _TOKEN_H

#include <sys/types.h>

/******************************************************************************
 * Token store
 ******************************************************************************/

struct slist;
struct mapred_token_store;

extern int  mapred_compare_token(const void *first, const void *second);

extern int  mapred_tokenize(struct mapred_token_store *store,
                            const char                *data,
                            size_t                     size);

extern void mapred_merge_token_store(struct mapred_token_store *result,
                                     struct mapred_token_store *source);

extern int  mapred_dump_token_store(const struct mapred_token_store *store);

extern int  mapred_init_token_store(struct mapred_token_store *store);

extern void mapred_fini_token_store(const struct mapred_token_store *store);

#endif
