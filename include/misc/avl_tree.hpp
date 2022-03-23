#pragma once

/**
 * @brief node in an avl_tree
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
     * @brief creates an avl_node with value v
     */
    avl_node<T>();

    /**
     * @brief creates an avl_node with value v
     */
    avl_node<T>(T v);

    /**
     * @brief deletes the avl_node
     */
    ~avl_node<T>();

    /**
     * @brief returns the next avl_node in the avl_tree
     * @return the next avl_node in the avl_tree
     */
    avl_node<T>* nxt();

    /**
     * @brief returns the previous avl_node in the avl_tree
     * @return the previous avl_node in the avl_tree
     */
    avl_node<T>* prv();
};

/**
 * @brief balanced binary search tree, for each node |height of left child - height of right child| <= 1 holds
 * @tparam T value type
 */
template <typename T>
class avl_tree {
    protected:
    avl_node<T> *r; // root of the avl_tree
    avl_node<T> *fst; // first node (node with the smallest value)
    avl_node<T> *lst; // last node (node with the greatest value)
    uint64_t s; // size
    uint8_t h; // height

    std::vector<avl_node<T>> *nds; // array of nodes that have been inserted whith insert_array()

    std::function<bool(T&,T&)> lt; // comparison function "less than" on values of type T
    std::function<bool(T&,T&)> gt; // comparison function "greater than" on values of type T
    std::function<bool(T&,T&)> eq; // comparison function "equals" on values of type T

    // comparison function "less than or equal to" on values of type T
    inline bool leq(T &v1, T &v2);

    // comparison function "greater than or equal to" on values of type T
    inline bool geq(T &v1, T &v2);

    /**
     * @brief returns the hight of the node n if n != NULL, else returns 0
     * 
     * @param n an avl_node in the avl_tree
     * @return height of n
     */
    inline uint8_t ht(avl_node<T> *n);

    /**
     * @brief updates the height of the subtree of node n
     * 
     * @param n an avl_node in the avl_tree
     * @return whether the height of n changed
     */
    inline bool update_height(avl_node<T> *n);

    /**
     * @brief left rotation around node x
     * @param x an avl_node in the avl_tree, must have a right child
     */
    inline void rotate_left(avl_node<T> *x);

    /**
     * @brief right rotation around node y
     * @param y an avl_node in the avl_tree, must have a left child
     */
    inline void rotate_right(avl_node<T> *y);

    /**
     * @brief balances the node n
     * 
     * @param n an avl_node in the avl_tree
     * @return whether n was unbalanced
     */
    inline bool balance(avl_node<T> *n);

    /**
     * @brief balances all nodes starting from nf up to nt
     * @param nf an avl_node in the avl_tree, has to be in the subtree of nt
     * @param nt an avl_node in the avl_tree
     */
    inline void balance_from_to(avl_node<T> *nf, avl_node<T> *nt);

    /**
     * @brief returns the node with the smallest value in the subtree of node n
     * @param n an avl_node in the avl_tree
     * @return the node with the smallest value in the subtree of node n
     */
    inline avl_node<T>* minimum(avl_node<T> *n);

    /**
     * @brief returns the node with the greatest value in the subtree of node n
     * @param n an avl_node in the avl_tree
     * @return the node with the greatest value in the subtree of node n
     */
    inline avl_node<T>* maximum(avl_node<T> *n);

    /**
     * @brief removes the node nr in the subtree of node n
     * @param nr an avl_node in the avl_tree, must be in the subtree of node n
     * @param n an avl_node in the avl_tree
     */
    inline void remove_node_in(avl_node<T> *nr, avl_node<T> *n);

    /**
     * @brief creates a balanced avl subtree of the nodes in nds[l..r] and returns it's root node
     * @param nds array of avl_nodes, must be sorted according to the comparison operators
     * @param l l in [0..|nds|-1]
     * @param r r in [0..|nds|-1], l <= r
     * @param max_tasks maximum number of tasks to start
     * @return the root of the avl subtree
     */
    inline avl_node<T>* build_subtree(std::vector<avl_node<T>> *nds, int l, int r, int max_tasks = 1, std::function<int(int)> at = [](int i){return i;});

    /**
     * @brief deletes the avl_node n
     * @param n an avl_node in the avl_tree
     */
    inline void delete_node(avl_node<T> *n);

    /**
     * @brief recursively deletes all nodes in the subtree of node n
     * @param n an avl_node
     * @param max_tasks maximum number of tasks to start
     */
    inline void delete_subtree(avl_node<T> *n, int max_tasks = 1);

    public:
    /**
     * @brief creates an empty avl_tree
     * @param lt comparison function "less than" on values of type T
     * @param gt comparison function "greater than" on values of type T
     * @param eq comparison function "equals" on values of type T
     */
    avl_tree<T>(
        std::function<bool(T&,T&)> lt,
        std::function<bool(T&,T&)> gt,
        std::function<bool(T&,T&)> eq
    );

    /**
     * @brief deletes the avl_tree and all of it's nodes
     */
    ~avl_tree<T>();

    /**
     * @brief returns the height of the avl_tree
     * @return height of the avl_tree
     */
    uint8_t height();

    /**
     * @brief returns the number of elements in the avl_tree
     * @return number of elements in the avl_tree
     */
    uint64_t size();

    /**
     * @brief returns whether the avl_tree is empty
     * @return whether the avl_tree is empty
     */
    bool empty();

    /**
     * @brief deletes all nodes in the avl_tree
     * @param max_tasks maximum number of tasks to start
     */
    void delete_nodes(int max_tasks = 1);

    /**
     * @brief disconnects all nodes in the avl_tree
     */
    void disconnect_nodes();

    /**
     * @brief inserts the nodes[l..r] in nds into the avl_tree if it is empty
     * @param nds sorted array of avl_nodes
     * @param l l in [0..|nds|-1]
     * @param r r in [0..|nds|-1], l <= r
     * @param max_tasks maximum number of tasks to start
     */
    void insert_array(std::vector<avl_node<T>> *nds, int l, int r, int max_tasks = 1, std::function<int(int)> at = [](int i){return i;});

    /**
     * @brief returns the node with the smallest value in the avl_tree
     * @return the node with the smallest value in the avl_tree if the avl_tree is not empty, else NULL
     */
    avl_node<T>* minimum();

    /**
     * @brief returns the node with the second smallest value in the avl_tree
     * @return the node with the second smallest value in the avl_tree if the avl_tree has at least 2 nodes, else NULL
     */
    avl_node<T>* second_smallest();

    /**
     * @brief returns the node with the greatest value in the avl_tree
     * @return the node with the greatest value in the avl_tree if the avl_tree is not empty, else NULL
     */
    avl_node<T>* maximum();

    /**
     * @brief returns the node with the second largest value in the avl_tree
     * @return the node with the second largest value in the avl_tree if the avl_tree has at least 2 nodes, else NULL
     */
    avl_node<T>* second_largest();

    /**
     * @brief creates and inserts a node with the value v into the avl_tree or
     *        updates the node in the avl_tree with a value equal to v
     * @param v value
     * @param n an avl_node in the avl_tree, v must be able to be inserted into the subtree of node n
     * @return the node in the avl_tree with the value v
     */
    avl_node<T>* insert_or_update_in(T &&v, avl_node<T> *n);

    /**
     * @brief creates and inserts a node with the value v into the avl_tree or
     *        updates the node in the avl_tree with a value equal to v
     * @param v value
     * @param n an avl_node in the avl_tree, v must be able to be inserted into the subtree of node n
     * @return the node in the avl_tree with the value v
     */
    avl_node<T>* insert_or_update_in(T &v, avl_node<T> *n);

    /**
     * @brief creates and inserts a node with the value v into the avl_tree or
     *        updates the node in the avl_tree with a value equal to v
     * @param v value
     * @return the node in the avl_tree with the value v
     */
    avl_node<T>* insert_or_update(T &&v);

    /**
     * @brief creates and inserts a node with the value v into the avl_tree or
     *        updates the node in the avl_tree with a value equal to v
     * @param v value
     * @return the node in the avl_tree with the value v
     */
    avl_node<T>* insert_or_update(T &v);

    /**
     * @brief inserts the node n into the avl_tree
     * @param n an avl_node, it and it's value must not be in the avl_tree
     * @param nin an avl_node in the avl_tree, n must be able to be inserted into the subtree of node na
     * @return n
     */
    avl_node<T>* insert_node_in(avl_node<T> *n, avl_node<T> *n_in);

    /**
     * @brief inserts the node n into the avl_tree
     * @param n an avl_node, it and it's value must not be in the avl_tree
     * @return n
     */
    avl_node<T>* insert_node(avl_node<T> *n);

    /**
     * @brief removes node n from the avl_tree
     * @param n an avl_node in the avl_tree
     */
    void remove_node(avl_node<T> *n);

    /**
     * @brief removes the node with a value equal to v from the avl_tree
     * @param v value
     * @return whether there was a node with a value equal to v in the avl_tree
     */
    bool remove(T &&v);

    /**
     * @brief removes the node with a value equal to v from the avl_tree
     * @param v value
     * @return whether there was a node with a value equal to v in the avl_tree
     */
    bool remove(T &v);

    /**
     * @brief searches for a node with value v in the avl_tree until it was found or a leaf has been reached
     * @param v value
     * @param n an avl_node in the avl_tree, at which the search starts
     * @return the node with a value equal to v or a leaf at which a node with value v can be inserted
     *         if the avl_tree is not empty, else NULL
     */
    avl_node<T>* find(T &&v, avl_node<T> *n);

    /**
     * @brief searches for a node with value v in the avl_tree until it was found or a leaf has been reached
     * @param v value
     * @param n an avl_node in the avl_tree, at which the search starts
     * @return the node with a value equal to v or a leaf at which a node with value v can be inserted
     *         if the avl_tree is not empty, else NULL
     */
    avl_node<T>* find(T &v, avl_node<T> *n);

    /**
     * @brief searches for a node with value v in the avl_tree until it was found or a leaf has been reached
     * @param v value
     * @return avl_node<T>* the node with a value equal to v or a leaf at which a node with value v can be inserted
     *         if the avl_tree is not empty, else NULL
     */
    avl_node<T>* find(T &&v);

    /**
     * @brief searches for a node with value v in the avl_tree until it was found or a leaf has been reached
     * @param v value
     * @return avl_node<T>* the node with a value equal to v or a leaf at which a node with value v can be inserted
     *         if the avl_tree is not empty, else NULL
     */
    avl_node<T>* find(T &v);

    /**
     * @brief returns the node with the smallest value greater than or equal to v
     * @param v value
     * @return avl_node<T>* the node with the smallest value greater than or equal to v, if it exists
     *         if not all nodes' values are smaller than v, else NULL 
     */
    avl_node<T>* minimum_geq(T &&v);

    /**
     * @brief returns the node with the smallest value greater than or equal to v
     * @param v value
     * @return avl_node<T>* the node with the smallest value greater than or equal to v, if it exists
     *         if not all nodes' values are smaller than v, else NULL 
     */
    avl_node<T>* minimum_geq(T &v);

    /**
     * @brief returns the node with the greatest value less than or equal to v
     * @param v value
     * @return avl_node<T>* the node with the greatest value less than or equal to v, if it exists
     *         if not all nodes' values are greater than v, else NULL 
     */
    avl_node<T>* maximum_leq(T &&v);

    /**
     * @brief returns the node with the greatest value less than or equal to v
     * @param v value
     * @return avl_node<T>* the node with the greatest value less than or equal to v, if it exists
     *         if not all nodes' values are greater than v, else NULL 
     */
    avl_node<T>* maximum_leq(T &v);

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
         * @param t an avl_tree
         * @param n an avl_node in t
         */
        avl_it(avl_tree<T> *t, avl_node<T> *n);

        /**
         * @brief deletes the avl_it
         */
        ~avl_it();
        
        /**
         * @brief checks if the iterator can iterate forward
         * @return whether it can iterate forward
         */
        bool has_next();

        /**
         * @brief checks if the iterator can iterate backward
         * @return whether it can iterate backward
         */
        bool has_prev();

        /**
         * @brief returns the value of the node, the iterator points to
         * @return the node, the iterator points to
         */
        avl_node<T>* current();

        /**
         * @brief iterates forward, has_next() must return true
         * @return the node the iterator points to after iterating forward
         */
        avl_node<T>* next();

        /**
         * @brief iterates forward, has_pred() must return true
         * @return the node the iterator points to after iterating backward
         */
        avl_node<T>* previous();

        /**
         * @brief points the iterator to the node n
         * @param n an avl_node in t
         */
        void set(avl_node<T> *n);
    };

    /**
     * @brief returns an iterator pointing to the node n
     * @param n an avl_node in the avl_tree
     * @return an iterator
     */
    avl_tree<T>::avl_it iterator(avl_node<T> *n);

    /**
     * @brief returns an iterator pointing to the minimum of the avl_tree if it is not empty
     * @return an iterator
     */
    avl_tree<T>::avl_it iterator();
};