#include <karn/bheap.h>
#include <stdlib.h>
#include <errno.h>

#define KEYS_NR (32U)

int bheaping_withkeys_malloced(void)
{
	int                *keys; /* integer keys pre-malloc'ed on heap */
	struct bheap_fixed  heap; /* fixed length array based binary heap */

	keys = malloc(KEYS_NR * sizeof(*keys));
	if (!keys)
		return -ENOMEM;

	bheap_init_fixed(&heap,         /* heap to init */
	                 (char *)keys,  /* keys memory area */
	                 sizeof(*keys), /* size of a single key */
	                 KEYS_NR);      /* max number of keys held by heap */

	/* do some stuff... */

	bheap_fini_fixed(&heap);

	free(keys);

	return 0;
}
