#include "../../include/mdsb/mdsb.hpp"

template <typename T>
pair_tree_node<T>* mdsb<T>::balance_upto_seq(pair_list_node<T> *pln_IpA, pair_tree_node<T> *ptn_J, T q_u, T p_cur, T *i_) {
    T p_j = ptn_J->v.v.first;
    T q_j = ptn_J->v.v.second;
    T d_j = interval_length_seq(&ptn_J->v);
    
    // d = p_{i+2a} - q_j is the maximum integer, so that [q_j, q_j + d - 1] has a incoming edges in the permutation graph.
    T d = pln_IpA->v.first - q_j;

    // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
    pair_tree_node<T> *ptn_NEW = new_nodes[0].emplace_back(pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d})));
    T_out[0].insert_node_in(ptn_NEW,ptn_J);
    L_in[0].insert_after_node(&ptn_NEW->v,&ptn_J->v);

    /*
    The output interval [q_y, q_y + d_y - 1], to which [p_j + d, p_j + d_j - 1] is connected in the permutation graph, can be unbalanced.

    Case 1: If p_j + d lies in [q_u, q_u + d_u - 1], then [q_u, q_u + d_u - 1] = [q_y, q_y + d_y - 1]. p_j + d = q_u obviously is not possible.
            In general, once balance_upto creates a new input interval starting within [q_u, q_u + d_u - 1], the sequence of recursive calls ends.
            Because of that, [q_u, q_u + d_u - 1] had a incoming edges in the permutation graph before [p_j + d, p_j + d_j - 1] was created.
            Therefore after that [q_y, q_y + d_y - 1] has a+1 < 2a incoming edges in the permutation graph, hence it is balanced.

    Case 2: Else if p_j + d >= q_u + d_u, then it is irrelevant if [q_y, q_y + d_y - 1] is unbalanced, because only output intervals upto [q_u, q_u + d_u - 1] must be balanced.

    Case 3: Else if p_j + d lies in [q_j, q_j + d - 1] or [q_j + d, q_j + d_j - 1], then [q_y, q_y + d_y - 1] equals one of them.
            Because they have at most a+1 < 2a and 2a-a+1 < 2a incoming edges in the permutation graph, [q_y, q_y + d_y - 1] cannot be unbalanced.
    
    Case 4: Else balance [q_y, q_y + d_y - 1], if it is unbalanced.
    */
    // check if case 4 holds
    if (p_j + d < q_u) {
        if (p_j + d < q_j || q_j + d_j <= p_j + d) {
            // find [q_y, q_y + d_y - 1]
            pair_tree_node<T> *ptn_Y = T_out[0].maximum_leq(pair_list_node<T>(interv_pair<T>{0,p_j + d}));

            // find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph
            pair_list_node<T> *pln_Z = &ptn_NEW->v;
            T i__ = 1;
            while (pln_Z->pr != NULL && pln_Z->pr->v.first >= ptn_Y->v.v.second) {
                pln_Z = pln_Z->pr;
                i__++;
            }
            pln_Z = &ptn_NEW->v;

            pair_list_node<T> *pln_ZpA = is_unbalanced(&pln_Z,&i__,ptn_Y);
            if (pln_ZpA != NULL) {
                balance_upto_seq(pln_ZpA,ptn_Y,q_u,p_cur,i_);
            }
        }
    } else if (p_j + d < p_cur) {
        (*i_)++;
    }

    return ptn_NEW;
}

template <typename T>
void mdsb<T>::balance_v3_seq() {

    // points to to the pair (p_i,q_i).
    pair_list_node<T> *pln_I = L_in[0].head();
    // points to the pair (p_j,q_j).
    typename pair_tree<T>::avl_it it_outp_cur = T_out[0].iterator();
    // points to the pair (p_{j'},q_{j'}), where q_j + d_j = q_{j'}.
    typename pair_tree<T>::avl_it it_outp_nxt = T_out[0].iterator(T_out[0].second_smallest());

    // temporary variables
    pair_list_node<T> *pln_IpA;
    T i_ = 1;

    // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
    bool stop = false;
    do {
        pln_IpA = is_unbalanced(&pln_I,&i_,it_outp_cur.current(),it_outp_nxt.current());

        // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
        if (pln_IpA != NULL) {
            it_outp_cur.set(balance_upto_seq(pln_IpA,it_outp_cur.current(),it_outp_cur.current()->v.v.second,pln_I->v.first,&i_));
            continue;
        }

        // Find the next output interval with an incoming edge in the permutation graph and the first input interval connected to it.
        do {
            if (!it_outp_nxt.has_next()) {stop = true; break;}
            it_outp_cur.set(it_outp_nxt.current());
            it_outp_nxt.next();
            while (pln_I->v.first < it_outp_cur.current()->v.v.second) {
                if (pln_I->sc == NULL) {stop = true; break;}
                pln_I = pln_I->sc;
            }
        } while (!stop && pln_I->v.first >= it_outp_nxt.current()->v.v.second);
        i_ = 1;
    } while (!stop);
}