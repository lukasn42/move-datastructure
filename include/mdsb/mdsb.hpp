#pragma once

#include <mutex>
#include <queue>

#include "../misc/avl_tree.hpp"
#include "../../src/misc/avl_tree.cpp"

#include "../misc/dl_list.hpp"
#include "../../src/misc/dl_list.cpp"

template <typename T> class mds;

template <typename T> using interv_pair = std::pair<T,T>;
template <typename T> using interv_seq = std::vector<interv_pair<T>>;

template <typename T> using pair_list_node = dll_node<interv_pair<T>>;
template <typename T> using pair_list = dl_list<interv_pair<T>>;

template <typename T> using pair_tree_node = avl_node<pair_list_node<T>>;
template <typename T> using pair_tree = avl_tree<pair_list_node<T>>;

template <typename T> using ins_pair = std::pair<pair_list_node<T>*,pair_list_node<T>*>;
template <typename T> using ins_matr = std::vector<std::vector<std::queue<ins_pair<T>>>>;

template <typename T> using te_pair_seq = std::pair<pair_list_node<T>*,pair_tree_node<T>*>;
template <typename T> using te_node_seq = avl_node<te_pair_seq<T>>;
template <typename T> using te_tree_seq = avl_tree<te_pair_seq<T>>;

template <typename T> using te_pair_par = std::tuple<pair_list_node<T>*,pair_tree_node<T>*,pair_tree_node<T>*>;
template <typename T> using te_node_par = avl_node<te_pair_par<T>>;
template <typename T> using te_tree_par = avl_tree<te_pair_par<T>>;

/**
 * @brief builds a mds
 * @tparam T (integer) type of the interval starting positions
 */
template <typename T>
class mdsb {
    public:
    /**
     * @brief builds the mds mds
     * 
     * @param mds a mds that has not yet been built
     * @param I interval sequence
     * @param n n = p_{k-1} + d_{k-1}, k <= n
     * @param a minimum number of incoming edges in the permutation graph, an output interval must have, to get cut, 4 <= a
     * @param b number of remaining incoming edges after an output interval is cut, 2 <= b < a-1
     * @param p number of threads to use
     * @param v version of the build method (1/2/3)
     * @param log enables log messages during build process
     */
    mdsb<T>(mds<T> *mds, interv_seq<T> *I, T n, T a, T b, int p, int v, bool log);

    /**
     * @brief deletes the mdsb
     */
    ~mdsb<T>();

    // ############################# VARIABLES #############################

    protected:
    mds<T> *md; // pointer to the move datastructure to build
    T n; // maximum value, n = p_{k-1} + d_{k-1}, k <= n
    T k; // number of intervals in the balanced inteval sequence B_I, 0 < k
    T a; // minimum number of incoming edges in the permutation graph, an output interval must have, to get cut, 4 <= a (default = 4)
    T b; // number of remaining incoming edges after an output interval gets cut, 2 <= b < a-1 (default = 2)
    int p; // number of threads to use

    // ############################# V1 #############################

    /**
     * @brief builds the move datastructure md
     * @param I interval sequence
     */
    void build_v1(interv_seq<T> *I, bool log);

    // ############################# V2/V3 #############################

    /** 
     * @brief [0..p-1] doubly linked lists; L_in[i_t] stores the pairs (p_i,q_i) in ascending order of p_i,
     *        where s[i_t+1] <= p_i < s[i_t+1] and i_t in [0..p-1]. L_in[0] ++ L_in[1] ++ ... ++ L_in[p-1] = I.
     */
    std::vector<pair_list<T>> L_in;
    /**
     * @brief [0..p-1] avl trees; T_out[i_t] stores doubly linked list nodes of lists in L_in, for whiches pairs
     *        (p_i,q_i) s[i_t+1] <= q_i < s[i_t+1] holds, in ascending order of q_i, with i_t in [0..p-1].
     */
    std::vector<pair_tree<T>> T_out;
    /**
     * @brief [0..p-1] section start positions in the range [0..n], 0 = s[0] < s[1] < ... < s[p-1] = n.
     *        Before building T_out, q is chosen so that |T_out[0]| ~ |T_out[1]| ~ ... ~ |T_out[p-1]| (see descr. of T_out).
     */
    std::vector<T> s;

    /**
     * @brief builds the move datastructure md
     * @param I interval sequence
     * @param v version
     */
    void build_v2_v3(interv_seq<T> *I, int v, bool log);

    // ############################# V2/V3 SEQUENTIAL/PARALLEL #############################

    /**
     * @brief builds L_in[0..p-1] and T_out[0..p-1] out of the interval sequence I
     * @param I interval sequence
     */
    void build_lin_tout(interv_seq<T> *I);

    /**
     * @brief inserts the pairs in L_in into D_pair
     */
    void build_dpair();

    /**
     * @brief deletes L_in and T_out
     */
    void delete_lin_tout();

    /**
     * @brief builds D_index
     */
    void build_dindex();

    /** 
     * @brief returns the length of the input/output interval starting at p_i/q_i
     * @param nodePair pointer to the pair (p_i,q_i)
     * @return length of the input/output interval starting at p_i/q_i
     */
    static T interval_length_seq(pair_list_node<T>* nodePair);

    /**
     * @brief checks if [q_j, q_j + d_j - 1] is unbalanced and returns the b+1-th input interval starting in it
     * @param pln_I (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval starting in [q_j, q_j + d_j - 1]
     * @param ptn_J (p_j,q_j)
     * @return (p_{i+b},q_{i+b}) if [q_j, q_j + d_j - 1] is unbalanced, else NULL
     */
    inline pair_list_node<T>* is_unbalanced_seq(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J);

    // ############################# V2 SEQUENTIAL #############################

    /**
     * @brief balances the interval sequence in L_in[0] and T_out[0]
     */
    void balance_v2_seq();

    // ############################# V2 PARALLEL #############################

    /*
     * @brief reduces T_e by inserting pairs into L_in[i_p] and Q_ins[][]
     */
    void reduce_te(te_tree_par<T> &T_e, ins_matr<T> &Q_ins);

    /**
     * @brief balances the interval sequence in L_in[0] and T_out[0]
     */
    void balance_v2_par();

    // ############################# V3 SEQUENTIAL #############################

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] and all unbalanced output intervals
     *        starting before or at q_u that have become unbalanced in the process
     * @param pln_IpB (p_{i+b},q_{i+b}), [p_i, p_i + d_i - 1] must be the first input interval starting in [q_j, q_j + d_j - 1]
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval
     * @param q_u starting position of an output interval
     */
    inline pair_tree_node<T>* balance_upto_seq(pair_list_node<T>* pln_IpB, pair_tree_node<T>* ptn_J, T q_u);

    /**
     * @brief balances the interval sequence in L_in[0] and T_out[0] sequentially
     */
    void balance_v3_seq();

    // ############################# V3 PARALLEL #############################

    /**
     * @brief checks if an output interval is unbalanced and returns the b+1-th input interval starting in it
     * @param pln_I (p_i,q_i), [p_i, p_i + d_i - 1] must be the first input interval starting in [q_j, q_j + d_j - 1]
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1]
     * @param ptn_J_nxt (p_{j+1},q_{j+1})
     * @return (p_{i+b},q_{i+b}) if [q_j, q_j + d_j - 1] is unbalanced, else NULL
     */
    inline pair_list_node<T>* is_unbalanced_par(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J, pair_tree_node<T>* ptn_J_nxt);

    /**
     * @brief balances the output interval [q_j, q_j + d_j - 1] by inserting the newly created pair into T_out and Q_ins
     * @param Q_ins insert queues
     * @param pln_IpB (p_{i+b},q_{i+b}), [p_i, p_i + d_i - 1] must be the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
     * @param ptn_J (p_j,q_j), [q_j, q_j + d_j - 1] must be the first unbalanced output interval starting in [s[i_t]..q_u]
     * @param q_u starting position of an output interval starting in [s[i_t]..s[i_t+1]]
     * @return the newly created pair
     */
    inline pair_tree_node<T>* insert_pair(ins_matr<T> &Q_ins, pair_list_node<T>* pln_IpB, pair_tree_node<T>* ptn_J, T q_u);

    /**
     * @brief balances the  interval sequence in L_in[0..p-1] and T_out[0..p-1] in parallel
     */
    void balance_v3_par();
};