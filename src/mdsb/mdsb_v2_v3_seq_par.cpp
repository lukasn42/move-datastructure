#include "../../extern/ips4o/include/ips4o.hpp"

#include "../../include/mdsb/mdsb.hpp"

template <typename T>
void mdsb<T>::build_lin_tout(interv_seq<T> *I) {
    L_in = std::vector<pair_list<T>>(p);

    T_out = std::vector<pair_tree<T>>(p,
        pair_tree<T>(
            [](auto n1, auto n2){return n1.v.second < n2.v.second;},
            [](auto n1, auto n2){return n1.v.second > n2.v.second;},
            [](auto n1, auto n2){return n1.v.second == n2.v.second;}
        )
    );

    // [0..p-1] nodes_tout[i_p] stores arrays to build T_out[i_p] out of.
    std::vector<std::vector<pair_tree_node<T>>*> nodes_tout(p);

    {
        // [0..p-1] x[i] stores the cumulative length of the sections 0..i-1
        std::vector<T> x(p+1);
        x[0] = 0;
        x[1] = 1+(k-1)/p; // length of the sections 0..p-1
        x[p] = k;
        for (int i=2; i<p; i++) {
            x[i] = x[i-1]+x[1];
        }

        /* section starting positions u[0..p-1] in I with 0 = u[0] < u[1] < ... < u[p] = k, for each pair
           (p_i,q_i) in I[u[j]..u[j+1]-1], s[j] <= p_i < s[j+1] holds, with i in [0..k-1] and j in [0..p-1] */
        std::vector<T> u;
        
        std::vector<T> pi_m1(0);
        {
            // create identity permutation pi of [0..k-1]
            std::vector<T> pi(k);
            #pragma omp parallel for num_threads(p)
            for (T i=0; i<k; i++) {
                pi[i] = i;
            }

            // sort pi by q_{pi[i]}
            auto comp = [I](T i1, T i2){return I->at(i1).second < I->at(i2).second;};
            if (p > 1) {
                ips4o::parallel::sort(pi.begin(),pi.end(),comp);
            } else {
                ips4o::sort(pi.begin(),pi.end(),comp);
            }

            s = std::vector<T>(p+1);
            s[0] = 0;
            s[p] = n;
            for (int i=1; i<p; i++) {
                s[i] = I->at(pi[x[i]]).second;
            }

            u = std::vector<T>(p+1);
            u[0] = 0;
            u[p] = k;
            #pragma omp parallel num_threads(p)
            {
                int i_p = omp_get_thread_num();

                // find first input interval [p_i, p_i + d_i -1] starting at or after s[i_p]
                T l = 0;
                T r = k-1;
                T m;
                while (l != r) {
                    m = (l+r)/2;
                    if (I->at(m).first >= s[i_p]) {
                        r = m;
                    } else {
                        l = m+1;
                    }
                }
                u[i_p] = l;
            }

            // calculate pi^{-1}
            pi_m1.resize(k);
            #pragma omp parallel for num_threads(p)
            for (T i=0; i<k; i++) {
                pi_m1[pi[i]] = i;
            }
        }

        // create nodes_tout
        #pragma omp parallel for num_threads(p)
        for (int i=0; i<p; i++) {
            nodes_tout[i] = new std::vector<pair_tree_node<T>>(x[i+1]-x[i]);
        }

        // build L_in[] and nodes_tout[] according to pi^{-1}
        T px1 = p*x[1];
        #pragma omp parallel num_threads(p)
        {
            int i_p = omp_get_thread_num();

            T l = u[i_p];
            T r = u[i_p+1];

            int i_1; // index in nodes_tout[] to insert the current pair in
            T i_2; // index in nodes_tout[i_1][] to insert the current pair in
            for (T i=l; i<r; i++) {
                i_1 = (pi_m1[i]*p)/px1;
                i_2 = pi_m1[i]-x[i_1];
                nodes_tout[i_1]->at(i_2).v.v = I->at(i);
                L_in[i_p].push_back_node(&nodes_tout[i_1]->at(i_2).v);
            }
        }
    }
    
    delete I;

    // build T_out[] from nodes_tout[]
    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        T_out[i_p].insert_array(nodes_tout[i_p]);
        T_out[i_p].insert_or_update_in(pair_list_node<T>(interv_pair<T>{s[i_p+1],s[i_p+1]}),T_out[i_p].maximum());
    }
}

template <typename T>
void mdsb<T>::build_dpair() {
    /* section start positions u[0..p-1] in D_pair with 0 = u[0] < u[1] < ... < u[p] = k, for each pair
       (p_i,q_i) in D_pair[u[j]..u[j+1]-1], s[l] <= p_i < s[l+1] holds, with i in [0..k-1], j,l in [0..p-1]
    */
    std::vector<T> u(p+1);
    u[0] = 0;
    for (int i=0; i<p; i++) {
        u[i+1] = u[i]+L_in[i].size();
    }
    
    k = u[p];
    md->k = k;
    md->D_pair.resize(k+1);
    md->D_pair[k] = interv_pair<T>{n,n};

    // Place the pairs in L_in[i_p] into D_pair[u[i_p]..u[i_p+1]-1] for each i_p in [0..p-1].
    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        T l = u[i_p];
        T r = u[i_p+1]-1;

        typename pair_list<T>::dll_it it = L_in[i_p].iterator();

        for (T i=l; i<=r; i++) {
            md->D_pair[i] = it.current()->v;
            it.next();
        }
    }
}

template <typename T>
void mdsb<T>::delete_lin_tout() {
    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        L_in[i_p].disconnect_nodes();
        T_out[i_p].delete_nodes();
    }
}

template <typename T>
void mdsb<T>::build_dindex() {
    md->D_index.resize(k);

    #pragma omp parallel for num_threads(p)
    for (T j=0; j<k; j++) {
        // For each output interval [q_j, q_j + d_j - 1], find the input interval [p_i, p_i + d_i - 1], q_j is in and set D_index[j] = i.
        // Find the maximum integer i \in [0,k-1], so that p_i <= q_j with a binary search over D_pair.
        T l = 0;
        T r = k-1;
        T m;
        while (l != r) {
            m = (l+r)/2+1;
            if (md->D_pair[m].first > md->D_pair[j].second) {
                r = m-1;
            } else {
                l = m;
            }
        }
        md->D_index[j] = l;
    }
}

template <typename T>
T mdsb<T>::interval_length_seq(pair_list_node<T>* nodePair) {
    return nodePair->sc->v.first - nodePair->v.first;
}

template <typename T>
pair_list_node<T>* mdsb<T>::is_unbalanced_seq(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J) {
    // [l,r] = [q_j, q_j + d_j - 1]
    T l = ptn_J->v.v.second;
    T r = l + interval_length_seq(&ptn_J->v) - 1;

    // If |[l,r]| < a, there cannot be at least a input intervals connected to [l,r] in the permutation graph.
    if (r-l+1 < a) return NULL;

    // Count the number i of input intervals connected to [l,r] in the permutation graph and stop as soon as i == b.
    uint8_t i = 1;
    while (i < b+1 && pln_I->sc->sc != NULL && pln_I->sc->v.first <= r) {
        pln_I = pln_I->sc;
        i++;
    }
    if (i != b+1) return NULL; // i < b < a-1 => [l,r] is unbalanced

    pair_list_node<T>* pln_IpB = pln_I;

    // Count further and stop as soon as i == a.
    while (i < a && pln_I->sc->sc != NULL && pln_I->sc->v.first <= r) {
        pln_I = pln_I->sc;
        i++;
    }
    if (i != a) return NULL; // i < a => [l,r] is unbalanced

    return pln_IpB;
}

template <typename T>
pair_list_node<T>* mdsb<T>::is_unbalanced_par(pair_list_node<T>* pln_I, pair_tree_node<T>* ptn_J, pair_tree_node<T>* ptn_J_nxt) {
    // [l,r] = [q_j, q_j + d_j - 1]
    T l = ptn_J->v.v.second;
    T r = ptn_J_nxt->v.v.second - 1;

    // If |[l,r]| < a, there cannot be at least a input intervals connected to [l,r] in the permutation graph.
    if (r-l+1 < a) return NULL;

    // Count the number i of input intervals connected to [l,r] in the permutation graph and stop as soon as i == b.
    uint8_t i = 1;
    while (i < b+1 && pln_I->sc != NULL && pln_I->sc->v.first <= r) {
        pln_I = pln_I->sc;
        i++;
    }
    if (i != b+1) return NULL; // i < b < a-1 => [l,r] is unbalanced

    pair_list_node<T>* pln_IpB = pln_I;

    // Count further and stop as soon as i == a.
    while (i < a && pln_I->sc != NULL && pln_I->sc->v.first <= r) {
        pln_I = pln_I->sc;
        i++;
    }
    if (i != a) return NULL; // i < a => [l,r] is unbalanced

    return pln_IpB;
}