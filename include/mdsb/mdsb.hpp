#pragma once

#include <queue>

#include <concurrentqueue.h>

#include <avl_tree.hpp>
#include <avl_tree.cpp>

#include <dl_list.hpp>
#include <dl_list.cpp>

#include <dg_io_nc.hpp>
#include <dg_io_nc.cpp>

template <typename T> class mds;

template <typename T> using interv_pair = std::pair<T,T>;
template <typename T> using interv_seq = std::vector<interv_pair<T>>;

// ############################# V2/3/4 #############################

template <typename T> using pair_list_node = dll_node<interv_pair<T>>;
template <typename T> using pair_list = dl_list<interv_pair<T>>;

template <typename T> using pair_tree_node = avl_node<pair_list_node<T>>;
template <typename T> using pair_tree = avl_tree<pair_list_node<T>>;

// ############################# V2 #############################

template <typename T> using te_pair = std::pair<pair_list_node<T>*,pair_tree_node<T>*>;
template <typename T> using te_node = avl_node<te_pair<T>>;
template <typename T> using te_tree = avl_tree<te_pair<T>>;

// ############################# V3/V4 PARALLEL #############################

template <typename T> using ins_pair = std::pair<pair_list_node<T>*,pair_list_node<T>*>;

// ############################# V3 PARALLEL #############################

template <typename T> using ins_matr_3 = std::vector<std::vector<std::queue<ins_pair<T>>>>;

// ############################# V4 PARALLEL #############################

template <typename T> using ins_matr_v4 = std::vector<std::vector<moodycamel::ConcurrentQueue<ins_pair<T>>>>;

/**
 * @brief builds a mds
 * @tparam T (integer) type of the interval starting positions
 */
template <typename T>
class mdsb {
    public:
    /**
     * @brief builds the move datastructure mds
     * @param mds a move datastructure that has not yet been built
     * @param I disjoint interval sequence
     * @param n n = p_{k-1} + d_{k-1}, k <= n
     * @param a balancing parameter, restricts size increase to the factor
     *          (1+1/(a-1)) and restricts move query runtime to 2a, 2 <= a
     * @param p number of threads to use
     * @param v version of the build method (1/2/3/4)
     * @param log enables log messages during build process
     * @param os output stream to write runtime and space usage to if log is enabled (default: NULL)
     */
    mdsb(mds<T> *mds, interv_seq<T> *I, T n, T a, int p, int v, bool log, std::ostream *os = NULL);

    /**
     * @brief deletes the mdsb
     */
    ~mdsb();

    /**
     * @brief prints messages for wrong pairs and unbalanced output intervals
     */
    inline void verify_correctness();

    // ############################# VARIABLES #############################

    protected:
    mds<T> *md; // pointer to the move datastructure to build
    T n; // maximum value, n = p_{k-1} + d_{k-1}, k <= n
    T k; // number of intervals in the balanced inteval sequence B_I, 0 < k
    T a; // balancing parameter, restricts size increase to the factor (1+1/(a-1)), 2 <= a
    int p; // number of threads to use

    // ############################# V1 #############################

    /**
     * @brief builds the move datastructure md
     * @param I disjoint interval sequence
     * @param log enables log messages
     * @param os output stream to write runtime and space usage to if log is enabled (default: NULL)
     */
    void build_v1(interv_seq<T> *I, bool log, std::ostream *os = NULL);

    // ############################# V2/V3/V4 #############################

    /** 
     * @brief [0..p-1] doubly linked lists; L_in[i_p] stores the pairs (p_i,q_i) in ascending order of p_i,
     *        where s[i_p] <= p_i < s[i_p+1] and i_p in [0..p-1]. L_in[0] ++ L_in[1] ++ ... ++ L_in[p-1] = I.
     */
    std::vector<pair_list<T>> L_in;
    /**
     * @brief [0..p-1] avl trees; T_out[i_p] stores nodes of lists in L_in in ascending order of q_i,
     *        for each pair (p_i,q_i), s[i_p] <= q_i < s[i_p+1] holds, with i_p in [0..p-1].
     */
    std::vector<pair_tree<T>> T_out;
    /**
     * @brief [0..p-1] section start positions in the range [0..n], 0 = s[0] < s[1] < ... < s[p-1] = n.
     *        Before building T_out, s is chosen so that |L_in[0]| ~ |L_in[1]| ~ ... ~ |L_in[p-1]| (see descr. of L_in).
     */
    std::vector<T> s;
    /**
     * @brief stores the nodes in L_in[0..p-1] and T_out[0..p-1] that they were initially created with
     */
    std::vector<std::vector<pair_tree_node<T>>*> nodes;
    /**
     * @brief [0..p-1] new_nodes[i_p] stores the newly created nodes in L_in[0..p-1] and T_out[0..p-1], that were created by thread i_p.
     */
    std::vector<dg_io_nc<pair_tree_node<T>>> new_nodes;

    /**
     * @brief builds the move datastructure md
     * @param I disjoint interval sequence
     * @param v version of the build method
     * @param log enables log messages
     * @param os output stream to write runtime and space usage to if log is enabled (default: NULL)
     */
    void build_v2_v3_v4(interv_seq<T> *I, int v, bool log, std::ostream *os = NULL);

    // ############################# V2/V3/V4 SEQUENTIAL/PARALLEL #############################

    /**
     * @brief builds L_in[0..p-1] and T_out[0..p-1] out of the disjoint interval sequence I
     * @param I disjoint interval sequence
     */
    void build_lin_tout(interv_seq<T> *I);

    /**
     * @brief inserts the pairs in L_in[0..p-1] into D_pair
     */
    void build_dpair();

    /**
     * @brief builds D_index
     */
    void build_dindex();

    /** 
     * @brief returns the length of the input/output interval starting at p_i/q_i
     * @param pln (p_i,q_i)
     * @return length of the input/output interval starting at p_i/q_i
     */
    inline T interval_length_seq(pair_list_node<T> *pln);

    /**
     * @brief checks if the output interval [q_j, q_j + d_j - 1] is unbalanced and iterates pln_IpI_ to the last output interval connected to it in the permutation graph
     * @param pln_IpI_ (p_{i+i_},q_{i+i_}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j)
     * @param ptn_J_nxt (p_{j'},q_{j'}), with q_j + d_j = q_{j'}
     * @param i_ 1 <= i_ <= 2a
     * @return (p_{i+a},q_{i+a}) if [q_j, q_j + d_j - 1] is unbalanced, else NULL
     */
    inline pair_list_node<T>* is_unbalanced(pair_list_node<T> **pln_IpI_, T *i_, pair_tree_node<T> *ptn_J, pair_tree_node<T> *ptn_J_nxt = NULL);

    // ############################# V2 #############################

    /**
     * @brief balances the disjoint interval sequence in L_in[0] and T_out[0]
     */
    void balance_v2();

    // ############################# V3 SEQUENTIAL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] and all unbalanced output intervals
     *        starting before or at q_u that have become unbalanced in the process
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval
     * @param q_u starting position of an output interval
     * @param p_cur starting position of the current input interval
     * @param i_ the current input interval is the i_-th input interval in [q_j, q_j + d_j - 1]
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* balance_upto_seq(pair_list_node<T> *pln_IpA, pair_tree_node<T> *ptn_J, T q_u, T p_cur, T *i_);

    /**
     * @brief balances the disjoint interval sequence in L_in[0] and T_out[0] sequentially
     */
    void balance_v3_seq();

    // ############################# V3 PARALLEL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] by inserting the newly created pair into T_out[i_p] and Q_ins[0..p-1][i_p]
     * @param Q_ins reference to Q_ins
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval starting in [s[i_p]..q_u]
     * @param ptn_J_next (p_jn,q_jn), [q_jn, q_jn + d_jn - 1] must be the first unbalanced output interval starting after [q_j, q_j + d_j - 1]
     * @param q_u starting position of an output interval starting in [s[i_p]..s[i_p+1]]
     * @param p_cur starting position of the current input interval
     * @param i_ the current input interval is the i_-th input interval in [q_j, q_j + d_j - 1]
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* balance_upto_par(ins_matr_3<T> &Q_ins, pair_list_node<T>* pln_IpA, pair_tree_node<T>* ptn_J, pair_tree_node<T>* ptn_J_nxt, T q_u, T p_cur, T *i_);

    /**
     * @brief balances the disjoint interval sequence in L_in[0..p-1] and T_out[0..p-1] in parallel
     */
    void balance_v3_par();

    // ############################# V4 PARALLEL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] by inserting the newly created pair into T_out[i_p] and Q_ins[0..p-1][i_p]
     * @param Q_ins reference to Q_ins
     * @param pln_IpA (p_{i+a},q_{i+a}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval starting in [s[i_p]..q_u]
     * @param ptn_J_next (p_jn,q_jn), [q_jn, q_jn + d_jn - 1] must be the first unbalanced output interval starting after [q_j, q_j + d_j - 1]
     * @param q_u starting position of an output interval starting in [s[i_p]..s[i_p+1]]
     * @param p_cur starting position of the current input interval
     * @param i_ the current input interval is the i_-th input interval in [q_j, q_j + d_j - 1]
     * @return the newly created pair (p_j+d,q_j+d)
     */
    inline pair_tree_node<T>* balance_upto_v4_par(ins_matr_v4<T> &Q_ins, pair_list_node<T> *pln_IpA, pair_tree_node<T> *ptn_J, pair_tree_node<T>* ptn_J_nxt, T q_u, T p_cur, T *i_);

    /**
     * @brief balances the disjoint interval sequence in L_in[0..p-1] and T_out[0..p-1] in parallel
     */
    void balance_v4_par();
};