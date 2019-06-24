/*
 * Singleton.c
 *
 *  Created on: Mar 16, 2017
 *      Author: oracle
 */
#include"Singleton.h"
#ifndef NULL
#define NULL (0)
#endif
static void *csingletonCtor(void *_self) {
    _CSingleton *self = _self;

    self->instance = _self;

    return self;
}

static void *csingletonDtor(void *_self) {
    _CSingleton *self = _self;

    self->instance = NULL;

    return self;
}

static void *csingletonCreateInstance(void *_self) {
    _CSingleton *self = _self;

    self->instance = _self;

    return self;
}
INI pase;
static _CSingleton _csingleton = {
    csingletonCtor, csingletonDtor, csingletonCreateInstance, &pase
};

const void *CSingleton = &_csingleton;

void *GetInstance(void) {
    if (NULL == ((_CSingleton*)CSingleton)->instance) {
        return csingletonCtor(CSingleton);
    } else {
        return ((_CSingleton*)CSingleton)->instance;
    }
}

