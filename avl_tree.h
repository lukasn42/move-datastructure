#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

/**
 * @brief node in an avl_tree
 * 
 * @tparam T value type
 */
template <typename T>
struct avl_node {
    T v; // value
    avl_node<T> *p; // parent
    avl_node<T> *lc; // left child
    avl_node<T> *rc; // right child
    uint8_t h; // height

    /**
     * @brief deletes the avl_node
     */
    ~avl_node();
};

/**
 * @brief balanced binary search tree, for each node |height of left child - height of right child| <= 1 holds
 * 
 * @tparam T value type
 */
template <typename T>
class avl_tree {
    protected:
    avl_node<T> *r; // root of the avl_tree
    avl_node<T> *fst; // first node (node with the smallest value)
    avl_node<T> *lst; // last node (node with the greatest value)
    uint8_t h; // height
    uint64_t s; // size

    std::function<bool(T*,T*)> lt; // comparison function "less than" on values of type T
    std::function<bool(T*,T*)> gt; // comparison function "greater than" on values of type T
    std::function<bool(T*,T*)> eq; // comparison function "equals" on values of type T

    // comparison function "less than or equal to" on values of type T
    inline bool leq(T *v1, T *v2);

    // comparison function "greater than or equal to" on values of type T
    inline bool geq(T *v1, T *v2);

    /**
     * @brief returns the hight of the node n if n != NULL, else returns 0
     * 
     * @param n an avl_node in the avl_tree
     * @return uint8_t height of n
     */
    inline uint8_t ht(avl_node<T> *n);

    /**
     * @brief updates the height of the subtree of node n
     * 
     * @param n an avl_node in the avl_tree
     * @return true if the height of n changed
     * @return false if the height of n did not change
     */
    inline bool update_height(avl_node<T> *n);

    /**
     * @brief left rotation around node x
     * 
     * @param x an avl_node in the avl_tree, must have a right child
     */
    inline void rotate_left(avl_node<T> *x);

    /**
     * @brief right rotation around node y
     * 
     * @param y an avl_node in the avl_tree, must have a left child
     */
    inline void rotate_right(avl_node<T> *y);

    /**
     * @brief balances the node n
     * 
     * @param n an avl_node in the avl_tree
     * @return true if it was unbalanced
     * @return false if it was balanced
     */
    inline bool balance(avl_node<T> *n);

    /**
     * @brief balances all nodes starting from nf up to nt
     * 
     * @param nf an avl_node in the avl_tree, has to be in the subtree of nt
     * @param nt an avl_node in the avl_tree
     */
    inline void balance_from_to(avl_node<T> *nf, avl_node<T> *nt);

    /**
     * @brief returns the node with the smallest value in the subtree of node n
     * 
     * @param n an avl_node in the avl_tree
     * @return avl_node<T>* the node with the smallest value in the subtree of node n
     */
    inline avl_node<T>* minimum(avl_node<T> *n);

    /**
     * @brief returns the node with the greatest value in the subtree of node n
     * 
     * @param n an avl_node in the avl_tree
     * @return avl_node<T>* the node with the greatest value in the subtree of node n
     */
    inline avl_node<T>* maximum(avl_node<T> *n);

    /**
     * @brief removes the node nr in the subtree of node n
     * 
     * @param n an avl_node in the avl_tree
     * @param nr an avl_node in the avl_tree, must be in the subtree of node n
     */
    void remove_node_in(avl_node<T> *n, avl_node<T> *nr);

    public:
    /**
     * @brief creates an empty avl_tree
     * 
     * @param lt comparison function "less than" on values of type T
     * @param gt comparison function "greater than" on values of type T
     * @param eq comparison function "equals" on values of type T
     */
    avl_tree(
        std::function<bool(T*,T*)> lt,
        std::function<bool(T*,T*)> gt,
        std::function<bool(T*,T*)> eq
    );

    /**
     * @brief deletes the avl_tree and all of it's nodes
     * 
     */
    ~avl_tree();

    /**
     * @brief returns the height of the avl_tree
     * 
     * @return uint8_t height of the avl_tree
     */
    uint8_t height();

    /**
     * @brief returns the number of elements in the avl_tree
     * 
     * @return uint64_t number of elements in the avl_tree
     */
    uint64_t size();

    /**
     * @brief returns whether the avl_tree is empty
     * 
     * @return true if the avl_tree is empty
     * @return false if the avl_tree is not empty
     */
    bool empty();

    /**
     * @brief deletes all nodes in the avl_tree
     */
    void remove_all_nodes();

    /**
     * @brief returns the node with the smallest value in the avl_tree
     * 
     * @return avl_node<T>* the node with the smallest value in the avl_tree, if the avl_tree is not empty
     * @return NULL if the avl_tree is empty
     */
    avl_node<T>* minimum();

    /**
     * @brief returns the node with the greatest value in the avl_tree
     * 
     * @return avl_node<T>* the node with the greatest value in the avl_tree, if the avl_tree is not empty
     * @return NULL if the avl_tree is empty
     */
    avl_node<T>* maximum();

    /**
     * @brief creates and inserts a node with the value v into the avl_tree or updates the node in the avl_tree with a value equal to v
     * 
     * @param v value
     * @return avl_node<T>* the node in the avl_tree with the value v
     */
    avl_node<T>* insert_or_update(T *v);

    /**
     * @brief removes node n from the avl_tree
     * 
     * @param n an avl_node in the avl_tree
     */
    void remove_node(avl_node<T> *n);

    /**
     * @brief removes the node with a value equal to v from the avl_tree
     * 
     * @param v value
     * @return true if there was a node with a value equal to v in the avl_tree
     * @return false if there was no node with a value equal to v in the avl_tree
     */
    bool remove(T *v);

    /**
     * @brief searches for a node with value v in the avl_tree until it was found or a leaf has been reached
     * 
     * @param v value
     * @return avl_node<T>* the node with a value equal to v or a leaf at which a node with value v can be inserted
     * @return NULL if the avl_tree is empty
     */
    avl_node<T>* find(T *v);

    /**
     * @brief returns the node with the smallest value greater than or equal to v
     * 
     * @param v value
     * @return avl_node<T>* the node with the smallest value greater than or equal to v, if it exists
     * @return NULL if all nodes' values are smaller than v
     */
    avl_node<T>* minimum_geq(T *v);

    /**
     * @brief returns the node with the greatest value less than or equal to v
     * 
     * @param v value
     * @return avl_node<T>* the node with the greatest value less than or equal to v, if it exists
     * @return NULL if all nodes' values are greater than v
     */
    avl_node<T>* maximum_leq(T *v); // 

    /**
     * @brief iterator for an avl_tree
     */
    class avl_it {
        protected:
        avl_tree<T> *t; // the avl_tree, the iterator iterates through
        avl_node<T> *cur; // the node the iterator points to

        public:
        /**
         * @brief creates an avl_it pointing to the node n in the avl_tree t
         * 
         * @param t an avl_tree
         * @param n an avl_node in t
         */
        avl_it(avl_tree<T> *t, avl_node<T> *n);

        /**
         * @brief deletes the avl_it
         */
        ~avl_it();
        
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
         * @return avl_node<T>* the node, the iterator points to
         */
        avl_node<T>* current();

        /**
         * @brief iterates forward, has_next() must return true
         * 
         * @return avl_node<T>* the node the iterator points to after iterating forward
         */
        avl_node<T>* next();

        /**
         * @brief iterates forward, has_pred() must return true
         * 
         * @return avl_node<T>* the node the iterator points to after iterating backward
         */
        avl_node<T>* previous();

        /**
         * @brief points the iterator to the node n
         * 
         * @param n an avl_node in t
         */
        void set(avl_node<T> *n);
    };

    /**
     * @brief returns an iterator pointing to the node n
     * 
     * @param n an avl_node in the avl_tree
     * @return avl_tree<T>::avl_it an iterator pointing to the node n
     */
    avl_tree<T>::avl_it iterator(avl_node<T> *n);
};