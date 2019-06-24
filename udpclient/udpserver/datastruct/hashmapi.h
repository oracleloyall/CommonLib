#ifndef HASHMAPI_H_INCLUDED
#define HASHMAPI_H_INCLUDED

#include "../hashmap.h"
#ifdef __cplusplus
extern "C" {
#endif


//Return an empty hashmap. Returns NULL if empty
extern map_t hashmapi_new(int initSize);

//Add an element to the hashmap. Return MAP_OK or MAP_OMEM.
extern int hashmapi_put(map_t in, int key, any_t value);

//Get an element from the hashmap. Return MAP_OK or MAP_MISSING.
extern int hashmapi_get(map_t in, int key, any_t *arg);

//Remove an element from the hashmap. Return MAP_OK or MAP_MISSING.
extern int hashmapi_remove(map_t in, int key);

extern void hashmapi_free(map_t in);

extern int hashmapi_length(map_t in);


#ifdef __cplusplus
}
#endif
#endif // HASHMAPI_H_INCLUDED
