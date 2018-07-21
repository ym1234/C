#include "Arraylist.h"
#include "PairingHeap.h"

#include <stdlib.h>

Heap *heap_create(int (*comp)(void *, void *)) {
	Heap *heap = malloc(sizeof(Heap));
	heap->children = arraylist_create_default();
	heap->element = NULL;
	heap->comp = comp;
	return heap;
}
Heap *heap_create_element(void *element, int (*comp)(void *, void *)) {
	Heap *heap = heap_create(comp);
	heap->element = element;
	return heap;
}

Heap *heap_shallow_copy(Heap *heap) {
	Heap *new_heap = heap_create_element(heap->element, heap->comp);
	arraylist_free(new_heap->children);
	new_heap->children = heap->children;
	return new_heap;
}

Heap *heap_merge(Heap *heap, Heap *second_heap) {
	int result = heap->comp(heap->element, second_heap->element);
	Heap *head = heap_shallow_copy(result == 1 ? second_heap : heap);
	arraylist_add(head->children, result == 1 ? heap : second_heap);
	return head;
}

Heap *heap_merge_destructive(Heap *heap, Heap *second_heap) {
	if(heap == NULL || second_heap == NULL) {
		return (heap == NULL) ? second_heap : heap;
	}
	int result = heap->comp(heap->element, second_heap->element);
	Heap *head = (result == 1) ? second_heap : heap;
	arraylist_add(head->children, (result == 1) ? heap : second_heap);
	return head;
}

Heap *heap_insert(Heap *heap, void *element) {
	return heap_merge_destructive(heap, heap_create_element(element, heap->comp));
}

void heap_free(Heap *heap) {
	arraylist_free(heap->children);
	free(heap);
}

Heap *merge_pairs(Arraylist *children, int start) {
	int size = arraylist_size(children) - start;
	if(size < 0) {
		return NULL;
	} else if(size == 1) {
		return arraylist_get(children, start);
	} else {
		return heap_merge_destructive(heap_merge_destructive(arraylist_get(children, start + 0), arraylist_get(children, start + 1)), merge_pairs(children, start + 2));
	}
}

Heap *heap_delete_min(Heap *heap) {
	Arraylist *children = heap->children;
	free(heap);
	return merge_pairs(children, 0);
}

void *heap_getmin(Heap *heap) {
	return heap->element;
}
