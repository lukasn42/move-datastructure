#include "../../include/mdsb/mdsb.hpp"

template <typename T>
pair_tree_node<T>* mdsb<T>::insert_pair(ins_matr<T> &Q_ins, pair_list_node<T> *pln_IpA, pair_tree_node<T> *ptn_J, T q_u) {
    int i_p = omp_get_thread_num();

    T p_j = ptn_J->v.v.first;
    T q_j = ptn_J->v.v.second;

    // d = p_{i+2a} - q_j is the maximum integer, so that [q_j, q_j + d - 1] has a incoming edges in the permutation graph.
    T d = pln_IpA->v.first - q_j;

    // Create the pair (p_j + d, q_j + d), which creates two new input intervals [p_j, p_j + d - 1] and [p_j + d, p_j + d_j - 1].
    pair_tree_node<T> *ptn_NEW = new pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{p_j + d, q_j + d}));
    T_out[i_p].insert_node_in(ptn_NEW,ptn_J);

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
        // Else insert it in L_in[i_p].
        L_in[i_p].insert_after_node(&ptn_NEW->v,&ptn_J->v);

        if (p_j + d < q_u) {
            pair_tree_node<T> *ptn_Y = T_out[i_p].maximum_leq(pair_list_node<T>(interv_pair<T>{0,p_j + d}));

            // find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph
            pair_list_node<T> *pln_Z = &ptn_NEW->v;
            while (pln_Z->pr != NULL && pln_Z->pr->v.first >= ptn_Y->v.v.second) {
                pln_Z = pln_Z->pr;
            }

            // find the output interval starting after [q_y, q_y + d_y - 1]
            pair_tree_node<T> *ptn_Y_nxt = ptn_Y->nxt();

            pair_list_node<T> *pln_ZpA = is_unbalanced_par(pln_Z,ptn_Y,ptn_Y_nxt);
            if (pln_ZpA != NULL) {
                insert_pair(Q_ins,pln_ZpA,ptn_Y,q_u);
            }
        }
    }
    return ptn_NEW;
}

template <typename T>
void mdsb<T>::balance_v3_par() {
    /** @brief [0..p-1] stores queues with tuples (*p1,*p2);
     *        Q_ins[i] stores the tuples to insert into thread i's section [s[i]..s[i+1] */
    ins_matr<T> Q_ins;
    /** @brief swap variable for Q_ins */
    ins_matr<T> Q_ins_swap;

    Q_ins = ins_matr<T>(p,std::vector<std::queue<ins_pair<T>>>(p));
    Q_ins_swap = ins_matr<T>(p,std::vector<std::queue<ins_pair<T>>>(p));

    bool done;

    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        // points to to the pair (p_i,q_i).
        typename pair_list<T>::dll_it it_inp = L_in[i_p].iterator();
        // points to the pair (p_j,q_j).
        typename pair_tree<T>::avl_it it_outp_cur = T_out[i_p].iterator();
        // points to the pair (p_{j'},q_{j'}), where q_j + d_j = q_{j'}.
        typename pair_tree<T>::avl_it it_outp_nxt = T_out[i_p].iterator(T_out[i_p].second_smallest());

        // temporary variables
        pair_list_node<T> *pln_IpA;

        // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
        // and all output intervals starting before [q_j, q_j + d_j - 1] are balanced.
        bool stop = false;
        do {
            pln_IpA = is_unbalanced_par(it_inp.current(),it_outp_cur.current(),it_outp_nxt.current());

            // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
            if (pln_IpA != NULL) {
                it_outp_cur.set(insert_pair(Q_ins,pln_IpA,it_outp_cur.current(),it_outp_cur.current()->v.v.second));
                it_inp.set(pln_IpA);
                continue;
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

        // temporary variables
        pair_list_node<T> *pln_I,*pln_Im1,*pln_Z,*pln_ZpA;
        pair_tree_node<T> *ptn_Y,*ptn_Y_nxt;

        do {
            #pragma omp single
            {
                std::swap(Q_ins,Q_ins_swap);
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
                        insert_pair(Q_ins,pln_ZpA,ptn_Y,n);
                    }
                }
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