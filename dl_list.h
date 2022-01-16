#pragma once

#include <cstddef>
#include <cstdint>

/**
 * @brief node in a dll_list
 * 
 * @tparam T value type
 */
template <typename T>
struct dll_node {
    T v; // value
    dll_node<T> *pr; // predecesor
    dll_node<T> *sc; // successor

    /**
     * @brief deletes the dll_node
     */
    ~dll_node<T>();
};

/**
 * @brief doubly linked list
 * 
 * @tparam T 
 */
template <typename T>
class dl_list {
    protected:
    dll_node<T> *hd; // first node
    dll_node<T> *tl; // last node
    uint64_t s; // size
    
    public:
    /**
     * @brief creates an empty dl_list
     */
    dl_list();

    /**
     * @brief deletes all nodes of the dl_list and the lsit
     */
    ~dl_list();

    /**
     * @brief returns whether the dl_list is empty
     * 
     * @return true if the dl_list is empty
     * @return false if the dl_list is not empty
     */
    bool empty();

    /**
     * @brief deletes all nodes in the dl_list
     */
    void remove_all_nodes();

    /**
     * @brief returns the number of nodes in the dl_list
     * 
     * @return uint64_t number of nodes in the dl_list
     */
    uint64_t size();

    /**
     * @brief returns the head of the dl_list
     * 
     * @return dll_node<T>* the head of the dl_list
     */
    dll_node<T>* head();

    /**
     * @brief returns the tail of the dl_list
     * 
     * @return dll_node<T>* the tail of the dl_list
     */
    dll_node<T>* tail();

    /**
     * @brief inserts a copy of the node n2 after the node n1
     * 
     * @param n1 a dll_node in the dl_list
     * @param n2 a dll_node, that is not in the dl_list, n1 != n2
     */
    void insert_node_after(dll_node<T> *n1, dll_node<T> *n2);

    /**
     * @brief appends a copy of the node n to the end of the dl_list
     * 
     * @param n a dll_node, that is not in the dl_list
     */
    void push_back_node(dll_node<T> *n);

    /**
     * @brief removes the node n from the dl_list
     * 
     * @param n a dll_node in the dl_list
     */
    void remove_node(dll_node<T> *n);

    /**
     * @brief disconnects the dl_list from it's nodes
     */
    void disconnect_nodes();

    /**
     * @brief iterator for a dl_list
     */
    class dll_it {
        protected:
        dl_list<T> *l; // the dl_list, the iterator iterates through
        dll_node<T> *cur; // the node the iterator points to

        public:
        /**
         * @brief creates an dl_it pointing to the node n in the dl_list l
         * 
         * @param l a dl_list
         * @param n a dll_node in l
         */
        dll_it(dl_list<T> *l, dll_node<T> *n);

        /**
         * @brief deletes the iterator
         */
        ~dll_it();

        /**
         * @brief returns if the iterator can iterate forward
         * 
         * @return true if it can iterate forward
         * @return false if it cannot iterate forward
         */
        bool has_next();

        /**
         * @brief returns if the iterator can iterate backward
         * 
         * @return true if it can iterate backward 
         * @return false if it cannot iterate backward 
         */
        bool has_prev();

        /**
         * @brief returns the value of the node, the iterator points to
         * 
         * @return dll_node<T>* the node, the iterator points to
         */
        dll_node<T>* current();

        /**
         * @brief iterates forward, has_next() must return true
         * 
         * @return dll_node<T>* the node the iterator points to after iterating forward
         */
        dll_node<T>* next();

        /**
         * @brief iterates forward, has_pred() must return true
         * 
         * @return dll_node<T>* the node the iterator points to after iterating backward
         */
        dll_node<T>* previous();

        /**
         * @brief points the iterator to the node n
         * 
         * @param n a dll_node in l
         */
        void set(dll_node<T> *n);
    };

    /**
     * @brief returns an iterator pointing to the node n
     * 
     * @param n a dll_node in the dl_list
     * @return dl_list<T>::dll_it an iterator pointing to the node n
     */
    dl_list<T>::dll_it iterator(dll_node<T> *n = head());
};