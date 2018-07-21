#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Arraylist.h"

#define INITIAL_SIZE 10

// TODO(ym): error checking, etc
// TODO(ym): function that take indicies should take signed ints not unsigned ints

Arraylist *arraylist_create_default(void) {
	return arraylist_create(0);
}

Arraylist *arraylist_create(int size) {
	size = (size < INITIAL_SIZE) ? INITIAL_SIZE : size;

	Arraylist *arraylist = malloc(sizeof(Arraylist));
	if(arraylist == NULL) {
		return NULL;
	}
	arraylist->size = size;
	arraylist->filled = 0;
	arraylist->elements = calloc(size, sizeof(void *));
	if(arraylist->elements == NULL) {
		free(arraylist);
		return NULL;
	}
	return arraylist;
}

int arraylist_size(Arraylist *list) {
	return list->filled;
}

int arraylist_add(Arraylist *list, void *element) {
	if(check_size(list)) {
		arraylist_set_length(list, list->size * 2);
	}
	list->elements[(list->filled)++] = element;
	return 0;
}

Arraylist *arraylist_merge(Arraylist *list, Arraylist *other_list) {
	Arraylist *merged_list = arraylist_create(list->filled + other_list->filled);
	memcpy(merged_list->elements, list->elements, sizeof(void *) * list->filled);
	memcpy(merged_list->elements + list->filled, other_list->elements, sizeof(void *) * other_list->filled);
	merged_list->filled = list->filled + other_list->filled;
	return merged_list;
}

void *arraylist_get(Arraylist *list, unsigned int index) {
	if(index >= list->size || index >= list->filled) {
		return NULL;
	}
	return list->elements[index];
}

int arraylist_set(Arraylist *list, unsigned int index, void *element) {
	if(index >= list->filled) {
		return -1;
	}
	list->elements[index] = element;
	return 0;
}

int arraylist_contains(Arraylist *list, void *element) {
	for(unsigned int i = 0; i < list->filled; i++) {
		if(list->elements[i] == element) {
			return 1;
		}
	}
	return 0;
}

void arraylist_remove(Arraylist *list, int index) {
	memmove(list->elements + index, list->elements + index + 1, sizeof(void *) * abs(index - (list->filled - 1)));
	--(list->filled);
}

int arraylist_remove_object(Arraylist *list, void *element) {
	for(unsigned int i = 0; i < list->filled; i++) {
		if(list->elements[i] == element) {
			arraylist_remove(list, i);
			return 1;
		}
	}
	return 0;
}

int arraylist_remove_range(Arraylist *list, unsigned int from_index, unsigned int to_index) {
	if(from_index >= list->filled || to_index >= list->filled) {
		return 0;
	}

	memmove(list->elements + from_index, list->elements + to_index, sizeof(void *) * abs(to_index - list->filled));
	list->filled = from_index + abs(to_index - list->filled);
	return 1;
}

int arraylist_set_length(Arraylist *list, int new_size) {
	if(new_size < 0) {
		return -1;
	}
	void *new_block = realloc(list->elements, new_size * sizeof(void *));
	if(new_block == NULL) {
		return -1;
	}
	list->elements = new_block;
	list->size = new_size;
	if(new_size < list->filled) {
		list->filled = new_size;
	}
	return 0;
}

int arraylist_free(Arraylist *list) {
	free(list->elements);
	free(list);
	return 1;
}

int arraylist_trim_to_size(Arraylist *list) {
	arraylist_set_length(list, list->filled);
	return 1;
}

int arraylist_insert(Arraylist *list, int pos, void *element) {
	if(check_size(list)) {
		arraylist_set_length(list, list->size * 2);
	}
	memmove(list + pos + 1, list + pos, list->filled - pos);
	list->elements[pos] = element;
	return 1;
}

int check_size(Arraylist *list) {
	return !(list->filled == list->size);
}
