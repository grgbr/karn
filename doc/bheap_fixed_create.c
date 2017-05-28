#include <karn/bheap.h>
#include <errno.h>

#define KEYS_NR (32U)

int bheaping_withkeys_created_bykarn(void)
{
	struct bheap_fixed *heap; /* fixed length array based binary heap */

	heap = bheap_create_fixed(sizeof(int), KEYS_NR);
	if (!heap)
		return -errno;

	/* do some stuff... */

	bheap_destroy_fixed(heap);

	return 0;
}
