#include <iostream>

#include "../../include/mdsb/mdsb.hpp"

template <typename T>
void mdsb<T>::build_v1(interv_seq<T> *I, bool log) {
    size_t baseline;
    std::chrono::steady_clock::time_point time;

    if (log) {
        baseline = malloc_count_current() - sizeof(I->at(0))*I->size();
        time = std::chrono::steady_clock::now();
        malloc_count_reset_peak();
        std::cout << std::endl;
    }

    if (log) log_memory_usage<T>(baseline,"building T_in and T_out");

    // stores the pairs in I sorted by p_i
    avl_tree<std::pair<T,T>> T_in(
        [](auto n1, auto n2){return n1.first < n2.first;},
        [](auto n1, auto n2){return n1.first > n2.first;},
        [](auto n1, auto n2){return n1.first == n2.first;}
    );

    // stores the pairs in I sorted by q_i
    avl_tree<std::pair<T,T>> T_out(
        [](auto n1, auto n2){return n1.second < n2.second;},
        [](auto n1, auto n2){return n1.second > n2.second;},
        [](auto n1, auto n2){return n1.second == n2.second;}
    );

    // stores the pairs in I sorted by p_i, whiches output intervals have at least 4 incoming edges in the permutation graph
    avl_tree<std::pair<T,T>> T_e(
        [](auto n1, auto n2){return n1.first < n2.first;},
        [](auto n1, auto n2){return n1.first > n2.first;},
        [](auto n1, auto n2){return n1.first == n2.first;}
    );

    I->push_back(std::make_pair(n,n));

    // build T_in and T_out
    for (auto p : *I) {
        T_in.insert_or_update(p);
        T_out.insert_or_update(p);
    }

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"building T_e");
    }

    // build T_e
    T q_i,q_next;
    avl_node<std::pair<T,T>> *node_cur;
    uint8_t e;
    for (T i=0; i<k; i++) {
        // For each output interval [q_i, q_i + d_i - 1], find the first input interval connected to it in the permutation graph.
        q_i = I->at(i).second;
        q_next = q_i + I->at(i+1).first - I->at(i).first;
        node_cur = T_in.minimum_geq(std::pair<T,T>{q_i,0});
        // Count the number of input intervals connected to it in the permutation graph.
        e = 0;
        while (node_cur != NULL) {
            if (node_cur->v.first < q_next) {
                e++;
                if (e == 2*a) {
                    // If there are at least 2a, insert it's corresponding pair into T_e.
                    T_e.insert_or_update(I->at(i));
                    break;
                }
            } else {
                break;
            }
            node_cur = node_cur->nxt();
        }
    }

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"balancing");
    }

    // balance the interval sequence
    T d,q_j,p_j,q_y,d_j,d_y;
    std::pair<T,T> pair_NEW,pair_Y;
    avl_node<std::pair<T,T>> *node_Ipb,*min,*node_NEW,*node_Y;
    std::vector<std::tuple<T,T,std::pair<T,T>>> intervals_to_check;
    while (!T_e.empty()) {
        // Find the pair creating the first unbalanced output interval [q_j, q_j + d_j - 1] and remove it from T_e.
        min = T_e.minimum();
        p_j = min->v.first;
        q_j = min->v.second;
        T_e.remove(min->v);

        // Find the a+1-st input interval in [q_j, q_j + d_j - 1] and set d = p_{i+a} - q_j.
        // d is the smallest integer, so that [q_j, q_j + d - 1] has a incoming edges in the permutation graph.
        node_Ipb = T_in.minimum_geq(std::make_pair(q_j,0));
        for (T i=0; i<a; i++) {
            node_Ipb = node_Ipb->nxt();
        }
        d = node_Ipb->v.first-q_j;

        // Create the new pair (p_j + d, q_j + d) and insert it into T_in and T_out.
        pair_NEW = std::make_pair(p_j+d,q_j+d);
        T_in.insert_or_update(pair_NEW);
        node_NEW = T_out.insert_or_update(pair_NEW);

        // Find the output interval [q_y, q_y + d_y - 1], [p_j + d, p_j + d_j - 1] is connected to in the permutation graph.
        node_Y = T_out.maximum_leq(std::make_pair(0,p_j+d));
        pair_Y = node_Y->v;
        q_y = pair_Y.second;

        // The number of input intervals connected to [q_j + d, q_j + d_j - 1] and [q_y, q_y + d_y - 1] may have changed.
        // For each, check if it has at least 2a incoming edges in the permutation graph and insert it into T_e, if it has.
        d_j = node_NEW->nxt()->v.second-q_j;
        d_y = node_Y->nxt()->v.second-q_y;
        intervals_to_check = {
            std::make_tuple(q_j+d,q_j+d_j-1,pair_NEW),
            std::make_tuple(q_y,q_y+d_y-1,pair_Y)
        };
        for (auto tup : intervals_to_check) {
            // Find the first input interval connected to the output interval in the permutation graph.
            node_cur = T_in.minimum_geq(std::make_pair(std::get<0>(tup),0));

            // Count the number of input intervals connected to it in the permutation graph.
            e = 0;
            while (node_cur != NULL) {
                if (node_cur->v.first <= std::get<1>(tup)) {
                    e++;
                    // If there are at least 2a, insert it's corresponding pair into T_e.
                    if (e == 2*a) {
                        T_e.insert_or_update(std::get<2>(tup));
                        break;
                    }
                } else {
                    break;
                }
                node_cur = node_cur->nxt();
            }
        }
    }
    // Because T_e is empty, there are no unbalanced output intervals.

    md->k = k = T_in.size()-1;

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"building D_pair");
    }
    
    // build D_pair
    md->D_pair.resize(k+1);
    auto it = T_in.iterator();
    for (T i=0; i<=k; i++) {
        md->D_pair[i] = it.current()->v;
        it.next();
    }

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"building D_index");
    }

    // build D_index
    md->D_index.resize(k);
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

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"move datastructure built");
    }

    if (log) std::cout << std::endl << "peak memory allocation during build: ~ " << (malloc_count_peak()-baseline)/1000000 << "MB" << std::endl << std::endl;
}