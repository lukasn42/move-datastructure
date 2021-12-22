#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "dl_list.h"

template <typename T>
dll_node<T>::dll_node(T *v) { // creates a node with value v
    this->v = v;
}

template <typename T>
dll_node<T>::~dll_node() { // deletes the node
    pr = sc = NULL;
}

template <typename T>
dl_list<T>::dl_list() { // creates an empty list
    hd = NULL;
    tl = NULL;
    s = 0;
}

template <typename T>
dl_list<T>::~dl_list() { // deletes all nodes of the list and the lsit
    dll_node<T> *n = hd;
    for (int i=0; i<s-1; i++) {
        n = n->sc;
        delete n->pr;
    }
    delete n;        
}

template <typename T>
int dl_list<T>::size() { // returns the number of nodes in the list
    return s;
}

template <typename T>
bool dl_list<T>::isEmpty() { // returns if the list is empty
    return s == 0;
}

template <typename T>
dll_node<T>* dl_list<T>::pushBack(T *v) { // creates a node with value v and pushes it to the back of the list, returns the new node
    dll_node<T> *newN;
    if (!isEmpty()) {
        newN = insertAfter(tl,v);
    } else {
        newN = new dll_node<T>(v);
        hd = newN;
        s = 1;
    }
    tl = newN;
    return newN;
}

template <typename T>
dll_node<T>* dl_list<T>::insertAfter(dll_node<T> *n, T *v) { // creates a node with value v and inserts it after node n in the list, returns the new node
    dll_node<T> *newN = new dll_node<T>(v);
    newN->pr = n;
    newN->sc = n->sc;
    if (n->sc != NULL) {
        n->sc->pr = newN;
    }
    n->sc = newN;
    s++;
    return newN;
}

template <typename T>
dll_node<T>* dl_list<T>::iterateSuccessor(dll_node<T> *n, int i) { // returns the i-th successor of node n if it exists, else returns NULL
    if (i == 0) {
        return n;
    } else if (i > 0 && n->sc != NULL) {
        return iterateSuccessor(n->sc,i-1);
    } else {
        return NULL;
    }
}

template <typename T>
dll_node<T>* dl_list<T>::head() { // returns the head of the list
    return hd;
}

template <typename T>
dll_node<T>* dl_list<T>::tail() { // returns the tail of the list
    return tl;
}