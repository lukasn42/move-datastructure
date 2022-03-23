#include <utility>
#include <cstddef>
#include <cstdint>

#include <dl_list.hpp>

template <typename T>
dll_node<T>::dll_node() {
    pr = sc = NULL;
}

template <typename T>
dll_node<T>::dll_node(T v) {
    this->v = v;
    pr = sc = NULL;
}

template <typename T>
dll_node<T>::~dll_node() {
    pr = sc = NULL;
}

template <typename T>
dl_list<T>::dl_list() {
    hd = tl = NULL;
    s = 0;
}

template <typename T>
dl_list<T>::~dl_list() {
    delete_nodes();
}

template <typename T>
bool dl_list<T>::empty() {
    return s == 0;
}

template <typename T>
uint64_t dl_list<T>::size() {
    return s;
}

template <typename T>
void dl_list<T>::set_size(uint64_t s) {
    this->s = s;
}

template <typename T>
dll_node<T>* dl_list<T>::head() {
    return hd;
}

template <typename T>
void dl_list<T>::set_head(dll_node<T> *n) {
    this->hd = n;
}

template <typename T>
dll_node<T>* dl_list<T>::tail() {
    return tl;
}

template <typename T>
void dl_list<T>::set_tail(dll_node<T> *n) {
    this->tl = n;
}

template <typename T>
dll_node<T>* dl_list<T>::push_front(T &&v) {
    return push_front_node(new dll_node<T>(v));
}

template <typename T>
dll_node<T>* dl_list<T>::push_front(T &v) {
    return push_front_node(new dll_node<T>(std::move(v)));
}

template <typename T>
dll_node<T>* dl_list<T>::push_back(T &&v) {
    return push_back_node(new dll_node<T>(v));
}

template <typename T>
dll_node<T>* dl_list<T>::push_back(T &v) {
    return push_back_node(new dll_node<T>(std::move(v)));
}

template <typename T>
dll_node<T>* dl_list<T>::insert_before(T &&v, dll_node<T> *n) {
    return insert_before_node(new dll_node<T>(v),n);
}

template <typename T>
dll_node<T>* dl_list<T>::insert_before(T &v, dll_node<T> *n) {
    return insert_before_node(new dll_node<T>(std::move(v)),n);
}

template <typename T>
dll_node<T>* dl_list<T>::insert_after(T &&v, dll_node<T> *n) {
    return insert_after_node(new dll_node<T>(v),n);
}

template <typename T>
dll_node<T>* dl_list<T>::insert_after(T &v, dll_node<T> *n) {
    return insert_after_node(new dll_node<T>(std::move(v)),n);
}

template <typename T>
void dl_list<T>::insert_before_node(dll_node<T> *n1, dll_node<T> *n2) {
    if (n2 == hd) {
        hd = n1;
    } else {
        n2->pr->sc = n1;
        n1->pr = n2->pr;
    }
    n1->sc = n2;
    n2->pr = n1;
    s++;
}

template <typename T>
void dl_list<T>::insert_after_node(dll_node<T> *n1, dll_node<T> *n2) {
    if (n2 == tl) {
        tl = n1;
    } else {
        n2->sc->pr = n1;
        n1->sc = n2->sc;
    }
    n1->pr = n2;
    n2->sc = n1;
    s++;
}

template <typename T>
void dl_list<T>::push_front_node(dll_node<T> *n) {
    if (empty()) {
        hd = tl = n;
        s = 1;
    } else {
        insert_before_node(n,hd);
    }
}

template <typename T>
void dl_list<T>::push_back_node(dll_node<T> *n) {
    if (empty()) {
        hd = tl = n;
        s = 1;
    } else {
        insert_after_node(n,tl);
    }
}

template <typename T>
void dl_list<T>::concat(dl_list<T> *l) {
    if (empty()) {
        if (!l->empty()) {
            hd = l->hd;
            tl = l->tl;
            s = l->s;
            l->disconnect_nodes();
        }
    } else if (!l->empty()) {
        tl->sc = l->hd;
        l->hd->pr = tl;
        tl = l->tail();
        s += l->s;
        l->disconnect_nodes();
    }
}

template <typename T>
void dl_list<T>::remove_node(dll_node<T> *n) {
    s--;
    if (n == hd) {
        hd = n->sc;
    } else if (n == tl) {
        tl = n->pr;
    }
    if (n->pr != NULL) {
        n->pr->sc = n->sc;
    }
    if (n->sc != NULL) {
        n->sc->pr = n->pr;
    }
}

template <typename T>
void dl_list<T>::disconnect_nodes() {
    hd = tl = NULL;
    s = 0;
}

template <typename T>
void dl_list<T>::delete_nodes() {
    if (!empty()) {
        dll_node<T> *n = hd;
        for (uint64_t i=1; i<s; i++) {
            n = n->sc;
            delete n->pr;
        }
        hd = tl = NULL;
        s = 0;
    }
}

template <typename T>
dl_list<T>::dll_it::dll_it(dl_list<T> *l, dll_node<T> *n) {
    this->l = l;
    this->cur = n;
}

template <typename T>
dl_list<T>::dll_it::~dll_it() {
    l = NULL;
    cur = NULL;
}

template <typename T>
bool dl_list<T>::dll_it::has_next() {
    return cur->sc != NULL;
}

template <typename T>
bool dl_list<T>::dll_it::has_prev() {
    return cur->pr != NULL;
}

template <typename T>
dll_node<T>* dl_list<T>::dll_it::current() {
    return cur;
}

template <typename T>
dll_node<T>* dl_list<T>::dll_it::next() {
    cur = cur->sc;
    return cur;
}

template <typename T>
dll_node<T>* dl_list<T>::dll_it::previous() {
    cur = cur->pr;
    return cur;
}

template <typename T>
void dl_list<T>::dll_it::set(dll_node<T> *n) {
    cur = n;
}

template <typename T>
typename dl_list<T>::dll_it dl_list<T>::iterator(dll_node<T> *n) {
    return dl_list<T>::dll_it(this,n);
}

template <typename T>
typename dl_list<T>::dll_it dl_list<T>::iterator() {
    return dl_list<T>::dll_it(this,hd);
}