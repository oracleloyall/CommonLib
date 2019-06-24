/*
  Date 2016 11 1
*/
#include "jsw_rbtree.h"
#include"balloc.h"
#ifndef NULL
#define NULL ((void *)0)
#endif
#ifdef __cplusplus
#include <cstdlib>

using std::malloc;
using std::free;
using std::size_t;
#else
#include <stdlib.h>
#endif

#ifndef HEIGHT_LIMIT
#define HEIGHT_LIMIT 64
#endif

typedef struct jsw_rbnode {
  int                red;     /* (1=red, 0=black) */
  void              *data;
  struct jsw_rbnode *link[2];
} jsw_rbnode_t;

struct jsw_rbtree {
  jsw_rbnode_t *root;
  cmp_f         cmp;
  dup_f         dup;
  rel_f         rel;
  size_t        size;
};

struct jsw_rbtrav {
  jsw_rbtree_t *tree;
  jsw_rbnode_t *it;
  jsw_rbnode_t *path[HEIGHT_LIMIT];
  size_t        top;
};


static int is_red ( jsw_rbnode_t *root )
{
  return root != NULL && root->red == 1;
}

static jsw_rbnode_t *jsw_single ( jsw_rbnode_t *root, int dir )
{
  jsw_rbnode_t *save = root->link[!dir];

  root->link[!dir] = save->link[dir];
  save->link[dir] = root;

  root->red = 1;
  save->red = 0;

  return save;
}


static jsw_rbnode_t *jsw_double ( jsw_rbnode_t *root, int dir )
{
  root->link[!dir] = jsw_single ( root->link[!dir], !dir );

  return jsw_single ( root, dir );
}

static jsw_rbnode_t *new_node ( jsw_rbtree_t *tree, void *data )
{
  jsw_rbnode_t *rn = (jsw_rbnode_t *)balloc( sizeof *rn );

  if ( rn == NULL )
    return NULL;

  rn->red = 1;
  rn->data = tree->dup ( data );
  rn->link[0] = rn->link[1] = NULL;

  return rn;
}

jsw_rbtree_t *jsw_rbnew ( cmp_f cmp, dup_f dup, rel_f rel )
{
  jsw_rbtree_t *rt = (jsw_rbtree_t *)balloc ( sizeof *rt );

  if ( rt == NULL )
    return NULL;

  rt->root = NULL;
  rt->cmp = cmp;
  rt->dup = dup;
  rt->rel = rel;
  rt->size = 0;

  return rt;
}

void jsw_rbdelete ( jsw_rbtree_t *tree )
{
  jsw_rbnode_t *it = tree->root;
  jsw_rbnode_t *save;


  while ( it != NULL ) {
    if ( it->link[0] == NULL ) {

      save = it->link[1];
      tree->rel ( it->data );
      bfree ( it );
    }
    else {

      save = it->link[0];
      it->link[0] = save->link[1];
      save->link[1] = it;
    }

    it = save;
  }

  bfree ( tree );
}

void *jsw_rbfind ( jsw_rbtree_t *tree, void *data )
{

  jsw_rbnode_t *it = tree->root;

  while ( it != NULL ) {
    int cmp = tree->cmp ( it->data, data );

    if ( cmp == 0 )
      break;

    it = it->link[cmp < 0];
  }

  return it == NULL ? NULL : it->data;
}
//增加接口修改结构体的值

int jsw_rbinsert ( jsw_rbtree_t *tree, void *data )
{
  if ( tree->root == NULL ) {

    tree->root = new_node ( tree, data );

    if ( tree->root == NULL )
      return 0;
  }
  else {
    jsw_rbnode_t head = {0};
    jsw_rbnode_t *g, *t;
    jsw_rbnode_t *p, *q;
    int dir = 0, last = 0;

    t = &head;
    g = p = NULL;
    q = t->link[1] = tree->root;

    for ( ; ; ) {
      if ( q == NULL ) {
        p->link[dir] = q = new_node ( tree, data );

        if ( q == NULL )
          return 0;
      }
      else if ( is_red ( q->link[0] ) && is_red ( q->link[1] ) ) {

        q->red = 1;
        q->link[0]->red = 0;
        q->link[1]->red = 0;
      }

      if ( is_red ( q ) && is_red ( p ) ) {

        int dir2 = t->link[1] == g;

        if ( q == p->link[last] )
          t->link[dir2] = jsw_single ( g, !last );
        else
          t->link[dir2] = jsw_double ( g, !last );
      }

      if ( tree->cmp ( q->data, data ) == 0 )
        break;

      last = dir;
      dir = tree->cmp ( q->data, data ) < 0;


      if ( g != NULL )
        t = g;

      g = p, p = q;
      q = q->link[dir];
    }


    tree->root = head.link[1];
  }

  tree->root->red = 0;
  ++tree->size;

  return 1;
}

int jsw_rberase ( jsw_rbtree_t *tree, void *data )
{
  if ( tree->root != NULL ) {
    jsw_rbnode_t head = {0};
    jsw_rbnode_t *q, *p, *g;
    jsw_rbnode_t *f = NULL;
    int dir = 1;


    q = &head;
    g = p = NULL;
    q->link[1] = tree->root;


    while ( q->link[dir] != NULL ) {
      int last = dir;


      g = p, p = q;
      q = q->link[dir];
      dir = tree->cmp ( q->data, data ) < 0;


      if ( tree->cmp ( q->data, data ) == 0 )
        f = q;


      if ( !is_red ( q ) && !is_red ( q->link[dir] ) ) {
        if ( is_red ( q->link[!dir] ) )
          p = p->link[last] = jsw_single ( q, dir );
        else if ( !is_red ( q->link[!dir] ) ) {
          jsw_rbnode_t *s = p->link[!last];

          if ( s != NULL ) {
            if ( !is_red ( s->link[!last] ) && !is_red ( s->link[last] ) ) {

              p->red = 0;
              s->red = 1;
              q->red = 1;
            }
            else {
              int dir2 = g->link[1] == p;

              if ( is_red ( s->link[last] ) )
                g->link[dir2] = jsw_double ( p, last );
              else if ( is_red ( s->link[!last] ) )
                g->link[dir2] = jsw_single ( p, last );


              q->red = g->link[dir2]->red = 1;
              g->link[dir2]->link[0]->red = 0;
              g->link[dir2]->link[1]->red = 0;
            }
          }
        }
      }
    }


    if ( f != NULL ) {
      tree->rel ( f->data );
      f->data = q->data;
      p->link[p->link[1] == q] =
        q->link[q->link[0] == NULL];
      bfree ( q );
    }


    tree->root = head.link[1];


    if ( tree->root != NULL )
      tree->root->red = 0;

    --tree->size;
  }

  return 1;
}


size_t jsw_rbsize ( jsw_rbtree_t *tree )
{
  return tree->size;
}

jsw_rbtrav_t *jsw_rbtnew ( void )
{
  return (jsw_rbtrav_t*)balloc( sizeof ( jsw_rbtrav_t ) );
}

void jsw_rbtdelete ( jsw_rbtrav_t *trav )
{
  bfree( trav );
}

static void *start ( jsw_rbtrav_t *trav, jsw_rbtree_t *tree, int dir )
{
  trav->tree = tree;
  trav->it = tree->root;
  trav->top = 0;


  if ( trav->it != NULL ) {
    while ( trav->it->link[dir] != NULL ) {
      trav->path[trav->top++] = trav->it;
      trav->it = trav->it->link[dir];
    }
  }

  return trav->it == NULL ? NULL : trav->it->data;
}

static void *move ( jsw_rbtrav_t *trav, int dir )
{
  if ( trav->it->link[dir] != NULL ) {

    trav->path[trav->top++] = trav->it;
    trav->it = trav->it->link[dir];

    while ( trav->it->link[!dir] != NULL ) {
      trav->path[trav->top++] = trav->it;
      trav->it = trav->it->link[!dir];
    }
  }
  else {

    jsw_rbnode_t *last;

    do {
      if ( trav->top == 0 ) {
        trav->it = NULL;
        break;
      }

      last = trav->it;
      trav->it = trav->path[--trav->top];
    } while ( last == trav->it->link[dir] );
  }

  return trav->it == NULL ? NULL : trav->it->data;
}

void *jsw_rbtfirst ( jsw_rbtrav_t *trav, jsw_rbtree_t *tree )
{
  return start ( trav, tree, 0 );
}

void *jsw_rbtlast ( jsw_rbtrav_t *trav, jsw_rbtree_t *tree )
{
  return start ( trav, tree, 1 );
}

void *jsw_rbtnext ( jsw_rbtrav_t *trav )
{
  return move ( trav, 1 );
}

void *jsw_rbtprev ( jsw_rbtrav_t *trav )
{
  return move ( trav, 0 );
}
