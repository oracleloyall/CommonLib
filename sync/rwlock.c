/*
 *  Project: Read-Write Lock for C
 *  File:   rwlock.c
 *
 */
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "rwlock.h"

struct ReadWriteLock_s
{
    sem_t wrt;
    sem_t mtx;
    sem_t delFlag;
    int readcount;
    int active;
};
//typedef struct ReadWriteLock_s* ReadWriteLock;
//forward declaration
/* This function is used to take the state of the lock.
 * Return values:
 *      [*] 1 is returned when the lock is alive.
 *      [*] 0 is returned when the lock is marked for delete.
 *      [*] -1 is returned if an error was encountered.
 */
int rwl_isActive(ReadWriteLock);

int rwl_init(ReadWriteLock* lock)
{
	(*lock) = (ReadWriteLock) malloc(sizeof(struct ReadWriteLock_s));
    if ((*lock) == NULL)
    {
        perror("rwl_init - could not allocate memory for lock\n");
        return -1;
    }
    if (sem_init(&((*lock)->wrt), 0, 1) == -1)
    {
        perror("rwl_init - could not allocate wrt semaphore\n");
        free(*lock);
        *lock = NULL;
        return -1;
    }
    if (sem_init(&((*lock)->mtx), 0, 1) == -1)
    {
        perror("rwl_init - could not allocate mtx semaphore\n");
        sem_destroy(&((*lock)->wrt));
        free(*lock);
        *lock = NULL;
        return -1;
    }
    if (sem_init(&((*lock)->delFlag), 0, 1) == -1)
    {
        perror("rwl_init - could not allocate delFlag semaphore\n");
        sem_destroy(&((*lock)->wrt));
        sem_destroy(&((*lock)->mtx));
        free(*lock);
        *lock = NULL;
        return -1;
    }
    (*lock)->readcount = 0;
    (*lock)->active = 1;
    return 0;
}

int rwl_destroy(ReadWriteLock* lock)
{
    if (lock == NULL) {
        perror("rwl_destroy - NULL argument");
        return -1;
    }
    errno = 0;
    if (sem_trywait(&((*lock)->wrt)) == -1)
        perror("rwl_destroy - trywait on wrt failed.");
    if ( errno == EAGAIN)
        perror("rwl_destroy - wrt is locked, undefined behaviour.");

    errno = 0;
    if (sem_trywait(&((*lock)->mtx)) == -1)
        perror("rwl_destroy - trywait on mtx failed.");
    if ( errno == EAGAIN)
        perror("rwl_destroy - mtx is locked, undefined behaviour.");

    if (sem_destroy(&((*lock)->wrt)) == -1)
        perror("rwl_destroy - destroy wrt failed");
    if (sem_destroy(&((*lock)->mtx)) == -1)
        perror("rwl_destroy - destroy mtx failed");
    if (sem_destroy(&((*lock)->delFlag)) == -1)
        perror("rwl_destroy - destroy delFlag failed");

    free(*lock);
    *lock = NULL;
    return 0;
}

int rwl_writeLock(ReadWriteLock lock)
{
    if (lock == NULL) {
        perror("rwl_writeLock - NULL argument");
        return -1;
    }
    if (sem_wait(&(lock->wrt)) == -1)
    {
        perror("rwl_writeLock - wait on wrt failed.");
        return -1;
    }
    return 0;
}

int rwl_writeUnlock(ReadWriteLock lock)
{
    if (lock == NULL) {
        perror("rwl_writeUnlock - NULL argument");
        return -1;
    }
    if (sem_post(&(lock->wrt)) == -1)
    {
        perror("rwl_writeUnlock - post on wrt failed.");
        return -1;
    }
    return 0;
}

int rwl_readLock(ReadWriteLock lock)
{
    if (lock == NULL) {
        perror("rwl_readLock - NULL argument");
        return -1;
    }
    if (sem_wait(&(lock->mtx)) == -1)
    {
        perror("rwl_readLock - wait on mtx failed");
        return -1;
    }

    lock->readcount += 1;
    if (lock->readcount == 1)
        if (sem_wait(&(lock->wrt)) == -1)
        {
            perror("rwl_readLock - wait on wrt failed");
            sem_post(&(lock->mtx)); //return semaphore to original state
            return -1;
        }
    if (sem_post(&(lock->mtx)) == -1)
    {
        perror("rwl_readLock - post on mtx failed, undefined behaviour");
        return -1;
    }
    return 0;
}

int rwl_readUnlock(ReadWriteLock lock)
{
    if (lock == NULL) {
        perror("rwl_readUnlock - NULL argument");
        return -1;
    }
    if (sem_wait(&(lock->mtx)) == -1)
    {
        perror("rwl_readUnlock - wait on mtx failed");
        return -1;
    }
    lock->readcount -= 1;
    if (lock->readcount == 0)
        if (sem_post(&(lock->wrt)) == -1)
        {
            perror("rwl_readUnlock - post on wrt failed");
            sem_post(&(lock->mtx)); //return semaphore to original state
            return -1;
        }
    if (sem_post(&(lock->mtx)) == -1)
    {
        perror("rwl_readUnlock - post on mtx failed, undefined behaviour");
        return -1;
    }
    return 0;
}

int rwl_isActive(ReadWriteLock lock)
{
    if (lock == NULL) {
        perror("rwl_isActive - NULL argument");
        return -1;
    }
    errno = 0;
    if (sem_trywait(&(lock->delFlag)) == -1)
    {
        perror("rwl_isActive - trywait on delFlag failed.");
        return -1;
    }
    if ( errno == EAGAIN)
    {//delFlag is down, lock is marked for delete
        perror("rwl_isActive - tried to lock but ReadWriteLock was marked for delete");
        return 0;
    }
    return 1;
}
