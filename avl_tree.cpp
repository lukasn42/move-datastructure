#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

template <typename T>
struct avl_node {
    T *v; // value
    struct avl_node<T> *p = NULL; // parent
    struct avl_node<T> *lc,*rc = NULL; // left child, right child
    int h = 1; // height
};

template <typename T>
class avl_tree {
    private:
    avl_node<T> *r; // root
    int h; // height
    int s; // size

    std::function<bool(T*,T*)> lt;
    std::function<bool(T*,T*)> gt;
    std::function<bool(T*,T*)> eq;
    bool leq(T *v1, T *v2) {return lt(v1,v2) || eq(v1,v2);};
    bool geq(T *v1, T *v2) {return gt(v1,v2) || eq(v1,v2);};
    int ht(avl_node<T> *n) {return n != NULL ? n->h : 0;};

    void rotateLeft(avl_node<T> *x) { // left rotation around x, x must have a right child
        avl_node<T> *y = x->rc;
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

    void rotateRight(avl_node<T> *y) { // right rotation around y, y must have a left child
        avl_node<T> *x = y->lc;
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

    void balance(avl_node<T> *n) { // balances the subtree of node n
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

    void updateHeight(avl_node<T> *n) { // updates height of the subtree of node n
        n->h = std::max(ht(n->lc),ht(n->rc))+1;
        if (n == r) {
            h = n->h;
        }
    }

    bool insert(avl_node<T> *n, T *v) { // inserts value v in the subtree of node n, returns if it could be inserted
        bool ret;
        if (lt(v,n->v)) {
            if (n->lc != NULL) {
                ret = insert(n->lc,v);
            } else {
                n->lc = new avl_node<T> {.v = v,.p = n};
                s++;
                ret = true;
            }
        } else if (gt(v,n->v)) {
            if (n->rc != NULL) {
                ret = insert(n->rc,v);
            } else {
                n->rc = new avl_node<T> {.v = v,.p = n};
                s++;
                ret = true;
            }
        } else {
            return false;
        }
        if (ret) {
            updateHeight(n);
            balance(n);
        }
        return ret;
    }

    bool remove(avl_node<T> *n, T *v) { // remove value v in the subtree of node n
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
                n = NULL;
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
                    n = NULL;
                }
            }
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
                n = NULL;
            }
        } else {
            avl_node<T> *m = maximum(n->lc);
            n->v = m->v;
            ret = remove(n->lc,m->v);
        }
        if (n != NULL) {
            updateHeight(n);
            balance(n);
        }
        return ret;
    }

    avl_node<T>* maximum(avl_node<T> *n) { // finds the node with the greatest value in the subtree of node n
        if (n == NULL) {
            return NULL;
        } else if (n->rc == NULL) {
            return n;
        } else {
            return maximum(n->rc);
        }
    }

    T* removeMin(avl_node<T> *n) { // finds the node with the smallest value in the subtree of node n, removes it and returns it's value
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

    avl_node<T>* minElemGreaterOrEqual(avl_node<T> *n, T *v) { // returns the node with the minimum value >= v in the subtree of node n
        if (n == NULL) {
            return NULL;
        } else if (lt(n->v,v)) {
            return minElemGreaterOrEqual(n->rc,v);
        } else {
            avl_node<T>* vLc = minElemGreaterOrEqual(n->lc,v);
            if (vLc != NULL) {
                return vLc;
            } else {
                return n;
            }
        }
    }

    avl_node<T>* maxElemLessOrEqual(avl_node<T> *n, T *v) { // returns the node with the maximum value <= v in the subtree of node n
        if (n == NULL) {
            return NULL;
        } else if (gt(n->v,v)) {
            return maxElemLessOrEqual(n->lc,v);
        } else {
            avl_node<T>* vRc = maxElemLessOrEqual(n->rc,v);
            if (vRc != NULL) {
                return vRc;
            } else {
                return n;
            }
        }
    }

    public:
    avl_tree<T>( // constructs empty tree
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

    int height() { // returns the height of the tree
        return h;
    }

    int size() { // returns the size of the tree
        return s;
    }

    bool isEmpty() {
        return s == 0;
    }

    bool insert(T *v) { // inserts value v, returns if it could be inserted
        if (r != NULL) {
            bool ret = insert(r,v);
            h = r->h;
            return ret;
        } else {
            r = new avl_node<T> {.v = v};
            h = 1;
            s++;
            return true;
        }
    }

    bool remove(T *v) { // removes value v, returns if it could be removed
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

    avl_node<T>* maximum() { // returns value of greatest element, else returns NULL
        if (r != NULL) {
            return maximum(r);
        }
        return NULL;
    }

    T* removeMin() { // finds the node with the smallest value in the avl tree, removes it and returns it's value
        if (r != NULL) {
            return removeMin(r);
        }
        return NULL;
    }

    avl_node<T>* minElemGreaterOrEqual(T *v) {
        if (r != NULL) {
            return minElemGreaterOrEqual(r,v);
        }
        return NULL;
    }

    avl_node<T>* maxElemLessOrEqual(T *v) {
        if (r != NULL) {
            return maxElemLessOrEqual(r,v);
        }
        return NULL;
    }
};