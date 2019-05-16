#include <stdio.h>
#include "PairingHeap.h"
#include "Arraylist.h"

int comparator(void *first, void *second);
void print_tree(Heap *, Arraylist *, int);
void test_heap(void);
void test_arraylist(void);
void print_arraylist(Arraylist *);

int main() {
	test_arraylist();
	test_heap();
	/* run(); */
}

void print_tree(Heap *heap, Arraylist *indent, int is_tail) {
	for(int i = 0; i < arraylist_size(indent); i++) {
		printf("%s",  (char *) arraylist_get(indent, i));
	}
	printf("%s%d\n", is_tail ? "└── " : "├── ", * (int *) heap->element);

	Arraylist *children = heap->children;
	arraylist_add(indent, is_tail ? "    " :  "│   ");

	for(int i = 0; i < arraylist_size(children); i++) {
		print_tree(arraylist_get(children, i), indent, i == (arraylist_size(children) - 1));
	}
	arraylist_remove(indent, arraylist_size(indent) - 1);
}

int comparator(void *first, void *second) {
	if(* (int *) second < * (int *) first) {
		return 1;
	} else {
		return -1;
	}
}

void test_arraylist(void) {
	int array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36  };
	Arraylist *test = arraylist_create_default();
	for(int i = 0; i < 37; i++) {
		arraylist_add(test, array + i);
	}

	printf("%s", "After add: ");
	print_arraylist(test);

	Arraylist *test2 = arraylist_create_default();
	int array2[] = { 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };
	for(int i = 0; i < 37; i++) {
		arraylist_add(test2, array2 + i);
	}

	printf("%s", "After merge: ");
	Arraylist *merged_list = arraylist_merge(test, test2);
	print_arraylist(merged_list);
	arraylist_free(merged_list);

	printf("%s", "After set: ");
	arraylist_set(test, arraylist_size(test) - 1, array);
	print_arraylist(test);

	printf("%s", "After remove: ");
	arraylist_remove(test, arraylist_size(test) - 1);
	print_arraylist(test);

	printf("%s", "After remove range: ");
	arraylist_remove_range(test, 10, 20);
	print_arraylist(test);

	printf("%s", "After set length: ");
	arraylist_set_length(test, 10);
	print_arraylist(test);

	arraylist_free(test);
	arraylist_free(test2);
	printf("%s\n", "Done!");
}

void print_arraylist(Arraylist *list) {
	for(int i = 0; i < arraylist_size(list); i++){
		printf("%d ", *(int *)arraylist_get(list, i));
	}
	printf("\n");
}

void test_heap(void) {
	int array[] = { 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 15, 14, 17, 16, 19, 18, 21, 20, 23, 22, 25, 24, 27, 26, 29, 28, 30, 12, 32, 31, 34, 33, 36, 35, 38, 37, 39, 40, 10 };
	Heap *heap = heap_create_element(array, comparator);
	for(int i = 1; i < sizeof(array) / sizeof(array[0]); i++) {
		heap = heap_insert(heap, array + i);
	}
	Arraylist *list;
	if((list = arraylist_create_default())) {
		print_tree(heap, list, 1);
		arraylist_free(list);
	}
	heap = heap_delete_min(heap);
	if((list = arraylist_create_default())) {
		print_tree(heap, list, 1);
		arraylist_free(list);
	}
	heap_free(heap);
}
