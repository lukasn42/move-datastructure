#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

template <typename T>
struct dll_node {
    T *v; // value
    dll_node<T> *pr,*sc = NULL; // predeccesor and successor
};

template <typename T>
class dl_list {
    private:
    dll_node<T> *hd,*tl; // first and last node
    int s; // s

    public:
    dl_list() {
        hd = NULL;
        tl = NULL;
        s = 0;
    }

    dll_node<T>* pushBack(T *v) { // pushes v to the back of the list, returns the new node
        dll_node<T> *newN;
        if (!isEmpty()) {
            newN = insertAfter(tl,v);
        } else {
            newN = new dll_node<T> {v};
            hd = newN;
            s = 1;
        }
        tl = newN;
        return newN;
    }

    dll_node<T>* insertAfter(dll_node<T> *n, T *v) { // inserts v after a node in the list, returns the new node
        dll_node<T> *newN = new dll_node<T> {.v = v,.pr = n,.sc = n->sc};
        if (n->sc != NULL) {
            n->sc->pr = newN;
        }
        n->sc = newN;
        s++;
        return newN;
    }

    dll_node<T>* ithSucc(dll_node<T> *n, int i) { // returns the i-th successor of node
        if (i == 0) {
            return n;
        } else if (i > 0 && n->sc != NULL) {
            return ithSucc(n->sc,i-1);
        } else {
            return NULL;
        }
    }

    dll_node<T>* head() { // returns the hd of the list
        return hd;
    }

    dll_node<T>* tail() { // returns the tl of the list
        return tl;
    }

    bool isEmpty() { // returns if the list is empty
        return s == 0;
    }

    int size() { // returns the number of elements in the list
        return s;
    }
};