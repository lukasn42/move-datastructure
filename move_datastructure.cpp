#include "move_datastructure.h"

extern "C" {
    #include "build/_deps/malloc_count-src/malloc_count.h"
    #include "build/_deps/malloc_count-src/malloc_count.c"
}

template <typename T>
T move_datastructure<T>::interval_length(pair_list_node<T>* nodePair) {
    return nodePair->sc->v.first - nodePair->v.first;
}

template <typename T>
bool move_datastructure<T>::is_unbalanced(pair_list_node<T>* nodePairI, pair_list_node<T>* nodePairj) {
    // [l,r] = [q_j, q_j + d_j - 1]
    T l = nodePairj->v.second;
    T r = l + interval_length(nodePairj) - 1;

    // If |[l,r]| < a, there cannot be at least a input intervals connected to [l,r] in the permutation graph.
    if (r-l+1 < a) return false;

    // Count the number i of input intervals connected to [l,r] in the permutation graph and stop as soon as i == a.
    uint8_t i = 1;
    while (i < a && nodePairI->sc->sc != NULL && nodePairI->sc->v.first <= r) {
        nodePairI = nodePairI->sc;
        i++;
    }

    return i == a; // [l,r] is unbalanced <=> i == a
}

template <typename T>
void move_datastructure<T>::balance_upto(pair_list<T> *L_in, pair_tree<T> *T_out, pair_list_node<T>* nodePairI, pair_list_node<T>* nodePairJ, T q_u) {
    T p_j = nodePairJ->v.first;
    T q_j = nodePairJ->v.second;
    T d_j = interval_length(nodePairJ);

    pair_list_node<T> *nodePairIplusB = nodePairI;
    for (uint8_t i=0; i<b; i++) {
        nodePairIplusB = nodePairIplusB->sc;
    }
    
    // d_1 = p_{i+b} - q_j is the maximum integer, so that [q_j, q_j + d_1 - 1] has b incoming edges in the permutation graph.
    T d_1 = nodePairIplusB->v.first - q_j;

    // Create the pair (p_j + d_1, q_j + d_1), which creates two new input intervals [p_j, p_j + d_1 - 1] and [p_j + d_1, p_j + d_j - 1].
    pair_list_node<T> n{interv_pair<T>{p_j + d_1, q_j + d_1},NULL,NULL};
    pair_list_node<T> *nodePairNew = &T_out->insert_or_update(&n)->v;
    L_in->insert_node_after(nodePairJ,nodePairNew);

    /*
    The output interval [q_y, q_y + d_y - 1], to which [p_j + d_1, p_j + d_j - 1] is connected in the permutation graph, can be unbalanced.

    Case 1: If p_j + d_1 lies in [q_u, q_u + d_u - 1], then [q_u, q_u + d_u - 1] = [q_y, q_y + d_y - 1]. p_j + d_1 = q_u obviously is not possible.
            In general, once balance_upto creates a new input interval starting within [q_u, q_u + d_u - 1], the sequence of recursive calls ends.
            Because of that, [q_u, q_u + d_u - 1] had b incoming edges in the permutation graph before [p_j + d_1, p_j + d_j - 1] was created.
            Therefore after that [q_y, q_y + d_y - 1] has b+1 < a incoming edges in the permutation graph, hence it is balanced.

    Case 2: Else if p_j + d_1 >= q_u + d_u, then it is irrelevant if [q_y, q_y + d_y - 1] is unbalanced, because only output intervals upto [q_u, q_u + d_u - 1] must be balanced.

    Case 3: Else if p_j + d_1 lies in [q_j, q_j + d_1 - 1] or [q_j + d_1, q_j + d_j - 1], then [q_y, q_y + d_y - 1] equals one of them.
            Because they have at most b+1 < a and a-b+1 < a incoming edges in the permutation graph, [q_y, q_y + d_y - 1] cannot be unbalanced.
    
    Case 4: Else balance [q_y, q_y + d_y - 1].
    */
    // check if case 4 holds
    if (p_j + d_1 < q_u && !(q_j <= p_j + d_1 && p_j + d_1 < q_j + d_j)) {
        // find [q_y, q_y + d_y - 1]
        pair_list_node<T> n{interv_pair<T>{0,p_j + d_1},NULL,NULL};
        pair_list_node<T> *nodePairY = &T_out->maximum_leq(&n)->v;

        // find the first input interval [p_z, p_z + d_z - 1], that is connected to [q_y, q_y + d_y - 1] in the permutation graph
        pair_list_node<T> *nodePairZ = nodePairNew;
        while (nodePairZ->pr != NULL && nodePairZ->pr->v.first >= nodePairY->v.second) {
            nodePairZ = nodePairZ->pr;
        }

        if (is_unbalanced(nodePairZ,nodePairY)) {
            balance_upto(L_in,T_out,nodePairZ,nodePairY,q_u);
        }
    }
}

template <typename T>
move_datastructure<T>::move_datastructure(interv_seq<T> *I, T n, uint8_t a, uint8_t b) {
    size_t baseline = malloc_count_current() - sizeof(I->at(0))*I->size();

    k = I->size();

    if (k <= 0) {std::cout << "invalid value for k: (0 < k)" << std::endl; return;}
    if (n < k) {std::cout << "invalid value for n: (k <= n)" << std::endl; return;}
    if (b < 2) {std::cout << "invalid value for b: (2 <= b)" << std::endl; return;}
    if (b >= a-1) {std::cout << "invalid value for a: (b < a-1)" << std::endl; return;}

    this->n = n;
    this->k = k;
    this->a = a;
    this->b = b;

    pair_list<T> *L_in = new pair_list<T>(); // stores all pairs in I ordered by p_i

    pair_tree<T> *T_out = new pair_tree<T>( // stores all nodes of L_in ordered by q_i
        [](auto *n1, auto *n2){return n1->v.second < n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second > n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second == n2->v.second;}
    );

    std::cout << std::endl;
    std::cout << "building L_in and T_out: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated" << std::endl;
    
    // Build L_in and T_out
    for (T i=0; i<k; i++) { // in O(k log k)
        pair_list_node<T> n{I->at(i),NULL,NULL};
        L_in->push_back_node(&T_out->insert_or_update(&n)->v);
    }
    pair_list_node<T> nodeN{interv_pair<T>{n,0},NULL,NULL};
    L_in->push_back_node(&nodeN); // Append pair (n,0), so that interval_length() also works for the last pair.
    delete I;

    std::cout << "balancing interval sequence: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated" << std::endl;

    // itInp points to to the pair (p_i, q_i).
    typename pair_list<T>::dll_it itInp = L_in->iterator(L_in->head());
    // itOutp points to the pair (p_j, q_j).
    typename pair_tree<T>::avl_it itOutp = T_out->iterator(T_out->minimum());
    // At the start of each iteration, [p_i, p_i + d_i - 1] is the first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
    // and all output intervals starting before [q_j, q_j + d_j - 1] are balanced.
    do {
        // If [q_j, q_j + d_j - 1] is unbalanced, balance it and all output intervals starting before it, that might get unbalanced in the process.
        if (is_unbalanced(itInp.current(),&itOutp.current()->v)) {
            balance_upto(L_in,T_out,itInp.current(),&itOutp.current()->v,itOutp.current()->v.v.second);
        }

        // Find the next output interval with an incoming edge in the permutation graph and the first input interval connected to it.
        do {
            if (!itOutp.has_next()) goto end;
            itOutp.next();
            while (itInp.current()->v.first < itOutp.current()->v.v.second) {
                itInp.next();
                if (!itInp.has_next()) goto end;
            }
        } while (itInp.current()->v.first >= itOutp.current()->v.v.second + interval_length(&itOutp.current()->v));
    } while (true);
    end:

    k = L_in->size() - 1;

    std::cout << "building D_pair: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated" << std::endl;

    // Build D_pair
    D_pair.resize(k);
    typename pair_list<T>::dll_it it = L_in->iterator(L_in->head());
    for (T i=0; i<k; i++) { // in O(k)
        D_pair[i] = it.current()->v;
        it.next();
    }

    std::cout << "deleteing L_in and T_out: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated (peak)" << std::endl;

    // Remove the last node, because it will not get deleted when T_out gets deleted.
    L_in->remove_node(L_in->tail());
    // Disconnect L_in from it's nodes, because they will get deleted when T_out gets deleted.
    L_in->disconnect_nodes();
    delete L_in;
    delete T_out;

    std::cout << "building D_index: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated" << std::endl;

    // Build D_index
    D_index.resize(k);
    for (T j=0; j<k; j++) { // in O(k log k)
        // Find the input interval [p_i, p_i + d_i - 1], q_j is in.
        // Find the maximum integer i \in [0,k-1], so that D_pair[j].first <= D_pair[i].second.
        T l = 0;
        T r = k-1;
        T m;
        while (l != r) { // in O(log k)
            m = (l+r)/2+1;
            if (D_pair[m].first == D_pair[j].second) {
                l = r = m;
            } else if (D_pair[m].first > D_pair[j].second) {
                r = m-1;
            } else {
                l = m;
            }
        }
        D_index[j] = l;
    }

    std::cout << "move_datastructure built: ~ " << (malloc_count_current()-baseline)/800000 << "MB allocated" << std::endl;
    std::cout << std::endl;
}

template <typename T>
T move_datastructure<T>::size() {
    return k;
}

template <typename T>
T move_datastructure<T>::find_interval(T i) {
    T l = 0;
    T r = k-1;
    T m;
    while (l != r) { // in O(log k)
        m = (l+r)/2+1;
        if (D_pair[m].first == i) {
            l = r = m;
        } else if (D_pair[m].first > i) {
            r = m-1;
        } else {
            l = m;
        }
    }
    return m;
}

template <typename T>
interv_pair<T> move_datastructure<T>::move(T i, T x) {
    T i_ = D_pair[x].second+(i-D_pair[x].first);
    T x_ = D_index[x];
    while (i_ < D_pair[x_].first) x_++; // in O(a)
    return interv_pair<T> {i_,x_};
}