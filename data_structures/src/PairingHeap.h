#include "Arraylist.h"

typedef struct {
	void *element;
	Arraylist *children;
	int (*comp)(void *, void *);
} Heap;
/* } HeapNode; */

/* TODO(ym): refactor to this? */
/* typedef struct { */
/* 	HeapNode *root; */
/* 	int (*comp)(void *, void *); */
/* } Heap; */

Heap *heap_create(int (*)(void *, void *));
Heap *heap_create_element(void *, int (*)(void *, void *));

Heap *heap_merge(Heap *, Heap *);
Heap *heap_merge_destructive(Heap *, Heap *);

Heap *heap_insert(Heap *, void *);
Heap *merge_pairs(Arraylist *, int);

Heap *heap_delete_min(Heap *);
void *heap_getmin(Heap *);

void heap_free(Heap *);
Heap *heap_shallow_copy(Heap *);
