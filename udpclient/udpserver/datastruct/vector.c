
#include <stdio.h>
#include <stdlib.h>

#include "vector.h"

vector* vector_init()
{
    vector* v = malloc(sizeof(vector));
	v->data = NULL;
	v->count = 0;
	v->size = INITIAL_VECTOR_SIZE;
    v->data = malloc(sizeof(void*) * v->size);
    memset(v->data, '\0', sizeof(void*) * v->size);
    return v;
}

//get vector length
int vector_count(vector *v)
{
	return v->count;
}

//add new element to last of vector, return length of vector
void vector_add(vector *v, void *e){
    if(v->size == v->count){
        //check if vector is full
        v->size *= 2;
        v->data = realloc(v->data, sizeof(void*) * v->size);
    }

    v->data[v->count] = e;
    v->count++;
}

//change value of an element, return -1 if outOfRange
int vector_set(vector *v, int index, void *e)
{
	if (index >= v->count || index < 0) {
		return -1;
	}

	v->data[index] = e;
	return index;
}

void *vector_get(vector *v, int index)
{
	if (index >= v->count || index < 0) {
		return NULL;
	}
    void* s = v->data[index];
	return v->data[index];
}

//delete element
void vector_delete(vector *v, int index)
{
	if (index >= v->count || index < 0) {
		return;
	}
    int i = 0;
	for(i = index; i < v->count - 1; i++){
        v->data[index] = v->data[index + 1];
	}
	v->count--;
	free(v->data[v->count]);
}

void vector_free(vector *v)
{
    int i;
    for(i = 0; i < v->count; i++){
        free(v->data[i]);
	}
	free(v->data);
}
