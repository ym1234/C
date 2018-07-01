#include "Arraylist.h"

typedef struct {
	void *element;
	Arraylist *children;
} Heap;
/* } HeapNode; */

/* TODO(ym): refactor */
/* typedef struct { */
/* 	HeapNode *root; */
/* 	int (*comp)(void *, void *); */
/* } Heap; */

Heap *heap_create(int (*comp)(void *, void *));
Heap *heap_create_element(void *element, int (*comp)(void *, void *));
Heap *heap_shallow_copy(Heap *heap);
Heap *heap_merge(Heap *heap, Heap *second_heap);
Heap *heap_merge_destructive(Heap *heap, Heap *second_heap);
Heap *heap_insert(Heap *heap, void *element);
void heap_free(Heap *heap);
Heap *merge_pairs(Arraylist *children, int start);
Heap *heap_delete_min(Heap *heap);
void *heap_getmin(Heap *heap);
