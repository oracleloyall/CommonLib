#include <stdlib.h>
#include <stdio.h>
#include "hashmapi.h"

typedef struct _hashmapi_element{
    int key;
    int in_use;
    any_t data;
} hashmapi_element;

typedef struct _hasmapi_map{
    int table_size;
    int size;
    hashmapi_element* data;
} hashmapi_map;

map_t hashmapi_new(int initSize){
    hashmapi_map* m = (hashmapi_map *)malloc(sizeof(hashmapi_map));
    if(!m) goto err;

    m->data = (hashmapi_element *)calloc(INITIAL_SIZE, sizeof(hashmapi_element));
    if(!m->data) goto err;

    m->table_size = initSize;
    m->size = 0;

    return m;

    err:
        if(m) hashmapi_free(m);
        return NULL;
}

unsigned int hashmapi_hash_int(hashmapi_map * m, unsigned int key){

    /* Robert Jenkins' 32 bit Mix Function */
    key += (key << 12);
    key ^= (key >> 22);
    key += (key << 4);
    key ^= (key >> 9);
    key += (key << 10);
    key ^= (key >> 2);
    key += (key << 7);
    key ^= (key >> 12);

    key = (key >> 3) * 2654435761;
    return key % m->table_size;
}

int hashmapi_hash(map_t in, int key){
    int curr;
    int i;

    hashmapi_map* m = (hashmapi_map *) in;
    //If full, return immediately
    if(m->size >= (m->table_size)) return MAP_FULL;
    curr = hashmapi_hash_int(m, key);

    for(i = 0; i < MAX_CHAIN_LENGTH; i++){//linear seach for next free slot
        if(m->data[curr].in_use == 0)
            return curr;
        if(m->data[curr].in_use == 1 && m->data[curr].key == key)
            return curr;

        curr = (curr + 1) % m->table_size;
    }
    //if can't seacrh a free slot, return MAP_FULL
    return MAP_FULL;
}

int hashmapi_rehash(map_t in){
    int i;
    int old_size;
    hashmapi_element* curr;
    hashmapi_map *m = (hashmapi_map *) in;
    hashmapi_element *temp = (hashmapi_element *) calloc(2 * m->table_size, sizeof(hashmapi_element));
    if(!temp) return MAP_OMEM;

    //init new meomory, it's size is double old size for data
    free(m->data);
    curr = m->data;
    m->data = temp;
    //update the size
    old_size = m->table_size;
    m->table_size = 2 * m->table_size;
    m->size = 0;//clear size
    //rehash the element
    for(i = 0; i < old_size; i++){
        int status;
        if(curr[i].in_use == 0){
            continue;
        }
        //put new element
        status = hashmapi_put(m, curr[i].key, curr[i].data);
        if(status != MAP_OK){
            return status;
        }
    }

    free(curr);
    return MAP_OK;
}

int hashmapi_put(map_t in, int key, any_t value){
    int index;
    hashmapi_map* m = (hashmapi_map *)in;
    //find place to put value
    index = hashmapi_hash(in, key);//if MAP_FULL rehash the table
    int numRe = 0;
    while(index == MAP_FULL){
        if(numRe > 2) return MAP_OMEM;//check if rehash too many time
        if(hashmapi_rehash(in) == MAP_OMEM){
            return MAP_OMEM;
        }
        numRe++;
        //after rehash it, put new value
        index = hashmapi_hash(in, key);
    }

    //set data
    m->data[index].data = value;
    m->data[index].key = key;
    m->data[index].in_use = 1;
    m->size++;

    return MAP_OK;
}

int hashmapi_get(map_t in, int key, any_t* arg){
    int curr;
    int i;
    hashmapi_map* m;
    m = (hashmapi_map *)in;
    //lay ra hash  value theo key
    curr = hashmapi_hash_int(m, key);
    int in_use;
    //duyet tat cac cac phan tu co chung 1 hash value trong hashmap
    for(i = 0; i < MAX_CHAIN_LENGTH; i++){
        in_use = m->data[curr].in_use;
        if(in_use == 1){
            if(m->data[curr].key == key){//neu tim thay phan tu co key dung
                *arg = (m->data[curr].data);
                int askk = *(int*)m->data[curr].data;
                //int aska = *(int*)m->data[21].data;
                //int askaa = *(int*)m->data[30].data;
                return MAP_OK;
            }
        }
        curr = (curr + 1) % m->table_size;
    }
    *arg = NULL;

    return MAP_MISSING;
}

int hashmapi_remove(map_t in, int key){
    int i;
    int curr;
    hashmapi_map *m;
    m = (hashmapi_map *) in;
    curr = hashmapi_hash_int(m, key);
    int in_use;

    for(i = 0; i < MAX_CHAIN_LENGTH; i++){
        in_use = m->data[curr].in_use;
        if(in_use == 1){
            if(m->data[curr].key == key){
                m->data[curr].in_use = 0;
                m->data[curr].data = NULL;
                m->data[curr].key = NULL;
                m->size--;
                return MAP_OK;
            }
        }
        curr = (curr + 1) % m->table_size;
    }
    return MAP_MISSING;
}

void hashmapi_free(map_t in){
    hashmapi_map *m = (hashmapi_map *)in;
    free(m->data);
    free(m);
}

int hashmapi_length(map_t in){
    hashmapi_map *m = (hashmapi_map *)in;
    if(m != NULL) return m->size;
    else return 0;
}
