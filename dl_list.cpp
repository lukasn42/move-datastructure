#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

template <typename T>
struct dll_node {
    T *val; // value
    dll_node<T> *pred,*succ = NULL; // predeccesor and successor
};

template <typename T>
class dl_list {
    private:
    dll_node<T> *head,*tail; // first and last node
    int size; // size

    public:
    dl_list() {
        head = NULL;
        tail = NULL;
        size = 0;
    }

    dll_node<T>* pushBack(T *val) { // pushes value to the back of the list, returns the new node
        dll_node<T> *newNode;
        if (!isEmpty()) {
            newNode = insertAfter(tail,val);
            size++;
        } else {
            newNode = new dll_node<T> {val};
            head = newNode;
            size = 1;
        }
        tail = newNode;
        return newNode;
    }

    dll_node<T>* insertAfter(dll_node<T> *node, T *val) { // inserts value after a node in the list, returns the new node
        dll_node<T> *newNode = new dll_node<T> {.val = val,.pred = node,.succ = node->succ};
        if (node->succ != NULL) {
            node->succ->pred = newNode;
        }
        node->succ = newNode;
        return newNode;
    }

    dll_node<T>* ithSucc(dll_node<T> *node, int i) { // returns the i-th successor of node
        if (i == 0) {
            return node;
        } else if (i > 0 && node->succ != NULL) {
            return ithSucc(node->succ,i-1);
        } else {
            return NULL;
        }
    }

    dll_node<T>* getHead() { // returns the head of the list
        return head;
    }

    dll_node<T>* getTail() { // returns the tail of the list
        return tail;
    }

    bool isEmpty() { // returns if the list is empty
        return getSize() == 0;
    }

    int getSize() { // returns the number of elements in the list
        return size;
    }
};