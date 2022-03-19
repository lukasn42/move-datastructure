#include <mdsb.hpp>

template <typename T>
void mdsb<T>::balance_v2() {
    /* 
        contains pairs (pair_list_node<T> *p1, pair_list_node<T> *p2),
        where p2 is associated with an output interval with at least 2a incoming edges in the permutation graph
        and p1 is the pair associated with the a+1-st input interval in the output interval associated with p2.
        The pairs are ordered by the starting position of the unbalanced output intervals associated with p2.
    */
    te_tree<T> T_e(
        [](auto n1, auto n2){return n1.second->v.v.second < n2.second->v.v.second;},
        [](auto n1, auto n2){return n1.second->v.v.second > n2.second->v.v.second;},
        [](auto n1, auto n2){return n1.second->v.v.second == n2.second->v.v.second;}
    );
    
    std::vector<te_node<T>> *nodes_te = new std::vector<te_node<T>>();
    nodes_te->reserve(k/(2*a));

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
        i_ = 1;

        // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
        if (pln_IpA != NULL) {
            nodes_te->push_back(te_node<T>(te_pair<T>{pln_IpA,it_outp_cur.current()}));
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
    } while (!stop);

    // Build T_e from nodes_te.
    T_e.insert_array(nodes_te,0,nodes_te->size()-1);
    nodes_te = NULL;

    // temporary variables
    T p_j,q_j,d_j,d,q_y;
    pair_tree_node<T> *ptn_J,*ptn_NEW,*ptn_Y;
    pair_list_node<T> *pln_Ip2A,*pln_Z,*pln_ZpA;

    while (!T_e.empty()) {
        // Find the first unbalanced output interval and the a+1-st input interval connected to it in the permutation graph.
        te_node<T> *min = T_e.minimum();
        pln_IpA = min->v.first;
        ptn_J = min->v.second;

        p_j = ptn_J->v.v.first;
        q_j = ptn_J->v.v.second;
        d_j = interval_length_seq(&ptn_J->v);

        // d is the smalles integer, so that [q_j, q_j + d - 1] has 2 incoming edges in the permutation graph.
        d = pln_IpA->v.first - q_j;

        // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
        ptn_NEW = new_nodes[0].emplace_back(pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d})));
        L_in[0].insert_after_node(&ptn_NEW->v,&ptn_J->v);
        T_out[0].insert_node_in(ptn_NEW,ptn_J);

        // Check if [q_j + d, q_j + d_j - 1] is unbalanced.
        i_ = 1;
        pln_Ip2A = is_unbalanced(&pln_IpA,&i_,ptn_NEW);

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
            i_ = 1;
            while (i_ < 2*a && pln_Z->pr != NULL && pln_Z->pr->v.first >= q_y) {
                pln_Z = pln_Z->pr;
                i_++;
            }
            if (p_j + d < q_j || pln_Z->pr == NULL || pln_Z->pr->v.first < q_y) {
                pln_Z = &ptn_NEW->v;
                T i__ = i_;

                pln_ZpA = is_unbalanced(&pln_Z,&i_,ptn_Y);
                if (pln_ZpA != NULL && i__ > a+1 && pln_Z->sc->v.first < q_y + interval_length_seq(&ptn_Y->v)) {
                    pln_ZpA = NULL;
                }
            }
        }

        if (pln_ZpA != NULL) {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] and [q_y, q_y + d_y - 1] are both new unbalanced output intervals
                if (pln_ZpA->v.first < pln_Ip2A->v.first) {
                    // and [q_y, q_y + d_y - 1] lies before [q_j + d, q_j + d_j - 1]
                    min->v = te_pair<T>{pln_Ip2A,ptn_NEW};
                    T_e.insert_or_update_in(te_pair<T>{pln_ZpA,ptn_Y},min);
                } else {
                    // and [q_y, q_y + d_y - 1] lies after [q_j + d, q_j + d_j - 1]
                    if (T_e.size() == 1 || ptn_Y->v.v.second < T_e.second_smallest()->v.second->v.v.second) {
                        // and is the second unbalanced output interval
                        min->v = te_pair<T>{pln_ZpA,ptn_Y};
                        T_e.insert_or_update_in(te_pair<T>{pln_Ip2A,ptn_NEW},min);
                    } else {
                        // and is not the second unbalanced output interval
                        min->v = te_pair<T>{pln_Ip2A,ptn_NEW};
                        T_e.insert_or_update(te_pair<T>{pln_ZpA,ptn_Y});
                    }
                }
            } else {
                // If [q_y, q_y + d_y - 1] is the only new unbalanced output interval
                if (T_e.size() == 1 || ptn_Y->v.v.second < T_e.second_smallest()->v.second->v.v.second) {
                    // and is the first unbalanced output interval
                    min->v = te_pair<T>{pln_ZpA,ptn_Y};
                } else {
                    // and is not the first unbalanced output interval
                    T_e.remove_node(min);
                    T_e.insert_or_update(te_pair<T>{pln_ZpA,ptn_Y});
                }
            }
        } else {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] is the only new unbalanced output interval
                min->v = te_pair<T>{pln_Ip2A,ptn_NEW};
            } else {
                // If there is no new unbalanced output interval
                T_e.remove_node(min);
            }
        }
    }
}