#include <karn/bheap.h>

void bheaping_withkeys_onstack(void)
{
	int                keys[32]; /* integer keys preallocated on stack */
	struct bheap_fixed heap;     /* fixed length array based binary heap */

	bheap_init_fixed(&heap,           /* heap to init */
	                 (char *)keys,    /* keys memory area */
	                 sizeof(keys[0]), /* size of a single key */
	                 array_nr(keys)); /* max number of keys held by heap */

	/* do some stuff... */

	bheap_fini_fixed(&heap);
}
