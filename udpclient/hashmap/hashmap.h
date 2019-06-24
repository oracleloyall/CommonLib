#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif
#define MAP_MISSING -3  /* No such element */
#define MAP_FULL -2 	/* Hashmap is full */
#define MAP_OMEM -1 	/* Out of Memory */
#define MAP_OK 0 	/* OK */
#define INITIAL_SIZE 2048
#define MAX_CHAIN_LENGTH 16

typedef void *any_t;

//map_t is a pointer to an internally maintained data structure.
typedef any_t map_t;

//Return an empty hashmap. Returns NULL if empty
extern map_t hashmap_new(int initSize);

//Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
extern int hashmap_put(map_t in, char* key, any_t value);

//Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
extern int hashmap_get(map_t in, char* key, any_t *arg);

//Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
extern int hashmap_remove(map_t in, char* key);

extern void hashmap_free(map_t in);

extern int hashmap_length(map_t in);
#ifdef __cplusplus
}
#endif
#endif // HASHMAP_H_INCLUDED
