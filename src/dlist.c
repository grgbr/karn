#include <karn/dlist.h>

void dlist_splice(struct dlist_node *restrict at,
                  struct dlist_node *first,
                  struct dlist_node *last)
{
	dlist_withdraw(first, last);
	dlist_embed(at, first, last);
}
