#include <vector>
#include <omp.h>
#include <functional>
#include <cstddef>
#include <cstdint>

#include <avl_tree.hpp>

template <typename T>
avl_node<T>::avl_node() {
    lc = rc = p = NULL;
    h = 0;
}

template <typename T>
avl_node<T>::avl_node(T v) {
    this->v = v;
    lc = rc = p = NULL;
    h = 0;
}

template <typename T>
avl_node<T>::~avl_node() {
    lc = rc = p = NULL;
    h = 0;
}

template <typename T>
avl_node<T>* avl_node<T>::nxt() {
    avl_node<T> *cur = this;
    if (cur->rc != NULL) {
        cur = cur->rc;
        while (cur->lc != NULL) {
            cur = cur->lc;
        }
    } else {
        while (cur->p != NULL && cur == cur->p->rc) {
            cur = cur->p;
        }
        cur = cur->p;
    }
    return cur;
}

template <typename T>
avl_node<T>* avl_node<T>::prv() {
    avl_node<T> *cur = this;
    if (cur->lc != NULL) {
        cur = cur->lc;
        while (cur->rc != NULL) {
            cur = cur->rc;
        }
    } else {
        while (cur->p != NULL && cur == cur->p->lc) {
            cur = cur->p;
        }
        cur = cur->p;
    }
    return cur;
}

template <typename T>
bool avl_tree<T>::leq(T &v1, T &v2) {
    return lt(v1,v2) || eq(v1,v2);
}

template <typename T>
bool avl_tree<T>::geq(T &v1, T &v2) {
    return gt(v1,v2) || eq(v1,v2);
}

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
    update_height(x);
    update_height(y);
}

template <typename T>
void avl_tree<T>::rotate_right(avl_node<T> *y) {
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
void avl_tree<T>::remove_node_in(avl_node<T> *n_rem, avl_node<T> *n) {
    avl_node<T> *n_rem_p = n_rem->p;
    if (n_rem->lc == NULL) {
        s--;
        if (n_rem->rc == NULL) {
            if (n_rem->p != NULL) {
                if (n_rem == n_rem->p->lc) {
                    n_rem->p->lc = NULL;
                } else {
                    n_rem->p->rc = NULL;
                }
            } else {
                r = NULL;
            }
        } else {
            if (n_rem->p != NULL) {
                if (n_rem == n_rem->p->lc) {
                    n_rem->p->lc = n_rem->rc;
                } else {
                    n_rem->p->rc = n_rem->rc;
                }
                n_rem->rc->p = n_rem->p;
            } else {
                n_rem->rc->p = NULL;
                r = n_rem->rc;
            }
            n_rem->rc = NULL;
        }
    } else if (n_rem->rc == NULL) {
        s--;
        if (n_rem->p != NULL) {
            if (n_rem == n_rem->p->lc) {
                n_rem->p->lc = n_rem->lc;
            } else {
                n_rem->p->rc = n_rem->lc;
            }
            n_rem->lc->p = n_rem->p;
        } else {
            n_rem->lc->p = NULL;
            r = n_rem->lc;
        }
        n_rem->lc = NULL;
    } else {
        avl_node<T> *min = minimum(n_rem->rc);
        avl_node<T> n_tmp = *min;

        min->p = n_rem->p;
        if (n_rem->p != NULL) {
            if (n_rem == n_rem->p->lc) {
                n_rem->p->lc = min;
            } else {
                n_rem->p->rc = min;
            }
        }
        min->lc = n_rem->lc;
        n_rem->lc->p = min;
        min->h = n_rem->h;
        if (min->rc != NULL) {
            min->rc->p = n_rem;
        }
        if (min == n_rem->rc) {
            min->rc = n_rem;
            n_rem->p = min;
        } else {
            min->rc = n_rem->rc;
            n_rem->rc->p = min;
            n_rem->p = n_tmp.p;
            n_tmp.p->lc = n_rem;
        }
        n_rem->rc = n_tmp.rc;
        n_rem->lc = NULL;
        n_rem->h = n_tmp.h;

        remove_node_in(n_rem,min->rc);
    }
    if (n_rem_p != NULL) {
        balance_from_to(n_rem_p,n);
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::build_subtree(int l, int r, std::function<avl_node<T>*(int)> &at, int max_tasks) {
    if (r == l) {
        return at(l);
    } else if (r == l+1) {
        avl_node<T>* n_l = at(l);
        avl_node<T>* n_r = at(r);
        n_r->lc = n_l;
        n_l->p = n_r;
        n_r->h = 1;
        return n_r;
    } else {
        int m = (l+r)/2;
        avl_node<T>* n_m = at(m);
        if (max_tasks > 1) {
            #pragma omp task
            {
                n_m->lc = build_subtree(l,m-1,at,max_tasks/2);
            }
            n_m->rc = build_subtree(m+1,r,at,max_tasks/2);
            #pragma omp taskwait
        } else {
            n_m->lc = build_subtree(l,m-1,at,1);
            n_m->rc = build_subtree(m+1,r,at,1);
        }
        n_m->lc->p = n_m;
        n_m->rc->p = n_m;
        update_height(n_m);
        return n_m;
    }
}

template <typename T>
void avl_tree<T>::delete_subtree(avl_node<T> *n) {
    if (n->lc != NULL) {
        delete_subtree(n->lc);
    }
    if (n->rc != NULL) {
        delete_subtree(n->rc);
    }
    n->p = NULL;
    n->h = 0;
    delete n;
}

template <typename T>
avl_tree<T>::avl_tree(
    std::function<bool(T&,T&)> lt,
    std::function<bool(T&,T&)> gt,
    std::function<bool(T&,T&)> eq
) { 
    this->lt = lt;
    this->gt = gt;
    this->eq = eq;
    fst = lst = r = NULL;
    h = s = 0;
}

template <typename T>
void avl_tree<T>::insert_array(int l, int r, std::function<avl_node<T>*(int)> &at, int max_tasks) {
    if (empty() && l >= 0 && r >= l) {
        this->r = build_subtree(l,r,at,omp_in_parallel() ? max_tasks : 1);
        this->fst = at(l);
        this->lst = at(r);
        h = this->r->h;
        s = r-l+1;
    }
}

template <typename T>
avl_tree<T>::~avl_tree() {
    fst = lst = r = NULL;
    h = s = 0;
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
void avl_tree<T>::delete_nodes() {
    if (!empty()) {
        delete_subtree(r);
        fst = lst = r = NULL;
        h = s = 0;
    }
}

template <typename T>
void avl_tree<T>::disconnect_nodes() {
    fst = lst = r = NULL;
    h = s = 0;
}

template <typename T>
avl_node<T>* avl_tree<T>::find(T &&v, avl_node<T> *n) {
    if (empty()) return NULL;
    do {
        if (gt(v,n->v)) {
            if (n->rc != NULL) {
                n = n->rc;
            } else {
                break;
            }
        } else if (lt(v,n->v)) {
            if (n->lc != NULL) {
                n = n->lc;
            } else {
                break;
            }
        } else {
            break;
        }
    } while (true);
    return n;
}

template <typename T>
avl_node<T>* avl_tree<T>::find(T &v, avl_node<T> *n) {
    return find(std::move(v),n);
}

template <typename T>
avl_node<T>* avl_tree<T>::find(T &&v) {
    return find(v,r);
}

template <typename T>
avl_node<T>* avl_tree<T>::find(T &v) {
    return find(std::move(v),r);
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum() {
    return fst;
}

template <typename T>
avl_node<T>* avl_tree<T>::second_smallest() {
    if (fst->rc != NULL) {
        if (fst->rc->lc != NULL) {
            return fst->rc->lc;
        } else {
            return fst->rc;
        }
    } else {
        return fst->p;
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum() {
    return lst;
}

template <typename T>
avl_node<T>* avl_tree<T>::second_largest() {
    if (lst->lc != NULL) {
        if (lst->lc->rc != NULL) {
            return lst->lc->rc;
        } else {
            return lst->lc;
        }
    } else {
        return lst->p;
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_or_update_in(T &&v, avl_node<T> *n) {
    if (empty()) {
        r = new avl_node<T>(v);
        h = s = 1;
        fst = lst = r;
        return r;
    } else {
        n = find(v,n);
        if (eq(n->v,v)) {
            n->v = v;
            return n;
        } else {
            avl_node<T> *n_new = new avl_node<T>(v);
            if (lt(n_new->v,n->v)) {
                n->lc = n_new;
                n_new->p = n;
            } else {
                n->rc = n_new;
                n_new->p = n;
            }
            balance_from_to(n_new->p,r);
            h = r->h;
            s++;
            if (lt(v,fst->v)) {
                fst = n_new;
            } else if (gt(v,lst->v)) {
                lst = n_new;
            }
            return n_new;
        }
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_or_update_in(T &v, avl_node<T> *n) {
    return insert_or_update_in(std::move(v),n);
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_or_update(T &&v) {
    return insert_or_update_in(v,r);
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_or_update(T &v) {
    return insert_or_update_in(std::move(v),r);
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_node_in(avl_node<T> *n, avl_node<T> *n_in) {
    avl_node<T> *n_at = find(n->v,n_in);
    if (lt(n->v,n_at->v)) {
        n_at->lc = n;
        n->p = n_at;
    } else {
        n_at->rc = n;
        n->p = n_at;
    }
    balance_from_to(n->p,r);
    h = r->h;
    s++;
    if (lt(n->v,fst->v)) {
        fst = n;
    } else if (gt(n->v,lst->v)) {
        lst = n;
    }
    return n;
}

template <typename T>
avl_node<T>* avl_tree<T>::insert_node(avl_node<T> *n) {
    if (empty()) {
        fst = lst = r = n;
        h = s = 1;
        return r;
    } else {
        return insert_node_in(n,r);
    }
}

template <typename T>
avl_node<T>* avl_tree<T>::remove_node(avl_node<T> *n) {
    if (s == 1) {
        r = fst = lst = NULL;
        h = s = 0;
    } else {
        if (n == fst) {
            fst = second_smallest();
        } else if (n == lst) {
            lst = second_largest();
        }
        remove_node_in(n,r);
        h = r->h;
    }
    return n;
}

template <typename T>
avl_node<T>* avl_tree<T>::remove(T &&v) {
    avl_node<T> *n = find(v);
    if (n == NULL || !eq(n->v,v)) return NULL;
    return remove_node(n);
}

template <typename T>
avl_node<T>* avl_tree<T>::remove(T &v) {
    return remove(std::move(v));
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum_geq(T &&v) {
    if (empty()) return NULL;
    avl_node<T> *n = r;
    avl_node<T> *min = NULL;
    do {
        if (lt(n->v,v)) {
            if (n->rc != NULL) {
                n = n->rc;
            } else {
                break;
            }
        } else if (eq(n->v,v)) {
            return n;
        } else if (n->lc != NULL) {
            if (lt(n->lc->v,v)) {
                min = n;
            }
            n = n->lc;
        } else {
            break;
        }
    } while (true);
    if (geq(n->v,v)) {
        return n;
    }
    return min;
}

template <typename T>
avl_node<T>* avl_tree<T>::minimum_geq(T &v) {
    return minimum_geq(std::move(v));
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum_leq(T &&v) {
    if (empty()) return NULL;
    avl_node<T> *n = r;
    avl_node<T>* max = NULL;
    do {
        if (gt(n->v,v)) {
            if (n->lc != NULL) {
                n = n->lc;
            } else {
                break;
            }
        } else if (eq(n->v,v)) {
            return n;
        } else if (n->rc != NULL) {
            if (gt(n->rc->v,v)) {
                max = n;
            }
            n = n->rc;
        } else {
            break;
        }
    } while (true);
    if (leq(n->v,v)) {
        return n;
    }
    return max;
}

template <typename T>
avl_node<T>* avl_tree<T>::maximum_leq(T &v) {
    return maximum_leq(std::move(v));
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
    return t->lt(cur->v,t->lst->v);
}

template <typename T>
bool avl_tree<T>::avl_it::has_prev() {
    return t->gt(cur->v,t->fst->v);
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::current() {
    return cur;
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::next() {
    cur = cur->nxt();
    return cur;
}

template <typename T>
avl_node<T>* avl_tree<T>::avl_it::previous() {
    cur = cur->prv();
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

template <typename T>
typename avl_tree<T>::avl_it avl_tree<T>::iterator() {
    return avl_tree<T>::avl_it(this,fst);
}