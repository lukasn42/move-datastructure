#include <cstdint>
#include <algorithm>

template <typename T>
struct avl_node {
    T v; // value
    struct avl_node<T> *p = NULL; // parent
    struct avl_node<T> *lc,*rc = NULL; // left child, right child
    int h; // height
};

template <typename T>
class avl_tree {
    private:
    avl_node<T> *r; // root
    int h; // height
    int s; // size

    int ht(avl_node<T> *n) { // returns height of subtree of node n
        return n != NULL ? n->h : 0;
    }

    int rotateLeft(avl_node<T> *x) { // left rotation around x, x must have a right child
        avl_node<T> *y = x->rc;
        x->rc = y->lc;
        if (y->lc != NULL) {
            y->lc->p = x;
        }
        y->p = x->p;
        if (x->p == NULL) {
            r = y;
            r->h = y->h;
        } else if (x == x->p->lc) {
            x->p->lc = y;
        } else {
            x->p->rc = y;
        }
        y->lc = x;
        x->p = y;
        updateHeight(x);
        updateHeight(y);
        return 0;
    }

    int rotateRight(avl_node<T> *y) { // right rotation around y, y must have a left child
        avl_node<T> *x = y->lc;
        y->lc = x->rc;
        x->p = y->p;
        if (y->p == NULL) {
            r = x;
            r->h = x->h;
        } else if (y == y->p->lc) {
            y->p->lc = x;
        } else {
            y->p->rc = x;
        }
        x->rc = y;
        y->p = x;
        updateHeight(y);
        updateHeight(x);
        return 0;
    }

    bool balance(avl_node<T> *n) { // balance subtree of node n, returns if it was not balanced
        if (ht(n->lc) > ht(n->rc)+1) {
            if (ht(n->lc->lc) < ht(n->lc->rc)) {
                rotateLeft(n->lc);
            }
            rotateRight(n);
            return true;
        } else if (ht(n->rc) > ht(n->lc)+1) {
            if (ht(n->rc->rc) < ht(n->rc->lc)) {
                rotateRight(n->rc);
            }
            rotateLeft(n);
            return true;
        }
        return false;
    }

    bool insert(T v, avl_node<T> *n) { // inserts value v in the subtree of node n, returns if it could be inserted
        bool ret;
        if (v < n->v) {
            if (n->lc == NULL) {
                n->lc = new avl_node<T> {.v = v,.p = n,.h = 1};
                s++;
                ret = true;
            } else {
                ret = insert(v,n->lc);
            }
        } else if (v > n->v) {
            if (n->rc == NULL) {
                n->rc = new avl_node<T> {.v = v,.p = n,.h = 1};
                s++;
                ret = true;
            } else {
                ret = insert(v,n->rc);
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

    bool updateHeight(avl_node<T> *n) { // updates height of subree of node n, returns if it changed
        int h_ = n->h;
        n->h = std::max(ht(n->lc),ht(n->rc))+1;
        return h != h_;
    }

    bool remove(T v, avl_node<T> *n) { // remove value v in the subtree of node n
        bool ret = true;
        if (v < n->v) {
            ret = remove(v,n->lc);
        } else if (v > n->v) {
            ret = remove(v,n->rc);
        } else if (n == NULL) {
            return false;
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
            ret = remove(m->v,n->lc);
        }
        if (n != NULL) {
            updateHeight(n);
            balance(n);
        }
        return ret;
    }

    avl_node<T>* minimum(avl_node<T> *n) { // finds the node with the smallest value in the subtree of node n
        if (n == NULL) {
            return NULL;
        } else if (n->lc == NULL) {
            return n;
        } else {
            return minimum(n->lc);
        }
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

    avl_node<T>* find(avl_node<T> *n, T val) {
        if (n == NULL) {
            return NULL;
        } else if (n->v < val) {
            return find(n->rc,val);
        } else if (n->v > val) {
            return find(n->lc,val);
        } else {
            return n;
        }
    }

    avl_node<T>* minElemNotLessThan(avl_node<T> *n, T val) {
        if (n == NULL) {
            return NULL;
        } else if (n->v < val) {
            return minElemNotLessThan(n->rc,val);
        } else if (n->lc != NULL && n->lc->v >= val) {
            return minElemNotLessThan(n->lc,val);
        } else {
            return n;
        }
    }

    avl_node<T>* minElemGreaterThan(avl_node<T> *n, T val) {
        if (n == NULL) {
            return NULL;
        } else if (n->v <= val) {
            return minElemGreaterThan(n->rc,val);
        } else if (n->lc != NULL && n->lc->v > val) {
            return minElemGreaterThan(n->lc,val);
        } else {
            return n;
        }
    }

    avl_node<T>* maxElemLessThan(avl_node<T> *n, T val) {
        if (n == NULL) {
            return NULL;
        } else if (n->v >= val) {
            return maxElemLessThan(n->lc,val);
        } else if (n->lc != NULL && n->rc->v < val) {
            return maxElemLessThan(n->rc,val);
        } else {
            return n;
        }
    }

    avl_node<T>* maxElemNotGreaterThan(avl_node<T> *n, T val) {
        if (n == NULL) {
            return NULL;
        } else if (n->v > val) {
            return maxElemNotGreaterThan(n->lc,val);
        } else if (n->lc != NULL && n->rc->v <= val) {
            return maxElemNotGreaterThan(n->rc,val);
        } else {
            return n;
        }
    }

    bool contains(T v, avl_node<T> *n) { // checks if the subtree of node n contains the value v
        if (n == NULL) {
            return false;
        } else if (v < n->v) {
            return contains(v,n->lc);
        } else if (v > n->v) {
            return contains(v,n->rc);
        } else {
            return true;
        }
    }

    public:
    avl_tree<T>() { // constructs empty tree
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

    bool contains(T v) { // checks if the tree contains an element with value v
        if (r != NULL) {
            return contains(v,r);
        } else {
            return false;
        }
        
    }

    bool insert(T v) { // inserts value v, returns if it could be inserted
        if (r == NULL) {
            r = new avl_node<T> {.v = v,.h = 1};
            h = 1;
            s++;
            return true;
        } else {
            return insert(v,r);
        }
    }

    bool remove(T v) { // removes value v, returns if it could be removed
        if (r != NULL) {
            return remove(v,r);
        }
        return false;
    }

    avl_node<T>* minimum() { // returns value of smallest element, else returns NULL
        if (r != NULL) {
            return minimum(r);
        }
        return NULL;
    }

    avl_node<T>* maximum() { // returns value of greatest element, else returns NULL
        if (r != NULL) {
            return maximum(r);
        }
        return NULL;
    }

    avl_node<T>* find(T val) {
        if (r == NULL) {
            return NULL;
        }
        return find(r,val);
    }

    avl_node<T>* minElemGreaterThan(T val) {
        if (r == NULL) {
            return NULL;
        }
        return minElemGreaterThan(r,val);
    }

    avl_node<T>* minElemNotLessThan(T val) {
        if (r == NULL) {
            return NULL;
        }
        return minElemNotLessThan(r,val);
    }

    avl_node<T>* maxElemLessThan(T val) {
        if (r == NULL) {
            return NULL;
        }
        return maxElemLessThan(r,val);
    }

    avl_node<T>* maxElemNotGreaterThan(T val) {
        if (r == NULL) {
            return NULL;
        }
        return maxElemNotGreaterThan(r,val);
    }

    avl_node<T>* next(avl_node<T> *n) {
        if (n != NULL) {
            if (n->rc != NULL) {
                return minimum(n->rc);
            } else {
                while (n->p != NULL && n == n->p->rc) {
                    n = n->p;
                }
                if (n->p != NULL) {
                    return n->p;
                }
            }
        }
        return NULL;
    }

    avl_node<T>* prev(avl_node<T> *n) {
        if (n != NULL) {
            if (n->lc != NULL) {
                return maximum(n->lc);
            } else {
                while (n->p != NULL && n == n->p->lc) {
                    n = n->p;
                }
                if (n->p != NULL) {
                    return n->p;
                }
            }
        }
        return NULL;
    }
};