#pragma once

template <typename T>
struct avl_node {
    T *v; // value
    avl_node<T> *p; // parent
    avl_node<T> *lc,*rc; // left child, right child
    int h; // height

    avl_node<T>(T *v); // creates a node with value v
    ~avl_node<T>(); // recursively deletes all children of the node and the node
};

template <typename T>
class avl_tree {
    private:
    avl_node<T> *r; // root
    int h; // height
    int s; // size

    // comparison operators
    std::function<bool(T*,T*)> lt;
    std::function<bool(T*,T*)> gt;
    std::function<bool(T*,T*)> eq;
    bool leq(T *v1, T *v2);
    bool geq(T *v1, T *v2);

    int ht(avl_node<T> *n);
    void rotateLeft(avl_node<T> *x); // left rotation around x, x must have a right child
    void rotateRight(avl_node<T> *y); // right rotation around y, y must have a left child
    void balance(avl_node<T> *n); // balances the subtree of node n
    void updateHeight(avl_node<T> *n); // updates height of the subtree of node n
    avl_node<T>* maximum(avl_node<T> *n); // finds the node with the greatest value in the subtree of node n
    int insert(avl_node<T> *n, T *v, bool upd);
    // inserts value v in the subtree of node n; returns 0 if it could not be inserted, returns != 0 if it could be inserted
    // if upd == true and there is a node with value v_ == v in the subtree of node n, v_ gets deleted and replaced by v and 2 is returned
    // if upd == false and there is a node with value v_ == v in the subtree of node n, v stays in the tree and 1 is returned
    bool remove(avl_node<T> *n, T *v); // remove value v in the subtree of node n
    T* removeMin(avl_node<T> *n); // finds the node with the smallest value in the subtree of node n, removes it and returns it's value
    T* minElemGreaterOrEqual(avl_node<T> *n, T *v); // returns the node with the minimum value >= v in the subtree of node n
    T* maxElemLessOrEqual(avl_node<T> *n, T *v); // returns the node with the maximum value <= v in the subtree of node n

    public:
    avl_tree<T>( // constructs empty tree
        std::function<bool(T*,T*)> lt,
        std::function<bool(T*,T*)> gt,
        std::function<bool(T*,T*)> eq
    );
    ~avl_tree<T>(); // recursively deletes all nodes in the tree and the tree

    int height(); // returns the height of the tree
    int size(); // returns the size of the tree
    bool isEmpty(); // returns if the tree is empty
    int insert(T *v, bool upd);
    // inserts value v into the tree; returns 0 if it could not be inserted, returns != 0 if it could be inserted
    // if upd == true and there is a node with value v_ == v in the tree, v_ gets deleted and replaced by v and 2 is returned
    // if upd == false and there is a node with value v_ == v in the tree, v stays in the tree and 1 is returned
    bool remove(T *v); // removes value v, returns if it could be removed
    T* removeMin(); // finds the node with the smallest value in the avl tree, removes it and returns it's value
    T* minElemGreaterOrEqual(T *v); // returns the node with the minimum value >= v in the avl tree
    T* maxElemLessOrEqual(T *v); // returns the node with the maximum value <= v in the avl tree
};