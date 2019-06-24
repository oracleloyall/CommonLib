#ifndef JSW_RBTREE_H
#define JSW_RBTREE_H

#ifdef __cplusplus
#include <cstddef>

using std::size_t;

extern "C" {
#else
#include <stddef.h>
#endif

typedef struct jsw_rbtree jsw_rbtree_t;
typedef struct jsw_rbtrav jsw_rbtrav_t;


typedef int   (*cmp_f) ( const void *p1, const void *p2 );//compare element function
typedef void *(*dup_f) ( void *p );
typedef void  (*rel_f) ( void *p );


jsw_rbtree_t *jsw_rbnew ( cmp_f cmp, dup_f dup, rel_f rel );
void          jsw_rbdelete ( jsw_rbtree_t *tree );
void         *jsw_rbfind ( jsw_rbtree_t *tree, void *data );
int           jsw_rbinsert ( jsw_rbtree_t *tree, void *data );
int           jsw_rberase ( jsw_rbtree_t *tree, void *data );
size_t        jsw_rbsize ( jsw_rbtree_t *tree );


jsw_rbtrav_t *jsw_rbtnew ( void );
void          jsw_rbtdelete ( jsw_rbtrav_t *trav );
void         *jsw_rbtfirst ( jsw_rbtrav_t *trav, jsw_rbtree_t *tree );
void         *jsw_rbtlast ( jsw_rbtrav_t *trav, jsw_rbtree_t *tree );
void         *jsw_rbtnext ( jsw_rbtrav_t *trav );
void         *jsw_rbtprev ( jsw_rbtrav_t *trav );

#ifdef __cplusplus
}
#endif

#endif
