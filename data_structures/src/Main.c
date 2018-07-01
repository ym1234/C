#include <stdio.h>
#include "PairingHeap.h"
#include "Arraylist.h"

int comparator(void *first, void *second);
void print_tree(Heap *, Arraylist *, int);
int test_heap(void);
int test_arraylist(void);
void print_arraylist(Arraylist *);

int main() {
	test_heap();
	test_arraylist();
	/* run(); */
}

void print_tree(Heap *heap, Arraylist *indent, int is_tail) {
	for(int i = 0; i < arraylist_size(indent); i++) {
		printf("%s", arraylist_get(indent, i));
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
	if((*((int *)second)) < (*((int *)first))) {
		return 1;
	} else {
		return -1;
	}
}

int test_arraylist(void) {
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

	// TODO(ym): fix memleak
	printf("%s", "After merge: ");
	print_arraylist(arraylist_merge(test, test2));

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

int test_heap(void) {
	// TODO(ym): fix memleak(s)
	/* int array[] = { 1, 3, 2, 10, 6, 24, 100, 39, 53, 87, 95, 11, 62, 5, 62, 40, 18, 11, 23, 15, 31, 14, 9, 46, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37 }; */
	int array[] = { 1, 3, 2, 10, 6, 7, 8, 10, 11, 12 };
	Heap *heap = heap_create_element(array, comparator);
	/* for(int i = 1; i < 35; i++) { */
	for(int i = 1; i < (sizeof(array) / sizeof(array[0])); i++) {
		heap = heap_insert(heap, array + i);
	}
	print_tree(heap, arraylist_create_default(), 1);
	puts("\n");
	heap = heap_delete_min(heap);
	print_tree(heap, arraylist_create_default(), 1);
}
