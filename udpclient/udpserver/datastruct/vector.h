#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#define INITIAL_VECTOR_SIZE 4
#ifdef __cplusplus
extern "C" {
#endif


typedef struct vector_ {
    void** data;
    int size;
    int count;
} vector;

vector* vector_init();
int vector_count(vector*);
void vector_add(vector*, void*);
int vector_set(vector*, int, void*);
void *vector_get(vector*, int);
void vector_delete(vector*, int);
void vector_free(vector*);

#ifdef __cplusplus
}
#endif
#endif // VECTOR_H_INCLUDED
