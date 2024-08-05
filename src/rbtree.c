#include "rbtree.h"

#include <stdio.h>
#include <stdlib.h>

#define IS_BLACK(x) ((x == NULL) || (x->color == RBTREE_BLACK))

rbtree *new_rbtree(void)
{
    rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
    // p->root = NULL;
    // sentinel 안 쓸래
    // p->nil = (node_t *)calloc(1, sizeof(node_t));
    // p->nil->color = RBTREE_BLACK;
    // p->nil->left = p->nil;
    // p->nil->right = p->nil;
    // p->nil->parent = p->nil;
    // p->root = p->nil;

    return p;
}

void _delete_node(node_t *node) {
    if (node != NULL) {
        _delete_node(node->left);
        _delete_node(node->right);
    }
    free(node);
}

void delete_rbtree(rbtree *t)
{
    _delete_node(t->root);
    free(t);
}

// rotate t↷t->parent
// t->parent->parent should be present
// returns old parent(new child)
node_t *_rotate_tree(rbtree *t, node_t *N) {
    node_t *P = N->parent;

    if (N == P->right) {
        P->right = N->left;
        if (N->left != NULL) {
            N->left->parent = P;
        }

        N->parent = P->parent;
        if (P->parent != NULL) {
            if (P->parent->left == P) P->parent->left = N;
            else P->parent->right = N;
        }

        P->parent = N;
        N->left = P;
    }
    else {
        P->left = N->right;
        if (N->right != NULL) {
            N->right->parent = P;
        }

        N->parent = P->parent;
        if (P->parent != NULL) {
            if (P->parent->right == P) P->parent->right = N;
            else P->parent->left = N;
        }

        P->parent = N;
        N->right = P;
    }
    if (t->root == P) t->root = N;

    return P;
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{
    node_t **ptrCur = &(t->root);
    node_t *N, *P, *U, *G; // N for now, P for parent, U for uncle node and G for grandparent node
    P = NULL;

    // binary search tree insertion
    // find insert point
    while (*ptrCur != NULL) {
        P = *ptrCur;
        ptrCur = (key < P->key) ? &(P->left) : &(P->right);
    }
    // allocate new node & place
    N = (node_t *)calloc(1, sizeof(node_t));
    N->parent = P;
    N->key = key;
    N->color = RBTREE_RED;
    *ptrCur = N;

    // rebalancing loop
    P = N->parent;
    while (!IS_BLACK(P)) {
        // P is red and not NULL
        if (P != t->root) {
            G = P->parent;
            U = (P == G->left) ? G->right : G->left;
        }
        else {
            P->color = RBTREE_BLACK;
            break;
        }

        if (IS_BLACK(U)) {
            // if N is inner grandchild
            if ((P == G->left && N == P->right)
            || (P == G->right && N == P->left)) {
                P = N;
                N = _rotate_tree(t, N);
            }
            _rotate_tree(t, P);
            P->color = RBTREE_BLACK;
            G->color = RBTREE_RED;
            break;
        }
        // if uncle node is red:
        else {
            P->color = RBTREE_BLACK;
            U->color = RBTREE_BLACK;
            G->color = RBTREE_RED;
            N = G;
        }
        P = N->parent;
    }

    return N;
}

node_t *rbtree_find(const rbtree *t, const key_t key)
{
    node_t *cur = t->root;
    while (cur != NULL && cur->key != key) {
        cur = key < cur->key? cur->left: cur->right;
    }
    return cur;
}

node_t *rbtree_min(const rbtree *t)
{
    node_t *cur = t->root;
    while (cur->left != NULL) cur = cur->left;
    return cur;
}

node_t *rbtree_max(const rbtree *t)
{
    node_t *cur = t->root;
    while (cur->right != NULL) cur = cur->right;
    return cur;
}

// predecessor as a fallback return value
node_t *_find_successor(node_t *t) {
    if (t->right == NULL) return t->left;

    node_t *cur;

    cur = t->right;
    while (cur->left != NULL) cur = cur->left;

    return cur;
}

int rbtree_erase(rbtree *t, node_t *p)
{
    node_t *N, *P, *S, *C = NULL, *D = NULL;
    N = p;

    while (N->left != NULL || N->right != NULL) {
        p = _find_successor(N);
        N->key = p->key;
        N = p;
    }
    // now N has no child

    if (N->color == RBTREE_BLACK) {
    // no need if N is red
        while (N != t->root) {
            // N for now, P for parent, S for sibling, C for close nephew, D for distant nephew node
            P = N->parent;
            // if S is red
            if (P->left == N) {
                S = P->right;
                if (S != NULL) {
                    C = S->left;
                    D = S->right;
                }
            }
            else {
                S = P->left;
                if (S != NULL) {
                    C = S->right;
                    D = S->left;
                }
            }
            if (!IS_BLACK(S)) {
                _rotate_tree(t, S);
                S->color = RBTREE_BLACK;
                P->color = RBTREE_RED;
            }
            else {
                // if S is black, D is red
                if (!IS_BLACK(D)) {
                    _rotate_tree(t, S);
                    S->color = P->color;
                    P->color = RBTREE_BLACK;
                    D->color = RBTREE_BLACK;
                    break;
                }
                // if S, D are black, C is red
                else if (!IS_BLACK(C)) {
                    _rotate_tree(t, C);
                    C->color = RBTREE_BLACK;
                    S->color = RBTREE_RED;
                }
                // if S, C, D are black, P is red
                else if (!IS_BLACK(P)) {
                    S->color = RBTREE_RED;
                    P->color = RBTREE_BLACK;
                    break;
                }
                // if S, P, C, D are black:
                else {
                    S->color = RBTREE_RED;
                    N = P;
                }
            }
        }
    }

    // delete p
    if (p != t->root) {
        if (p->parent->left == p) p->parent->left = NULL;
        else p->parent->right = NULL;
    }
    else {
        t->root = NULL;
    }
    free(p);

    return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
    int sbp = 0; // stack pointer. use arr as stack
    int i = 0;
    node_t *stack[(int)n];
    node_t *cur = t->root;

    do {
        if (cur != NULL) {
            stack[sbp++] = cur;
            cur = cur->left;
        }
        else {
            cur = stack[--sbp];
            arr[i++] = cur->key;
            cur = cur->right;
        }
    } while (sbp > 0 || cur != NULL);

    return 0;
}
