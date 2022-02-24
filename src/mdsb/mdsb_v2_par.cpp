#include "../../include/mdsb/mdsb.hpp"
#include "../../extern/ips4o/include/ips4o.hpp"

template <typename T>
void mdsb<T>::reduce_te(te_tree_par<T> &T_e, ins_matr<T> &Q_ins) {
    int i_p = omp_get_thread_num();

    // temporary variables
    T p_j,q_j,d_j,q_y,d,steps;
    pair_list_node<T> *pln_IpA,*pln_Ip2A,*pln_Z,*pln_ZpA;
    pair_tree_node<T> *ptn_Y,*ptn_Y_nxt,*ptn_NEW,*ptn_J,*ptn_J_nxt;

    while (!T_e.empty()) {
        // Find the first unbalanced output interval and the a+1-st input interval connected to it in the permutation graph.
        te_node_par<T> *min = T_e.minimum();
        pln_IpA = std::get<0>(min->v);
        ptn_J = std::get<1>(min->v);
        ptn_J_nxt = std::get<2>(min->v);

        p_j = ptn_J->v.v.first;
        q_j = ptn_J->v.v.second;
        d_j = ptn_J_nxt->v.v.second-q_j;

        // d is the smalles integer, so that [q_j, q_j + d - 1] has 2 incoming edges in the permutation graph.
        d = pln_IpA->v.first - q_j;

        // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
        ptn_NEW = new pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d}));
        T_out[i_p].insert_node_in(ptn_NEW,ptn_J);

        // Check if [q_j + d, q_j + d_j - 1] is unbalanced.
        pln_Ip2A = is_unbalanced_par(pln_IpA,ptn_NEW,ptn_J_nxt);

        pln_ZpA = NULL;
        if (!(s[i_p] <= p_j + d && p_j + d < s[i_p+1])) {
            // If the new pair must be inserted in L_in[i_p_] of another thread i_p_ != i_p, find i_p_ with a binary search.
            T l = 0;
            T r = p-1;
            T m;
            while (l != r) {
                m = (l+r)/2+1;
                if (p_j + d >= s[m]) {
                    l = m;
                } else {
                    r = m-1;
                }
            }
            T i_p_ = l;

            Q_ins[i_p_][i_p].emplace(ins_pair<T>{&ptn_NEW->v,&ptn_J->v});
        } else {
            L_in[i_p].insert_after_node(&ptn_NEW->v,&ptn_J->v);

            // Check whether [q_y, q_y + d_y - 1] = [q_j + d, q_j + d_j - 1]
            if (p_j + d < q_j || q_j + d_j <= p_j + d) {
                // Else find the output interval [q_y, q_y + d_y - 1], to which [p_j + d, p_j + d_j - 1] is connected in the permutation graph.
                ptn_Y = T_out[i_p].maximum_leq(pair_list_node<T>(interv_pair<T>{0,p_j + d}));
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
                    // find the output interval starting after [q_y, q_y + d_y - 1]
                    ptn_Y_nxt = ptn_Y->nxt();

                    pln_ZpA = is_unbalanced_par(pln_Z,ptn_Y,ptn_Y_nxt);
                }
            }
        }

        if (pln_ZpA != NULL) {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] and [q_y, q_y + d_y - 1] are both new unbalanced output intervals
                if (pln_ZpA->v.first < pln_Ip2A->v.first) {
                    // and [q_y, q_y + d_y - 1] lies before [q_j + d, q_j + d_j - 1]
                    min->v = te_pair_par<T>{pln_Ip2A,ptn_NEW,ptn_J_nxt};
                    T_e.insert_or_update_in(te_pair_par<T>{pln_ZpA,ptn_Y,ptn_Y_nxt},min);
                } else {
                    // and [q_y, q_y + d_y - 1] lies after [q_j + d, q_j + d_j - 1]
                    if (T_e.size() == 1 || ptn_Y->v.v.second < std::get<1>(T_e.second_smallest()->v)->v.v.second) {
                        // and is the second unbalanced output interval
                        min->v = te_pair_par<T>{pln_ZpA,ptn_Y,ptn_Y_nxt};
                        T_e.insert_node_in(new te_node_par<T>(te_pair_par<T>{pln_Ip2A,ptn_NEW,ptn_J_nxt}),min);
                    } else {
                        // and is not the second unbalanced output interval
                        min->v = te_pair_par<T>{pln_Ip2A,ptn_NEW,ptn_J_nxt};
                        T_e.insert_or_update(te_pair_par<T>{pln_ZpA,ptn_Y,ptn_Y_nxt});
                    }
                }
            } else {
                // If [q_y, q_y + d_y - 1] is the only new unbalanced output interval
                if (T_e.size() == 1 || ptn_Y->v.v.second < std::get<1>(T_e.second_smallest()->v)->v.v.second) {
                    // and is the first unbalanced output interval
                    min->v = te_pair_par<T>{pln_ZpA,ptn_Y,ptn_Y_nxt};
                } else {
                    // and is not the first unbalanced output interval
                    T_e.remove_node(min);
                    T_e.insert_or_update(te_pair_par<T>{pln_ZpA,ptn_Y,ptn_Y_nxt});
                }
            }
        } else {
            if (pln_Ip2A != NULL) {
                // If [q_j + d, q_j + d_j - 1] is the only new unbalanced output interval
                min->v = te_pair_par<T>{pln_Ip2A,ptn_NEW,ptn_J_nxt};
            } else {
                // If there is no new unbalanced output interval
                T_e.remove_node(min);
            }
        }
    }
}

template <typename T>
void mdsb<T>::balance_v2_par() {
    /** @brief [0..p-1] stores queues with tuples (*p1,*p2);
     *        Q_ins[i] stores the tuples to insert into thread i's section [s[i]..s[i+1] */
    std::vector<std::vector<std::queue<ins_pair<T>>>> Q_ins;
    /**
     * @brief swap variable for Q_ins
     */
    std::vector<std::vector<std::queue<ins_pair<T>>>> Q_ins_swap;

    Q_ins = std::vector<std::vector<std::queue<ins_pair<T>>>>(p,std::vector<std::queue<ins_pair<T>>>(p));
    Q_ins_swap = std::vector<std::vector<std::queue<ins_pair<T>>>>(p,std::vector<std::queue<ins_pair<T>>>(p));

    bool done;

    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        /*
            contains pairs (pair_list_node<T> *p1, pair_tree_node<T> *p2, pair_tree_node<T> *p3),
            where p2 is associated with an output interval with at least 2a incoming edges in the permutation graph
            and p1 is the pair associated with the a+1-st input interval in the output interval associated with p2.
            p3 is associated with the output interval starting after the output interval associated with p2.
            The pairs are ordered by the starting position of the unbalanced output intervals associated with p2.
        */
        te_tree_par<T> T_e(
            [](auto n1, auto n2){return std::get<1>(n1)->v.v.second < std::get<1>(n2)->v.v.second;},
            [](auto n1, auto n2){return std::get<1>(n1)->v.v.second > std::get<1>(n2)->v.v.second;},
            [](auto n1, auto n2){return std::get<1>(n1)->v.v.second == std::get<1>(n2)->v.v.second;}
        );

        std::vector<te_node_par<T>> *nodes_te = new std::vector<te_node_par<T>>();
        nodes_te->reserve(L_in[i_p].size()/(2*a));

        // points to to the pair (p_i,q_i).
        typename pair_list<T>::dll_it it_inp = L_in[i_p].iterator();
        // points to the pair (p_j,q_j).
        typename pair_tree<T>::avl_it it_outp_cur = T_out[i_p].iterator();
        // points to the pair (p_{j'},q_{j'}), where q_j + d_j = q_{j'}.
        typename pair_tree<T>::avl_it it_outp_nxt = T_out[i_p].iterator(T_out[i_p].second_smallest());

        // temporary variables
        pair_list_node<T> *pln_IpA;

        // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
        bool stop = false;
        do {
            pln_IpA = is_unbalanced_par(it_inp.current(),it_outp_cur.current(),it_outp_nxt.current());

            // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
            if (pln_IpA != NULL) {
                it_inp.set(pln_IpA);
                nodes_te->push_back(te_node_par<T>(te_pair_par<T>{it_inp.current(),it_outp_cur.current(),it_outp_nxt.current()}));
            }

            // Find the next output interval with an incoming edge in the permutation graph and the first input interval connected to it.
            do {
                if (!it_outp_nxt.has_next()) {stop = true; break;}
                it_outp_cur.set(it_outp_nxt.current());
                it_outp_nxt.next();
                while (it_inp.current()->v.first < it_outp_cur.current()->v.v.second) {
                    if (!it_inp.has_next()) {stop = true; break;}
                    it_inp.next();
                }
            } while (!stop && it_inp.current()->v.first >= it_outp_nxt.current()->v.v.second);
        } while (!stop);

        if (!nodes_te->empty()) {
            T_e.insert_array(nodes_te);
            reduce_te(T_e,Q_ins);
            nodes_te = NULL;
        }

        #pragma omp barrier

        // temporary variables
        pair_list_node<T> *pln_I,*pln_Im1,*pln_Z,*pln_ZpA;
        pair_tree_node<T> *ptn_Y,*ptn_Y_nxt;
        //te_pair_par<T> tep_new;
        //te_node_par<T> *ten_find;

        do {
            #pragma omp single
            {
                std::swap(Q_ins,Q_ins_swap);
            }

            if (nodes_te == NULL) {
                nodes_te = new std::vector<te_node_par<T>>();
                nodes_te->reserve(L_in[i_p].size()/(2*a));
            }

            for (int i=0; i<p; i++) {
                while (!Q_ins_swap[i_p][i].empty()) {
                    pln_I = Q_ins_swap[i_p][i].front().first;
                    pln_Im1 = Q_ins_swap[i_p][i].front().second;
                    Q_ins_swap[i_p][i].pop();

                    L_in[i_p].insert_after_node(pln_I,pln_Im1);

                    // check if an output interval could have become unbalanced by inserting the new pair
                    ptn_Y = T_out[i_p].maximum_leq(pair_list_node<T>(interv_pair<T>{0,pln_I->v.first}));

                    // find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph
                    pln_Z = pln_I;
                    while (pln_Z->pr != NULL && pln_Z->pr->v.first >= ptn_Y->v.v.second) {
                        pln_Z = pln_Z->pr;
                    }

                    // find the output interval starting after [q_y, q_y + d_y - 1]
                    ptn_Y_nxt = ptn_Y->nxt();

                    pln_ZpA = is_unbalanced_par(pln_Z,ptn_Y,ptn_Y_nxt);
                    if (pln_ZpA != NULL) {
                        /*
                        if (T_e.empty()) {
                            T_e.insert_or_update(te_pair_par<T>(pln_ZpA,ptn_Y,ptn_Y_nxt));
                        } else {
                            tep_new = te_pair_par<T>(pln_ZpA,ptn_Y,ptn_Y_nxt);
                            ten_find = T_e.find(tep_new);
                            if (ptn_Y->v.v.second == std::get<1>(ten_find->v)->v.v.second && pln_ZpA->v.first < std::get<0>(ten_find->v)->v.first) {
                                std::get<0>(ten_find->v) = pln_ZpA;
                            } else {
                                T_e.insert_or_update_in(tep_new,ten_find);
                            }
                        }
                        */
                        nodes_te->push_back(te_node_par<T>(te_pair_par<T>(pln_ZpA,ptn_Y,ptn_Y_nxt)));
                    }
                }
            }

            /*
            if (!T_e.empty()) {
                reduce_te(T_e,Q_ins);
            }
            */
            if (!nodes_te->empty()) {
                ips4o::sort(nodes_te->begin(),nodes_te->end(),[](auto n1, auto n2){return std::get<0>(n1.v)->v.first < std::get<0>(n2.v)->v.first;});
                auto last = std::unique(nodes_te->begin(),nodes_te->end(),[](auto n1, auto n2){return std::get<1>(n1.v)->v.v.second == std::get<1>(n2.v)->v.v.second;});
                nodes_te->erase(last,nodes_te->end());
                T_e.insert_array(nodes_te);
                reduce_te(T_e,Q_ins);
                nodes_te = NULL;
            }
            
            #pragma omp barrier

            #pragma omp single
            {
                done = true;
                for (int i=0; i<p; i++) {
                    for (int j=0; j<p; j++) {
                        if (!Q_ins[i][j].empty()) {
                            done = false;
                            break;
                        }
                    }
                    if (!done) {
                        break;
                    }
                }
            }

            #pragma omp barrier

        } while (!done);
    }
}