#ifndef ARRAYLIST_H_
#define ARRAYLIST_H_

typedef struct {
	unsigned int size;
	unsigned int filled;
	void **elements;
} Arraylist;

Arraylist *arraylist_create(int);
Arraylist *arraylist_create_default(void);

int arraylist_add(Arraylist *, void *);
int arraylist_contains(Arraylist *, void *);
void *arraylist_get(Arraylist *, unsigned int);

void arraylist_remove(Arraylist *, int);
int arraylist_remove_object(Arraylist *, void *);
int arraylist_remove_range(Arraylist *, unsigned int, unsigned int);

int arraylist_free(Arraylist *);
int arraylist_trim_to_size(Arraylist *);
Arraylist *arraylist_merge(Arraylist *, Arraylist *);

int arraylist_size(Arraylist *);
int arraylist_set(Arraylist *, unsigned int, void *);
int arraylist_set_length(Arraylist *, int);

int check_size(Arraylist *list);
int arraylist_insert(Arraylist *list, int pos, void *element);

#endif
