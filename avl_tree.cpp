#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "avl_tree.h"

template <typename T>
avl_node<T>::avl_node(T *v) { // creates a node with value v
    this->v = v;
    p = lc = rc = NULL;
    h = 1;
}

template <typename T>
avl_node<T>::~avl_node() { // recursively deletes all children of the node and the node
    if (lc != NULL) {
        delete lc;
    }
    if (rc != NULL) {
        delete rc;
    }
    lc = rc = NULL;
}

template <typename T>
bool avl_tree<T>::leq(T *v1, T *v2) {return lt(v1,v2) || eq(v1,v2);};

template <typename T>
bool avl_tree<T>::geq(T *v1, T *v2) {return gt(v1,v2) || eq(v1,v2);};

template <typename T>
int avl_tree<T>::ht(avl_node<T> *n) {return n != NULL ? n->h : 0;};

template <typename T>
void avl_tree<T>::rotateLeft(avl_node<T> *x) { // left rotation around x, x must have a right child
    auto *y = x->rc;
    x->rc = y->lc;
    if (y->lc != NULL) {
        y->lc->p = x;
    }
    y->p = x->p;
    if (x->p == NULL) {
        r = y;
    } else if (x == x->p->lc) {
        x->p->lc = y;
    } else {
        x->p->rc = y;
    }
    y->lc = x;
    x->p = y;
    updateHeight(x);
    updateHeight(y);
}

template <typename T>
void avl_tree<T>::rotateRight(avl_node<T> *y) { // right rotation around y, y must have a left child
    auto *x = y->lc;
    y->lc = x->rc;
    if (x->rc != NULL) {
        x->rc->p = y;
    }
    x->p = y->p;
    if (y->p == NULL) {
        r = x;
    } else if (y == y->p->lc) {
        y->p->lc = x;
    } else {
        y->p->rc = x;
    }
    x->rc = y;
    y->p = x;
    updateHeight(y);
    updateHeight(x);
}

template <typename T>
void avl_tree<T>::balance(avl_node<T> *n) { // balances the subtree of node n
    if (ht(n->lc) > ht(n->rc)+1) {
        if (ht(n->lc->lc) < ht(n->lc->rc)) {
            rotateLeft(n->lc);
        }
        rotateRight(n);
    } else if (ht(n->rc) > ht(n->lc)+1) {
        if (ht(n->rc->rc) < ht(n->rc->lc)) {
            rotateRight(n->rc);
        }
        rotateLeft(n);
    }
}

template <typename T>
void avl_tree<T>::updateHeight(avl_node<T> *n) { // updates height of the subtree of node n
    n->h = std::max(ht(n->lc),ht(n->rc))+1;
    if (n == r) {
        h = n->h;
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum(avl_node<T> *n) { // finds the node with the greatest value in the subtree of node n
    if (n == NULL) {
        return NULL;
    } else if (n->rc == NULL) {
        return n;
    } else {
        return maximum(n->rc);
    }
}

// inserts value v in the subtree of node n; returns 0 if it could not be inserted, returns != 0 if it could be inserted
// if upd == true and there is a node with value v_ == v in the subtree of node n, v_ gets deleted and replaced by v and 2 is returned
// if upd == false and there is a node with value v_ == v in the subtree of node n, v stays in the tree and 1 is returned
template <typename T>
int avl_tree<T>::insert(avl_node<T> *n, T *v, bool upd) {
    int ret;
    if (lt(v,n->v)) {
        if (n->lc != NULL) {
            ret = insert(n->lc,v,upd);
        } else {
            n->lc = new avl_node<T>(v);
            s++;
            ret = 1;
        }
    } else if (gt(v,n->v)) {
        if (n->rc != NULL) {
            ret = insert(n->rc,v,upd);
        } else {
            n->rc = new avl_node<T>(v);
            s++;
            ret = 1;
        }
    } else {
        if (upd) {
            delete n->v;
            n->v = v;
            return 2;
        } else {
            return 0;
        }
    }
    if (ret) {
        updateHeight(n);
        balance(n);
    }
    return ret;
}

template <typename T>
bool avl_tree<T>::remove(avl_node<T> *n, T *v) { // remove value v in the subtree of node n
    bool ret = true;
    if (n == NULL) {
        return false;
    } else if (lt(v,n->v)) {
        ret = remove(n->lc,v);
    } else if (gt(v,n->v)) {
        ret = remove(n->rc,v);
    } else if (n->lc == NULL) {
        s--;
        if (n->rc == NULL) {
            if (n->p != NULL) {
                if (n == n->p->lc) {
                    n->p->lc = NULL;
                } else {
                    n->p->rc = NULL;
                }
            } else {
                r = NULL;
            }
        } else {
            if (n->p != NULL) {
                if (n == n->p->lc) {
                    n->p->lc = n->rc;
                } else {
                    n->p->rc = n->rc;
                }
                n->rc->p = n->p;
            } else {
                n->rc->p = NULL;
                r = n->rc;
            }
            n->rc = NULL;
        }
        delete n;
        n = NULL;
    } else if (n->rc == NULL) {
        s--;
        if (n->p != NULL) {
            if (n == n->p->lc) {
                n->p->lc = n->lc;
            } else {
                n->p->rc = n->lc;
            }
            n->lc->p = n->p;
        } else {
            n->lc->p = NULL;
            r = n->lc;
        }
        n->lc = NULL;
        delete n;
        n = NULL;
    } else {
        auto *m = maximum(n->lc);
        n->v = m->v;
        ret = remove(n->lc,m->v);
    }
    if (n != NULL) {
        updateHeight(n);
        balance(n);
    }
    return ret;
}

template <typename T>
T* avl_tree<T>::removeMin(avl_node<T> *n) { // finds the node with the smallest value in the subtree of node n, removes it and returns it's value
    T *min;
    if (n == NULL) {
        return NULL;
    } else if (n->lc != NULL) {
        min = removeMin(n->lc);
        updateHeight(n);
        balance(n);
    } else {
        min = n->v;
        remove(n,n->v);
    }
    return min;
}

template <typename T>
T* avl_tree<T>::minElemGreaterOrEqual(avl_node<T> *n, T *v) { // returns the node with the minimum value >= v in the subtree of node n
    if (n == NULL) {
        return NULL;
    } else if (lt(n->v,v)) {
        return minElemGreaterOrEqual(n->rc,v);
    } else {
        T *min = minElemGreaterOrEqual(n->lc,v);
        if (min != NULL) {
            return min;
        } else {
            return n->v;
        }
    }
}

template <typename T>
T* avl_tree<T>::maxElemLessOrEqual(avl_node<T> *n, T *v) { // returns the node with the maximum value <= v in the subtree of node n
    if (n == NULL) {
        return NULL;
    } else if (gt(n->v,v)) {
        return maxElemLessOrEqual(n->lc,v);
    } else {
        T *max = maxElemLessOrEqual(n->rc,v);
        if (max != NULL) {
            return max;
        } else {
            return n->v;
        }
    }
}

template <typename T>
avl_tree<T>::avl_tree( // constructs empty tree
    std::function<bool(T*,T*)> lt,
    std::function<bool(T*,T*)> gt,
    std::function<bool(T*,T*)> eq
) { 
    this->lt = lt;
    this->gt = gt;
    this->eq = eq;
    r = NULL;
    h = 0;
    s = 0;
}

template <typename T>
avl_tree<T>::~avl_tree() { // recursively deletes all nodes in the tree and the tree
    if (r != NULL) {
        delete r;
        r = NULL;
    }
}

template <typename T>
int avl_tree<T>::height() { // returns the height of the tree
    return h;
}

template <typename T>
int avl_tree<T>::size() { // returns the size of the tree
    return s;
}

template <typename T>
bool avl_tree<T>::isEmpty() { // returns if the tree is empty
    return s == 0;
}

// inserts value v into the tree; returns 0 if it could not be inserted, returns != 0 if it could be inserted
// if upd == true and there is a node with value v_ == v in the tree, v_ gets deleted and replaced by v and 2 is returned
// if upd == false and there is a node with value v_ == v in the tree, v stays in the tree and 1 is returned
template <typename T>
int avl_tree<T>::insert(T *v, bool upd) {
    if (r != NULL) {
        bool ret = insert(r,v,upd);
        h = r->h;
        return ret;
    } else {
        r = new avl_node<T>(v);
        h = 1;
        s++;
        return 1;
    }
}

template <typename T>
bool avl_tree<T>::remove(T *v) { // removes value v, returns if it could be removed
    if (r != NULL) {
        bool ret = remove(r,v);
        if (r != NULL) {
            h = r->h;
        } else {
            h = 0;
        }
        return ret;
    }
    return false;
}

template <typename T>
T* avl_tree<T>::removeMin() { // finds the node with the smallest value in the avl tree, removes it and returns it's value
    if (r != NULL) {
        return removeMin(r);
    }
    return NULL;
}

template <typename T>
T* avl_tree<T>::minElemGreaterOrEqual(T *v) { // returns the node with the minimum value >= v in the avl tree
    if (r != NULL) {
        return minElemGreaterOrEqual(r,v);
    }
    return NULL;
}

template <typename T>
T* avl_tree<T>::maxElemLessOrEqual(T *v) { // returns the node with the maximum value <= v in the avl tree
    if (r != NULL) {
        return maxElemLessOrEqual(r,v);
    }
    return NULL;
}