#pragma once

template <typename T>
struct dll_node {
    T *v; // value
    dll_node<T> *pr; // predecesor
    dll_node<T> *sc; // successor

    dll_node<T>(T *v); // creates a node with value v
    ~dll_node<T>(); // deletes the node
};

template <typename T>
class dl_list {
    private:
    dll_node<T> *hd; // first node
    dll_node<T> *tl; // last node
    int s; // size
    
    public:
    dl_list(); // creates an empty list
    ~dl_list<T>(); // deletes all nodes of the list and the lsit

    int size(); // returns the number of nodes in the list
    bool isEmpty(); // returns if the list is empty
    dll_node<T>* pushBack(T *v); // creates a node with value v and pushes it to the back of the list, returns the new node
    dll_node<T>* insertAfter(dll_node<T> *n, T *v); // creates a node with value v and inserts it after node n in the list, returns the new node
    dll_node<T>* iterateSuccessor(dll_node<T> *n, int i); // returns the i-th successor of node n if it exists, else returns NULL
    dll_node<T>* head(); // returns the head of the list
    dll_node<T>* tail(); // returns the tail of the list
};