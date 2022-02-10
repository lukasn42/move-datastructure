#include "../../include/mdsb/mdsb.hpp"

template <typename T>
pair_tree_node<T>* mdsb<T>::balance_upto_seq(pair_list_node<T>* pln_IpB, pair_tree_node<T>* ptn_J, T q_u) {
    T p_j = ptn_J->v.v.first;
    T q_j = ptn_J->v.v.second;
    T d_j = interval_length_seq(&ptn_J->v);
    
    // d = p_{i+b} - q_j is the maximum integer, so that [q_j, q_j + d - 1] has b incoming edges in the permutation graph.
    T d = pln_IpB->v.first - q_j;

    // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
    pair_tree_node<T> *ptn_NEW = new pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d}));
    T_out[0].insert_node_in(ptn_NEW,ptn_J);
    L_in[0].insert_after_node(&ptn_NEW->v,&ptn_J->v);

    /*
    The output interval [q_y, q_y + d_y - 1], to which [p_j + d, p_j + d_j - 1] is connected in the permutation graph, can be unbalanced.

    Case 1: If p_j + d lies in [q_u, q_u + d_u - 1], then [q_u, q_u + d_u - 1] = [q_y, q_y + d_y - 1]. p_j + d = q_u obviously is not possible.
            In general, once balance_upto creates a new input interval starting within [q_u, q_u + d_u - 1], the sequence of recursive calls ends.
            Because of that, [q_u, q_u + d_u - 1] had b incoming edges in the permutation graph before [p_j + d, p_j + d_j - 1] was created.
            Therefore after that [q_y, q_y + d_y - 1] has b+1 < a incoming edges in the permutation graph, hence it is balanced.

    Case 2: Else if p_j + d >= q_u + d_u, then it is irrelevant if [q_y, q_y + d_y - 1] is unbalanced, because only output intervals upto [q_u, q_u + d_u - 1] must be balanced.

    Case 3: Else if p_j + d lies in [q_j, q_j + d - 1] or [q_j + d, q_j + d_j - 1], then [q_y, q_y + d_y - 1] equals one of them.
            Because they have at most b+1 < a and a-b+1 < a incoming edges in the permutation graph, [q_y, q_y + d_y - 1] cannot be unbalanced.
    
    Case 4: Else balance [q_y, q_y + d_y - 1], if it is unbalanced.
    */
    // check if case 4 holds
    if (p_j + d < q_u && !(q_j <= p_j + d && p_j + d < q_j + d_j)) {
        // find [q_y, q_y + d_y - 1]
        pair_tree_node<T> *ptn_Y = T_out[0].maximum_leq(pair_list_node<T>(interv_pair<T>{0,p_j + d}));

        // find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph
        pair_list_node<T> *pln_Z = &ptn_NEW->v;
        while (pln_Z->pr != NULL && pln_Z->pr->v.first >= ptn_Y->v.v.second) {
            pln_Z = pln_Z->pr;
        }

        pair_list_node<T> *pln_ZpB = is_unbalanced_seq(pln_Z,ptn_Y);

        if (pln_ZpB != NULL) {
            balance_upto_seq(pln_ZpB,ptn_Y,q_u);
        }
    }

    return ptn_NEW;
}

template <typename T>
void mdsb<T>::balance_v3_seq() {
    L_in[0].push_back_node(&T_out[0].maximum()->v);

    // it_inp points to to the pair (p_i,q_i).
    typename pair_list<T>::dll_it it_inp = L_in[0].iterator(L_in[0].head());

    // it_outp points to the pair (p_j,q_j).
    typename pair_tree<T>::avl_it it_outp = T_out[0].iterator(T_out[0].minimum());

    // pointer to the pair (p_{i+b},q_{i+b}), return value of is_unbalanced_seq((p_i,q_i),(p_j,q_j))
    pair_list_node<T> *pln_IpB;

    bool stop = false;

    // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
    // and all output intervals starting before [q_j, q_j + d_j - 1] are balanced.
    do {
        // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
        pln_IpB = is_unbalanced_seq(it_inp.current(),it_outp.current());
        if (pln_IpB != NULL) {
            it_inp.set(pln_IpB);
            it_outp.set(balance_upto_seq(it_inp.current(),it_outp.current(),it_outp.current()->v.v.second));
            continue;
        }

        // Find the next output interval with an incoming edge in the permutation graph and the first input interval connected to it.
        do {
            if (!it_outp.has_next()) {stop = true; break;}
            it_outp.next();
            while (it_inp.current()->v.first < it_outp.current()->v.v.second) {
                it_inp.next();
                if (!it_inp.has_next()) {stop = true; break;}
            }
        } while (!stop && it_inp.current()->v.first >= it_outp.current()->v.v.second + interval_length_seq(&it_outp.current()->v));
    } while (!stop);

    L_in[0].remove_node(L_in[0].tail());
}