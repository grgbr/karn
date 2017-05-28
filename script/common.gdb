macro define offsetof(type, field) \
	&((type *) 0)->field)

macro define containerof(pointer, type, field) \
	(type *) ((char *)(pointer) - (char *)offsetof(type, field))
