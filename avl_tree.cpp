#include "avl_tree.h"

template <typename T>
avl_node<T>::~avl_node() {
    lc = rc = p = NULL;
    h = 0;
}

template <typename T>
bool avl_tree<T>::leq(T *v1, T *v2) {
    return lt(v1,v2) || eq(v1,v2);
};

template <typename T>
bool avl_tree<T>::geq(T *v1, T *v2) {
    return gt(v1,v2) || eq(v1,v2);
};

template <typename T>
uint8_t avl_tree<T>::ht(avl_node<T> *n) {
    return n != NULL ? n->h : 0;
}

template <typename T>
bool avl_tree<T>::update_height(avl_node<T> *n) {
    uint8_t n_h = n->h;
    n->h = std::max(ht(n->lc),ht(n->rc))+1;
    return n->h != n_h;
}

template <typename T>
void avl_tree<T>::rotate_left(avl_node<T> *x) {
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
    update_height(x);
    update_height(y);
}

template <typename T>
void avl_tree<T>::rotate_right(avl_node<T> *y) {
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
    update_height(y);
    update_height(x);
}

template <typename T>
bool avl_tree<T>::balance(avl_node<T> *n) {
    if (ht(n->lc) > ht(n->rc)+1) {
        if (ht(n->lc->lc) < ht(n->lc->rc)) {
            rotate_left(n->lc);
        }
        rotate_right(n);
    } else if (ht(n->rc) > ht(n->lc)+1) {
        if (ht(n->rc->rc) < ht(n->rc->lc)) {
            rotate_right(n->rc);
        }
        rotate_left(n);
    } else {
        return false;
    }
    return true;
}

template <typename T>
void avl_tree<T>::balance_from_to(avl_node<T> *nf, avl_node<T> *nt) {
    while (nf != nt) {
        if (nf == nf->p->rc) {
            nf = nf->p;
            if (!update_height(nf->rc) & !balance(nf->rc)) return;
        } else {
            nf = nf->p;
            if (!update_height(nf->lc) & !balance(nf->lc)) return;
        }
    }
    update_height(nt);
    balance(nt);
}

template <typename T>
avl_node<T>* avl_tree<T>::find(avl_node<T> *n, T *v) {
    do {
        if (gt(v,&n->v)) {
            if (n->rc != NULL) {
                n = n->rc;
            } else {
                return NULL;
            }
        } else if (lt(v,&n->v)) {
            if (n->lc != NULL) {
                n = n->lc;
            } else {
                return NULL;
            }
        } else {
            break;
        }
    } while (true);
    return n;
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum(avl_node<T> *n) {
    while (n->lc != NULL) {
        n = n->lc;
    }
    return n;
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum(avl_node<T> *n) {
    while (n->rc != NULL) {
        n = n->rc;
    }
    return n;
}

template <typename T>
void avl_tree<T>::remove_node(avl_node<T> *n, avl_node<T> *nr) {
    auto *nr_p = nr->p;
    if (nr->lc == NULL) {
        s--;
        if (nr->rc == NULL) {
            if (nr->p != NULL) {
                if (nr == nr->p->lc) {
                    nr->p->lc = NULL;
                } else {
                    nr->p->rc = NULL;
                }
            } else {
                r = NULL;
            }
        } else {
            if (nr->p != NULL) {
                if (nr == nr->p->lc) {
                    nr->p->lc = nr->rc;
                } else {
                    nr->p->rc = nr->rc;
                }
                nr->rc->p = nr->p;
            } else {
                nr->rc->p = NULL;
                r = nr->rc;
            }
        }
        nr->rc = NULL;
        nr->lc = NULL;
        delete nr;
    } else if (nr->rc == NULL) {
        s--;
        if (nr->p != NULL) {
            if (nr == nr->p->lc) {
                nr->p->lc = nr->lc;
            } else {
                nr->p->rc = nr->lc;
            }
            nr->lc->p = nr->p;
        } else {
            nr->lc->p = NULL;
            r = nr->lc;
        }
        nr->rc = NULL;
        nr->lc = NULL;
        delete nr;
    } else {
        auto *min = minimum(nr->rc);
        nr->v = min->v;
        remove_node(nr->rc,min);
    }
    if (nr_p != NULL) {
        balance_from_to(nr_p,n);
    }
}

template <typename T>
avl_tree<T>::avl_tree(
    std::function<bool(T*,T*)> lt,
    std::function<bool(T*,T*)> gt,
    std::function<bool(T*,T*)> eq
) { 
    this->lt = lt;
    this->gt = gt;
    this->eq = eq;
    fst = lst = r = NULL;
    h = s = 0;
}

template <typename T>
avl_tree<T>::~avl_tree() {
    if (!empty()) {
        auto n = fst;
        do {
            if (n->lc != NULL) {
                n = n->lc;
            } else if (n->rc != NULL) {
                n = n->rc;
            } else if (n->p != NULL) {
                if (n == n->p->lc) {
                    n = n->p;
                    delete n->lc;
                    n->lc = NULL;
                } else {
                    n = n->p;
                    delete n->rc;
                    n->rc = NULL;
                }
            } else {
                delete n;
                break;
            }
        } while (true);
        r = NULL;
        h = s = 0;
    }
}

template <typename T>
uint8_t avl_tree<T>::height() {
    return h;
}

template <typename T>
uint64_t avl_tree<T>::size() {
    return s;
}

template <typename T>
bool avl_tree<T>::empty() {
    return s == 0;
}

template <typename T>
avl_node<T>* avl_tree<T>::find(T *v){
    return find(r,v);
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum() {
    return fst;
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum() {
    return lst;
}

template <typename T>
avl_node<T>* avl_tree<T>::insert(T *v) {
    if (empty()) {
        r = new avl_node<T>{*v,NULL,NULL,NULL,1};
        h = s = 1;
        fst = lst = r;
        return r;
    } else {
        auto *n = r;
        do {
            if (gt(v,&n->v)) {
                if (n->rc != NULL) {
                    n = n->rc;
                } else {
                    n->rc = new avl_node<T>{*v,n,NULL,NULL,1};
                    n = n->rc;
                    break;
                }
            } else if (lt(v,&n->v)) {
                if (n->lc != NULL) {
                    n = n->lc;
                } else {
                    n->lc = new avl_node<T>{*v,n,NULL,NULL,1};
                    n = n->lc;
                    break;
                }
            } else {
                return NULL;
            }
        } while (true);
        if (n->p != NULL) {
            balance_from_to(n->p,r);
        }
        h = r->h;
        s++;
        if (lt(v,&fst->v)) {
            fst = n;
        } else if (gt(v,&lst->v)) {
            lst = n;
        }
        return n;
    }
}

template <typename T>
void avl_tree<T>::remove_node(avl_node<T> *n) {
    if (s == 1) {
        delete n;
        r = fst = lst = NULL;
        h = s = 0;
    } else {
        if (n == fst) {
            if (n->rc != NULL) {
                fst = n->rc;
            } else {
                fst = n->p;
            }
        } else if (n == lst) {
            if (n->lc != NULL) {
                lst = n->lc;
            } else {
                lst = n->p;
            }
        }
        remove_node(r,n);
        h = r->h;
    }
}

template <typename T>
bool avl_tree<T>::remove(T *v) {
    if (empty()) return false;
    auto *n = find(v);
    if (n == NULL) return false;
    remove_node(n);
    return true;
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum_geq(T *v) {
    if (empty()) return NULL;
    auto *n = r;
    avl_node<T>* min = NULL;
    do {
        if (lt(&n->v,v)) {
            if (n->rc != NULL) {
                n = n->rc;
            } else {
                break;
            }
        } else if (eq(&n->v,v)) {
            return n;
        } else if (n->lc != NULL) {
            if (lt(&n->lc->v,v)) {
                min = n;
            }
            n = n->lc;
        } else {
            break;
        }
    } while (true);
    if (geq(&n->v,v) && (min == NULL || lt(&n->v,&min->v))) {
        return n;
    }
    return min;
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum_leq(T *v) {
    if (empty()) return NULL;
    auto *n = r;
    avl_node<T>* max = NULL;
    do {
        if (gt(&n->v,v)) {
            if (n->lc != NULL) {
                n = n->lc;
            } else {
                break;
            }
        } else if (eq(&n->v,v)) {
            return n;
        } else if (n->rc != NULL) {
            if (gt(&n->rc->v,v)) {
                max = n;
            }
            n = n->rc;
        } else {
            break;
        }
    } while (true);
    if (leq(&n->v,v) && (max == NULL || gt(&n->v,&max->v))) {
        return n;
    }
    return max;
}

template <typename T>
avl_tree<T>::avl_it::avl_it(avl_tree<T> *t, avl_node<T> *n) {
    this->t = t;
    this->cur = n;
}

template <typename T>
avl_tree<T>::avl_it::~avl_it() {
    t = NULL;
    cur = NULL;
}

template <typename T>
bool avl_tree<T>::avl_it::has_next() {
    return t->lt(&cur->v,&t->maximum()->v);
}

template <typename T>
bool avl_tree<T>::avl_it::has_prev() {
    return t->gt(&cur->v,&t->minimum()->v);
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::current() {
    return cur;
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::next() {
    if (cur->rc != NULL) {
        cur = t->minimum(cur->rc);
    } else {
        while (cur->p != NULL && cur == cur->p->rc) {
            cur = cur->p;
        }
        cur = cur->p;
    }
    return cur;
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::previous() {
    if (cur->lc != NULL) {
        cur = t->maximum(cur->lc);
    } else {
        while (cur->p != NULL && cur == cur->p->lc) {
            cur = cur->p;
        }
        cur = cur->p;
    }
    return cur;
}

template <typename T>
void avl_tree<T>::avl_it::set(avl_node<T> *n) {
    cur = n;
}

template <typename T>
typename avl_tree<T>::avl_it avl_tree<T>::iterator(avl_node<T> *n) {
    return avl_tree<T>::avl_it(this,n);
}