#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

template <typename T>
struct avl_node { // AVL node
    T v; // value
    avl_node<T> *p; // parent
    avl_node<T> *lc; // left child
    avl_node<T> *rc; // right child
    uint8_t h; // height

    ~avl_node(); // deletes the node, it's value and it's children
};

template <typename T>
class avl_tree { // AVL tree
    protected:
    avl_node<T> *r; // root
    avl_node<T> *fst; // first node
    avl_node<T> *lst; // last node
    uint8_t h; // height
    uint64_t s; // size

    // comparison operators
    std::function<bool(T*,T*)> lt;
    std::function<bool(T*,T*)> gt;
    std::function<bool(T*,T*)> eq;
    inline bool leq(T *v1, T *v2);
    inline bool geq(T *v1, T *v2);

    inline uint8_t ht(avl_node<T> *n); // returns the hight of the node n if n != NULL, else returns 0
    inline bool update_height(avl_node<T> *n); // updates the height of the subtree of node n, returns if it changed
    inline void rotate_left(avl_node<T> *x); // left rotation around node x, x must have a right child
    inline void rotate_right(avl_node<T> *y); // right rotation around node y, y must have a left child
    inline bool balance(avl_node<T> *n); // balances the node n, returns if it was unbalanced
    inline void balance_from_to(avl_node<T> *nf, avl_node<T> *nt); // balances all nodes starting from nf up to nt, nf has to be in the subtree of nt
    inline avl_node<T>* find(avl_node<T> *n, T *v); // returns the node with value v in the subtree of the node n, if it exists, else returns NULL
    inline avl_node<T>* minimum(avl_node<T> *n); // returns the node with the smallest value in the subtree of node n
    inline avl_node<T>* maximum(avl_node<T> *n); // returns the node with the greatest value in the subtree of node n
    void remove_node(avl_node<T> *n, avl_node<T> *nr); // removes the node nr from the subtree of node n, nr has to be in the subtree of node n

    public:
    avl_tree( // creates empty tree
        std::function<bool(T*,T*)> lt,
        std::function<bool(T*,T*)> gt,
        std::function<bool(T*,T*)> eq
    );
    ~avl_tree(); // recursively deletes all nodes in the tree and the tree

    uint8_t height(); // returns the height of the tree
    uint64_t size(); // returns the size of the tree
    bool empty(); // returns whether the tree is empty
    avl_node<T>* minimum(); // returns the node node with the smallest value in the tree
    avl_node<T>* maximum(); // returns the node node with the greatest value in the tree
    avl_node<T>* insert(T *v); // inserts value v into the tree, returns the new node if it could be inserted, else returns NULL
    void remove_node(avl_node<T> *n); // removes node n from the tree, n has to be in the tree
    bool remove(T *v); // removes the node with value v from the tree if it exists, returns if it could be removed
    avl_node<T>* find(T *v); // returns the node with value v, if it exists, else returns NULL
    avl_node<T>* minimum_geq(T *v); // returns the node with the minimum value >= v in the tree
    avl_node<T>* maximum_leq(T *v); // returns the node with the maximum value <= v in the tree

    class avl_it { // AVL iterator
        protected:
        avl_tree<T> *t; // tree, the iterator iterates through
        avl_node<T> *cur; // current node in the tree

        public:
        avl_it(avl_tree<T> *t, avl_node<T> *n); // creates iterator pointing to node n in the tree t
        ~avl_it(); // deletes the iterator
        
        bool has_next(); // returns if the iterator can iterate forward
        bool has_prev(); // returns if the iterator can iterate backward
        avl_node<T>* current(); // returns the value of the current node
        avl_node<T>* next(); // iterates forward and returns the new current node, has_next() must return true
        avl_node<T>* previous(); // iterates backward and returns the new current node, has_prev() must return true
        void set(avl_node<T> *n); // points the iterator to the node n
    };

    avl_tree<T>::avl_it iterator(avl_node<T> *n); // returns an iterator pointing to node n, n has to be in the tree
};