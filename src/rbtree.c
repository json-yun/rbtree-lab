#include "rbtree.h"

#include <stdlib.h>

#define IS_BLACK(x) (((x) == NULL) || ((x)->color == RBTREE_BLACK))

rbtree *new_rbtree(void)
{
    rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));

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

// rotate N↷N->parent
// returns old parent(new child)
node_t *_rotate_tree(rbtree *t, node_t *N) {
    node_t *P = N->parent;

    // when N is right child
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
    // when N is left child
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
    // N for now, P for parent, U for uncle node and G for grandparent node
    node_t *N, *P, *U, *G;
    P = NULL;

    // binary search tree insertion
    // find insert position
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
    // no need if P is black or NULL
    for (; !IS_BLACK(P); P = N->parent) {
        // I4: if P is root and red
        if (P == t->root) {
            P->color = RBTREE_BLACK;
            break;
        }

        G = P->parent;
        U = (P == G->left) ? G->right : G->left;

        // if U is black
        if (IS_BLACK(U)) {
            // I5: if N is inner grandchild
            if ((P == G->left && N == P->right)
            || (P == G->right && N == P->left)) {
                P = N;
                N = _rotate_tree(t, N);
            }
            // I6: if N is outer grandchild
            _rotate_tree(t, P);
            P->color = RBTREE_BLACK;
            G->color = RBTREE_RED;
            break;
        }
        // I2: if U is red:
        else {
            P->color = RBTREE_BLACK;
            U->color = RBTREE_BLACK;
            G->color = RBTREE_RED;
            N = G;
        }
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

// returns predecessor if successor absent
node_t *_find_successor(node_t *t) {
    node_t *cur = t->right;
    // fallback
    if (cur == NULL) return t->left;

    while (cur->left != NULL) cur = cur->left;

    return cur;
}

int rbtree_erase(rbtree *t, node_t *p)
{
    // N for now, P for parent, S for sibling,
    // C for close nephew and D for distant nephew node
    node_t *N, *P, *S, *C = NULL, *D = NULL;
    N = p;

    // if N is not leaf node
    while (N->left != NULL || N->right != NULL) {
        p = _find_successor(N);
        N->key = p->key;
        N = p;
    }
    // now N has no child

    // no task needed if N is red
    if (N->color == RBTREE_BLACK) {
        for (P = N->parent; N != t->root; P = N->parent) {
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
            // D3: if S is red
            if (!IS_BLACK(S)) {
                _rotate_tree(t, S);
                S->color = RBTREE_BLACK;
                P->color = RBTREE_RED;
            }
            // D6: if S is black, D is red
            else if (!IS_BLACK(D)) {
                _rotate_tree(t, S);
                S->color = P->color;
                P->color = RBTREE_BLACK;
                D->color = RBTREE_BLACK;
                break;
            }
            // D5: if S, D are black, C is red
            else if (!IS_BLACK(C)) {
                _rotate_tree(t, C);
                C->color = RBTREE_BLACK;
                S->color = RBTREE_RED;
            }
            // D4: if S, C, D are black, P is red
            else if (!IS_BLACK(P)) {
                S->color = RBTREE_RED;
                P->color = RBTREE_BLACK;
                break;
            }
            // D2: if S, P, C, D are black:
            else {
                S->color = RBTREE_RED;
                N = P;
            }
        }
    }

    // free p
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
    int sbp = 0; // stack pointer.
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