#include <ips4o.hpp>

#include <mdsb.hpp>

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

    // [0..p-1] section start positions in the range [0..n], 0 = s[0] < s[1] < ... < s[p-1] = n
    s = std::vector<T>(p+1);
    s[0] = 0;
    s[p] = n;

    // [0..p-1], u[i] stores the number of output intervals in I starting before s[i]
    std::vector<T> u(p+1);
    u[0] = 0;
    u[p] = k;

    // [0..p-1], x[i] stores the number of input intervals in I starting before s[i]
    std::vector<T> x(p+1);
    x[0] = 0;
    x[p] = k;

    {
        // create identity permutation pi of [0..k-1]
        std::vector<T> pi(k);
        #pragma omp parallel for num_threads(p)
        for (T i=0; i<k; i++) {
            pi[i] = i;
        }

        // sort pi by q
        auto comp = [I](T i1, T i2){return I->at(i1).second < I->at(i2).second;};
        if (p > 1) {
            ips4o::parallel::sort(pi.begin(),pi.end(),comp);
        } else {
            ips4o::sort(pi.begin(),pi.end(),comp);
        }

        // calculate seperation positions
        #pragma omp parallel
        {
            int i_p = omp_get_thread_num();

            T o = i_p*((2*k)/p);

            T l_s,l_x,l_u,m_s,m_x,m_u,r_s,r_x,r_u;

            l_s = 0;
            r_s = n-1;
            do {
                m_s = (l_s+r_s)/2;

                l_x = 0;
                r_x = k-1;
                while (l_x != r_x) {
                    m_x = (l_x+r_x)/2;
                    if (I->at(m_x).first < m_s) {
                        l_x = m_x+1;
                    } else {
                        r_x = m_x;
                    }
                }

                l_u = 0;
                r_u = k-1;
                while (l_u != r_u) {
                    m_u = (l_u+r_u)/2;
                    if (I->at(pi[m_u]).second < m_s) {
                        l_u = m_u+1;
                    } else {
                        r_u = m_u;
                    }
                }

                if (l_s == r_s) {
                    break;
                }

                if (l_x+l_u < o) {
                    l_s = m_s + 1;
                } else {
                    r_s = m_s;
                }
            } while (true);

            x[i_p] = l_x;
            u[i_p] = l_u;
            s[i_p] = l_s;
        }

        // stores the nodes to build L_in[0..p-1] and T_out[0..p-1] out of
        nodes = new std::vector<pair_tree_node<T>>(k);

        // insert pairs into nodes and build L_in[0..p-1]
        nodes->at(0).v.v = I->at(0);
        #pragma omp parallel for num_threads(p)
        for (T i=1; i<k; i++) {
            nodes->at(i).v.v = I->at(i);
            nodes->at(i).v.pr = &nodes->at(i-1).v;
            nodes->at(i-1).v.sc = &nodes->at(i).v;
        }

        for (int i=0; i<p; i++) {
            L_in[i].set_head(&nodes->at(x[i]).v);
            L_in[i].set_tail(&nodes->at(x[i+1]-1).v);
            L_in[i].set_size(x[i+1]-x[i]);
        }
        for (int i=1; i<p; i++) {
            L_in[i-1].tail()->sc = NULL;
            L_in[i].head()->pr = NULL;
        }

        delete I;

        // build T_out[0..p-1] from nodes[0..k-1]
        #pragma omp parallel num_threads(p)
        {
            #pragma omp single
            {
                for (int i_p=0; i_p<p; i_p++) {
                    #pragma omp task
                    {
                        T_out[i_p].insert_array(nodes,u[i_p],u[i_p+1]-1,2,[&pi](int i){return pi[i];});
                    }
                }

                #pragma omp taskwait
            }
        }
    }

    // build new_nodes[0..p-1]
    new_nodes = std::vector<dg_io_nc<pair_tree_node<T>>>(p);
    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        new_nodes[i_p] = dg_io_nc<pair_tree_node<T>>(k/(double) (16*p*(a-1)));
    }

    // make sure each list L_in[i], with i in [0..p-1], contains a pair creating an input interval starting at s[i]
    for (int i=1; i<p; i++) {
        if (L_in[i].empty() || L_in[i].head()->v.first != s[i]) {
            pair_list_node<T> *pln = L_in[i-1].tail();
            pair_tree_node<T> *ptn = new_nodes[i].emplace_back(pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{s[i],pln->v.second+s[i]-pln->v.first})));

            // find i_ \in [0,p-1], so that s[i_] <= ptn->v.v.second < s[i_+1]
            int l = 0;
            int r = p-1;
            int m;
            while (l != r) {
                m = (l+r)/2+1;
                if (s[m] > ptn->v.v.second) {
                    r = m-1;
                } else {
                    l = m;
                }
            }
            int i_ = l;

            T_out[i_].insert_node(ptn);
            L_in[i].push_front_node(&ptn->v);
        }
    }
    
    // make sure each avl tree T_out[i], with i in [0..p-1], contains a pair creating an output interval starting at s[i]
    for (int i=1; i<p; i++) {
        if (T_out[i].empty() || T_out[i].minimum()->v.v.second != s[i]) {
            pair_list_node<T> *pln = &T_out[i-1].maximum()->v;
            pair_tree_node<T> *ptn = new_nodes[i].emplace_back(pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{pln->v.first+s[i]-pln->v.second,s[i]})));

            // find i_ \in [0,p-1], so that s[i_] <= ptn->v.v.first < s[i_+1]
            int l = 0;
            int r = p-1;
            int m;
            while (l != r) {
                m = (l+r)/2+1;
                if (s[m] > pln->v.first) {
                    r = m-1;
                } else {
                    l = m;
                }
            }
            int i_ = l;

            L_in[i_].insert_after_node(&ptn->v,pln);
            T_out[i].insert_node(ptn);
        }
    }

    // insert the pair (s[i+1],s[i+1]) into each avl tree T_out[i], with i in [0..p-1]
    #pragma omp parallel num_threads(p)
    {
        int i_p = omp_get_thread_num();

        T_out[i_p].insert_node(new_nodes[i_p].emplace_back(pair_tree_node<T>(pair_list_node<T>(interv_pair<T>{s[i_p+1],s[i_p+1]}))));
    }
}

template <typename T>
void mdsb<T>::build_dpair() {
    // [0..p-1], x[i] stores the number of input intervals in I starting before s[i]
    std::vector<T> x(p+1);
    x[0] = 0;
    for (int i=0; i<p; i++) {
        x[i+1] = x[i]+L_in[i].size();
    }
    
    k = x[p];
    md->k = k;
    md->D_pair.resize(k+1);
    md->D_pair[k] = interv_pair<T>{n,n};

    // Place the pairs in L_in[i_p] into D_pair[x[i_p]..x[i_p+1]-1] for each i_p in [0..p-1].
    #pragma omp parallel num_threads(p)
    {
        #pragma omp single
        {
            for (int i_p=0; i_p<p; i_p++) {
                
                T l = x[i_p];
                T r = x[i_p+1]-1;
                T m = (l+r)/2;

                #pragma omp task
                {
                    typename pair_list<T>::dll_it it = L_in[i_p].iterator();

                    for (T i=l; i<=m; i++) {
                        md->D_pair[i] = it.current()->v;
                        it.next();
                    }
                }
                #pragma omp task
                {
                    typename pair_list<T>::dll_it it = L_in[i_p].iterator(L_in[i_p].tail());

                    for (T i=r; i>m; i--) {
                        md->D_pair[i] = it.current()->v;
                        it.previous();
                    }
                }
            }

            #pragma omp taskwait
        }
    }

    for (int i=0; i<p; i++) {
        L_in[i].disconnect_nodes();
        T_out[i].disconnect_nodes();
    }

    new_nodes.clear();
    
    delete nodes;
    nodes = NULL;
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
T mdsb<T>::interval_length_seq(pair_list_node<T> *pln) {
    return (pln->sc != NULL ? pln->sc->v.first : n) - pln->v.first;
}

template <typename T>
pair_list_node<T>* mdsb<T>::is_unbalanced(pair_list_node<T> **pln_IpI_, T *i_, pair_tree_node<T> *ptn_J, pair_tree_node<T> *ptn_J_nxt) {
    // [l,r] = [q_j, q_j + d_j - 1]
    T rp1 = ptn_J_nxt == NULL ? ptn_J->v.v.second + interval_length_seq(&ptn_J->v) : ptn_J_nxt->v.v.second;

    // If |[l,r]| < 2a, there cannot be at least 2a input intervals connected to [l,r] in the permutation graph.
    if (rp1-ptn_J->v.v.second < 2*a) return NULL;

    T i_start = *i_;

    // Count the number i of input intervals connected to [l,r] in the permutation graph and stop as soon as i > a.
    do {
        if (*i_ > a) break;
        if ((*pln_IpI_)->sc == NULL) return NULL;
        *pln_IpI_ = (*pln_IpI_)->sc;
        (*i_)++;
        if ((*pln_IpI_)->v.first >= rp1) return NULL;
    } while (true);

    pair_list_node<T> *pln_IpA = *pln_IpI_;

    // Count further and stop as soon as i >= 2a.
    do {
        if (*i_ >= 2*a) {
            // if i_ > a+1 held in the beginning, correct pln_IpA
            while (i_start > a+1) {
                pln_IpA = pln_IpA->pr;
                i_start--;
            }
            *i_ = a;
            return pln_IpA;
        }
        if ((*pln_IpI_)->sc == NULL) return NULL;
        *pln_IpI_ = (*pln_IpI_)->sc;
        (*i_)++;
        if ((*pln_IpI_)->v.first >= rp1) return NULL;
    } while (true);
}