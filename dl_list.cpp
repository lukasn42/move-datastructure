#include "dl_list.h"

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
    if (!empty()) {
        remove_all_nodes();
    }
}

template <typename T>
bool dl_list<T>::empty() {
    return s == 0;
}

template <typename T>
void dl_list<T>::remove_all_nodes() {
    if (!empty()) {
        auto *n = hd;
        for (uint64_t i=1; i<s; i++) {
            n = n->sc;
            delete n->pr;
        }
        hd = tl = NULL;
        s = 0;
    }
}

template <typename T>
uint64_t dl_list<T>::size() {
    return s;
}

template <typename T>
dll_node<T>* dl_list<T>::head() {
    return hd;
}

template <typename T>
dll_node<T>* dl_list<T>::tail() {
    return tl;
}

template <typename T>
void dl_list<T>::insert_node_after(dll_node<T> *n1, dll_node<T> *n2) {
    n2->pr = n1;
    n2->sc = n1->sc;
    if (n1->sc != NULL) {
        n1->sc->pr = n2;
    }
    n1->sc = n2;
    s++;
}

template <typename T>
void dl_list<T>::push_back_node(dll_node<T> *n) {
    if (empty()) {
        hd = n;
        s = 1;
    } else {
        insert_node_after(tl,n);
    }
    tl = n;
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
dl_list<T>::dll_it::dll_it(dl_list<T> *l, dll_node<T> *n) {
    this->l = l;
    this->cur = n;
}

template <typename T>
dl_list<T>::dll_it::~dll_it() {
    l = NULL;
    cur = NULL;
};

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