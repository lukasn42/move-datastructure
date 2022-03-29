#pragma once

/**
 * @brief node in a dll_list
 * @tparam T value type
 */
template <typename T>
struct dll_node {
    T v; // value
    dll_node<T> *pr; // predecesor
    dll_node<T> *sc; // successor

    /**
     * @brief creates a dll_node with value v
     */
    dll_node();

    /**
     * @brief creates a dll_node with value v
     */
    dll_node(T v);

    /**
     * @brief deletes the dll_node
     */
    ~dll_node();
};

/**
 * @brief doubly linked list
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
     * @brief checks whether the dl_list is empty
     * @return whether the dl_list is empty
     */
    bool empty();

    /**
     * @brief returns the number of nodes in the dl_list
     * @return number of nodes in the dl_list
     */
    uint64_t size();

    /**
     * @brief adjusts the size of the dl_list
     * @param s new size
     */
    void set_size(uint64_t s);

    /**
     * @brief returns the head of the dl_list
     * @return the head of the dl_list
     */
    dll_node<T>* head();

    /**
     * @brief adjusts the head of the dl_list
     * @param n new head
     */
    void set_head(dll_node<T> *n);

    /**
     * @brief returns the tail of the dl_list
     * @return the tail of the dl_list
     */
    dll_node<T>* tail();

    /**
     * @brief adjusts the head of the dl_list
     * @param n new tail
     */
    void set_tail(dll_node<T> *n);

    /**
     * @brief inserts a node with value v before the head of the dl_list
     * @param v value
     * @return the newly created node
     */
    dll_node<T>* push_front(T &&v);

    /**
     * @brief inserts a node with value v before the head of the dl_list
     * @param v value
     * @return the newly created node
     */
    dll_node<T>* push_front(T &v);

    /**
     * @brief inserts a node with value v after the head of the dl_list
     * @param v value
     * @return the newly created node
     */
    dll_node<T>* push_back(T &&v);

    /**
     * @brief inserts a node with value v after the head of the dl_list
     * @param v value
     * @return the newly created node
     */
    dll_node<T>* push_back(T &v);

    /**
     * @brief inserts a node with value v before the node n2
     * @param v value
     * @param n2 a dll_node in the dl_list
     * @return the newly created node
     */
    dll_node<T>* insert_before(T &&v, dll_node<T> *n);

    /**
     * @brief inserts a node with value v before the node n2
     * @param v value
     * @param n2 a dll_node in the dl_list
     * @return the newly created node
     */
    dll_node<T>* insert_before(T &v, dll_node<T> *n);

    /**
     * @brief inserts a node with value v after the node n2
     * @param v value
     * @param n2 a dll_node in the dl_list
     * @return the newly created node
     */
    dll_node<T>* insert_after(T &&v, dll_node<T> *n);

    /**
     * @brief inserts a node with value v after the node n2
     * @param v value
     * @param n2 a dll_node in the dl_list
     * @return the newly created node
     */
    dll_node<T>* insert_after(T &v, dll_node<T> *n);

    /**
     * @brief inserts the node n1 before the node n2
     * @param n1 a dll_node, that is not in the dl_list
     * @param n2 a dll_node in the dl_list, n1 != n2
     */
    void insert_before_node(dll_node<T> *n1, dll_node<T> *n2);

    /**
     * @brief inserts the node n1 after the node n2
     * @param n1 a dll_node, that is not in the dl_list
     * @param n2 a dll_node in the dl_list, n1 != n2
     */
    void insert_after_node(dll_node<T> *n1, dll_node<T> *n2);

    /**
     * @brief inserts the node n before the head of the dl_list
     * @param n a dll_node, that is not in the dl_list
     */
    void push_front_node(dll_node<T> *n);

    /**
     * @brief inserts the node n after the tail of the dl_list
     * @param n a dll_node, that is not in the dl_list
     */
    void push_back_node(dll_node<T> *n);

    /**
     * @brief concatenates the dl_list l to the end of the dl_list
     * @param l another dl_list
     */
    void concat(dl_list<T> *l);

    /**
     * @brief removes the node n from the dl_list
     * @param n a dll_node in the dl_list
     */
    void remove_node(dll_node<T> *n);

    /**
     * @brief disconnects the dl_list from it's nodes
     */
    void disconnect_nodes();

    /**
     * @brief deletes all nodes in the dl_list
     */
    void delete_nodes();

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
         * @param l a dl_list
         * @param n a dll_node in l
         */
        dll_it(dl_list<T> *l, dll_node<T> *n);

        /**
         * @brief deletes the iterator
         */
        ~dll_it();

        /**
         * @brief checks whether the iterator can iterate forward
         * @return whether it can iterate forward
         */
        bool has_next();

        /**
         * @brief checks whether the iterator can iterate backward
         * @return whether it can iterate backward
         */
        bool has_prev();

        /**
         * @brief returns the value of the node, the iterator points to
         * @return the node, the iterator points to
         */
        dll_node<T>* current();

        /**
         * @brief iterates forward, has_next() must return true
         * @return the node the iterator points to after iterating forward
         */
        dll_node<T>* next();

        /**
         * @brief iterates forward, has_pred() must return true
         * @return the node the iterator points to after iterating backward
         */
        dll_node<T>* previous();

        /**
         * @brief points the iterator to the node n
         * @param n a dll_node in l
         */
        void set(dll_node<T> *n);
    };

    /**
     * @brief returns an iterator pointing to the node n
     * 
     * @param n a dll_node in the dl_list
     * @return an iterator
     */
    dl_list<T>::dll_it iterator(dll_node<T> *n);

    /**
     * @brief returns an iterator pointing to the head of the list
     * @return an iterator
     */
    dl_list<T>::dll_it iterator();
};