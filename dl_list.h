#pragma once

#include <cstddef>
#include <cstdint>

template <typename T>
struct dll_node { // doubly linked list node
    T v; // value
    dll_node<T> *pr; // predecesor
    dll_node<T> *sc; // successor

    ~dll_node<T>(); // deletes the node
};

template <typename T>
class dl_list { // doubly linked list
    protected:
    dll_node<T> *hd; // first node
    dll_node<T> *tl; // last node
    uint64_t s; // size
    
    public:
    dl_list(); // creates an empty list
    ~dl_list(); // deletes all nodes of the list and the lsit

    bool empty(); // returns whether the list is empty
    uint64_t size(); // returns the number of nodes in the list
    dll_node<T>* head(); // returns the head of the list
    dll_node<T>* tail(); // returns the tail of the list
    void insert_node_after(dll_node<T> *n1, dll_node<T> *n2); // inserts the node n2 after the node n1, n1 must be in the list and must not equal tl
    void push_back_node(dll_node<T> *n); // appends the node n to the end of the list
    void remove_node(dll_node<T> *n); // removes the node n from the list, n has to be in the list
    void disconnect_from_nodes(); // disconnects the lsit from it's nodes

    class dll_it {
        protected:
        dl_list<T> *l; // list, the iterator iterates through
        dll_node<T> *cur; // current node in the list

        public:
        dll_it(dl_list<T> *l, dll_node<T> *n); // creates iterator pointing to node n in the list l
        ~dll_it(); // deletes the iterator

        bool has_next(); // returns if the iterator can iterate forward
        bool has_prev(); // returns if the iterator can iterate backward
        dll_node<T>* current(); // returns the value of the current node
        dll_node<T>* next(); // iterates forward and returns the new current node, cur must have a next node
        dll_node<T>* previous(); // iterates backward and returns the new current node, cur must have a previous node
        void set(dll_node<T> *n); // points the iterator to the node n
    };

    dl_list<T>::dll_it iterator(dll_node<T> *n = head()); // returns an iterator pointing to node n, n has to be in the tree
};