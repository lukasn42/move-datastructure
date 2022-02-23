#include "../../include/mdsb/mdsb.hpp"

template <typename T>
void mdsb<T>::balance_v2_seq() {
    L_in[0].push_back_node(&T_out[0].maximum()->v);

    /* 
        contains pairs (pair_list_node<T> *p1, pair_list_node<T> *p2),
        where p2 is associated with an output interval with at least 2a incoming edges in the permutation graph
        and p1 is the pair associated with the a+1-st input interval in the output interval associated with p2.
        The pairs are ordered by the starting position of the unbalanced output intervals associated with p2.
    */
    te_tree_seq<T> T_e(
        [](auto n1, auto n2){return n1.second->v.v.second < n2.second->v.v.second;},
        [](auto n1, auto n2){return n1.second->v.v.second > n2.second->v.v.second;},
        [](auto n1, auto n2){return n1.second->v.v.second == n2.second->v.v.second;}
    );
    
    std::vector<te_node_seq<T>> *nodes_te = new std::vector<te_node_seq<T>>();
    nodes_te->reserve(k/10);

    // it_inp points to to the pair (p_i,q_i).
    typename pair_list<T>::dll_it it_inp = L_in[0].iterator();

    // it_outp points to the pair (p_j,q_j).
    typename pair_tree<T>::avl_it it_outp = T_out[0].iterator();

    // pointer to the pair (p_{i+2a},q_{i+2a})
    pair_list_node<T> *pln_IpA;

    // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph.
    bool stop = false;
    do {
        pln_IpA = is_unbalanced_seq(it_inp.current(),it_outp.current());

        // If [q_j, q_j + d_j - 1] is unbalanced, insert ((p_{i+2a},q_{i+2a}),(p_j,q_j)) into T_e.
        if (pln_IpA != NULL) {
            it_inp.set(pln_IpA);
            nodes_te->push_back(te_node_seq<T>(te_pair_seq<T>{it_inp.current(),it_outp.current()}));
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

    // Build T_e from nodes_te.
    T_e.insert_array(nodes_te);
    nodes_te = NULL;

    // temporary variables
    T p_j,q_j,d_j,d,q_y,steps;
    pair_tree_node<T> *ptn_J,*ptn_NEW,*ptn_Y;
    pair_list_node<T> *pln_Ip2A,*pln_Z,*pln_ZpA;

    while (!T_e.empty()) {
        // Find the first unbalanced output interval and the a+1-st input interval connected to it in the permutation graph.
        te_node_seq<T> *min = T_e.minimum();
        pln_IpA = min->v.first;
        ptn_J = min->v.second;

        p_j = ptn_J->v.v.first;
        q_j = ptn_J->v.v.second;
        d_j = interval_length_seq(&ptn_J->v);

        // d is the smalles integer, so that [q_j, q_j + d - 1] has 2 incoming edges in the permutation graph.
        d = pln_IpA->v.first - q_j;

        // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
        ptn_NEW = new pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d}));
        L_in[0].insert_after_node(&ptn_NEW->v,&ptn_J->v);
        T_out[0].insert_node_in(ptn_NEW,ptn_J);

        // Check if [q_j + d, q_j + d_j - 1] is unbalanced.
        pln_Ip2A = is_unbalanced_seq(pln_IpA,ptn_NEW);

        pln_ZpA = NULL;
        // If p_j + d lies in [q_j, q_j + d - 1] or [q_j + d, q_j + d_j - 1], [q_j + d, q_j + d_j - 1] is the only possibly new unbalanced output interval.
        if (p_j + d < q_j || q_j + d_j <= p_j + d) {
            // Else find the output interval [q_y, q_y + d_y - 1], to which [p_j + d, p_j + d_j - 1] is connected in the permutation graph.
            ptn_Y = T_out[0].maximum_leq(pair_list_node<T>(interv_pair<T>{0,p_j + d}));
            q_y = ptn_Y->v.v.second;

            /*
                Find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph.
                Case 1: [p_z, p_z + d_z - 1] lies before q_j.
                    Then [q_y, q_y + d_y - 1] does as well and because [q_y, q_y + d_y - 1] was not unbalanced before [q_j + d, q_j + d_j - 1] has been
                    created, [p_z, p_z + d_z - 1] can be reached by iterating backwards in L_in at most 2a-1 steps starting from (p_j + d, q_j + d).
                Case 2: [p_z, p_z + d_z - 1] lies after q_j + d_j.
                    Case 2.1: [p_z, p_z + d_z - 1] is found by iterating backwards at most 2a-1 steps in L_in starting from (p_j + d, q_j + d).
                        Then check if [q_y, q_y + d_y - 1] is unbalanced and insert ((p_{z+a},q_{z+a}),(p_y,q_y)) into T_e if it is.
                    Case 2.2: [p_z, p_z + d_z - 1] is not found by iterating backwards at most 2a-1 steps in L_in starting from (p_j + d, q_j + d).
                        Then there are at least 2a input intervals connected to [q_y, q_y + d_y - 1] in the permutation graph, hence there exists a pair
                        ((p_{x+a},q_{x+a}),(p_y,q_y)) in T_e, where [p_{x+a}, p_{x+a} + d_{x+a} - 1] was the first input interval connected to
                        [q_y, q_y + d_y - 1] in the permutation graph before [q_j + d, q_j + d_j - 1] has been created. It still is, because if it
                        was not, [p_z, p_z + d_z - 1] would have been found in a < 2a-1 steps, hence ((p_{x+a},q_{x+a}),(p_y,q_y)) still is a valid pair in T_e.
            */
            pln_Z = &ptn_NEW->v;
            steps = 0;
            while (steps < 2*a-1 && pln_Z->pr != NULL && pln_Z->pr->v.first >= q_y) {
                pln_Z = pln_Z->pr;
                steps++;
            }

            if (p_j + d < q_j || pln_Z->pr == NULL || pln_Z->pr->v.first < q_y) {
                pln_ZpA = is_unbalanced_seq(pln_Z,ptn_Y);
            }
        }

        if (pln_ZpA != NULL) {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] and [q_y, q_y + d_y - 1] are both new unbalanced output intervals
                if (pln_ZpA->v.first < pln_Ip2A->v.first) {
                    // and [q_y, q_y + d_y - 1] lies before [q_j + d, q_j + d_j - 1]
                    min->v = te_pair_seq<T>{pln_Ip2A,ptn_NEW};
                    T_e.insert_or_update_in(te_pair_seq<T>{pln_ZpA,ptn_Y},min);
                } else {
                    // and [q_y, q_y + d_y - 1] lies after [q_j + d, q_j + d_j - 1]
                    if (T_e.size() == 1 || ptn_Y->v.v.second < T_e.second_smallest()->v.second->v.v.second) {
                        // and is the second unbalanced output interval
                        min->v = te_pair_seq<T>{pln_ZpA,ptn_Y};
                        T_e.insert_node_in(new te_node_seq<T>(te_pair_seq<T>{pln_Ip2A,ptn_NEW}),min);
                    } else {
                        // and is not the second unbalanced output interval
                        min->v = te_pair_seq<T>{pln_Ip2A,ptn_NEW};
                        T_e.insert_or_update(te_pair_seq<T>{pln_ZpA,ptn_Y});
                    }
                }
            } else {
                // If [q_y, q_y + d_y - 1] is the only new unbalanced output interval
                if (T_e.size() == 1 || ptn_Y->v.v.second < T_e.second_smallest()->v.second->v.v.second) {
                    // and is the first unbalanced output interval
                    min->v = te_pair_seq<T>{pln_ZpA,ptn_Y};
                } else {
                    // and is not the first unbalanced output interval
                    T_e.remove_node(min);
                    T_e.insert_or_update(te_pair_seq<T>{pln_ZpA,ptn_Y});
                }
            }
        } else {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] is the only new unbalanced output interval
                min->v = te_pair_seq<T>{pln_Ip2A,ptn_NEW};
            } else {
                // If there is no new unbalanced output interval
                T_e.remove_node(min);
            }
        }
    }

    L_in[0].remove_node(L_in[0].tail());
}