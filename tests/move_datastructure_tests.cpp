#include <gtest/gtest.h>
#include "../move_datastructure.cpp"

template <typename T>
class move_datastructure_test : public move_datastructure<T> {
    public:
    using move_datastructure<T>::n;
    using move_datastructure<T>::k;
    using move_datastructure<T>::a;
    using move_datastructure<T>::b;
    using move_datastructure<T>::D_pair;
    using move_datastructure<T>::D_index;

    using move_datastructure<T>::interval_length;
    using move_datastructure<T>::is_unbalanced;
    using move_datastructure<T>::balance_upto;

    using move_datastructure<T>::move_datastructure;
    using move_datastructure<T>::find_interval;
    using move_datastructure<T>::move;
};

/**
 * @brief inserts the pairs in I and (n,0) into L_in and tests if interval_length((p_i,q_i)) == expect
 * 
 * @tparam T (unsigned integer) type of the interval starting positions
 * @param I interval sequence I[0..k-1]
 * @param n maximum value, n = p_{k-1} + d_{k-1}, k <= n
 * @param i i in [0..k-1]
 * @param expect expected return value of interval_length((p_i,q_i))
 */
template <typename T>
void interval_length_test_case(interv_seq<T> I, T n, T i, T expect) {
    // create L_in
    pair_list<T> *L_in = new pair_list<T>();
    for (auto p : I) {
        L_in->push_back_node(new pair_list_node<T>{p,NULL,NULL});
    }
    // push back pair (n,0) so that interval_length() also works for the last pair
    L_in->push_back_node(new pair_list_node<T>{interv_pair<T>{n,0},NULL,NULL});

    // find node nodePairI in L_in
    pair_list_node<T> *nodePairI = L_in->head();
    for (T k=0; k<i; k++) {
        nodePairI = nodePairI->sc;
    }

    // test if interval_length((p_i,q_i)) == expect
    EXPECT_EQ(move_datastructure_test<T>::interval_length(nodePairI),expect);

    delete L_in;
}

/**
 * @brief inserts the pairs in I and (n,0) into L_in and tests if is_unbalanced((p_i,q_i),(p_j,q_j)) == expect
 * 
 * @tparam T (unsigned integer) type of the interval starting positions
 * @param I interval sequence I[0..k-1]
 * @param n maximum value, n = p_{k-1} + d_{k-1}, k <= n
 * @param i i in [0..k-1]
 * @param j j in [0..k-1]
 * @param expect expected return value of is_unbalanced((p_i,q_i),(p_j,q_j))
 */
template <typename T>
void is_unbalanced_test_case(interv_seq<T> I, T n, T i, T j, bool expect) {
    // create L_in
    pair_list<T> *L_in = new pair_list<T>();
    for (auto p : I) {
        L_in->push_back_node(new pair_list_node<T>{p,NULL,NULL});
    }
    // push back pair (n,0) so that interval_length() also works for the last pair
    L_in->push_back_node(new pair_list_node<T>{interv_pair<T>{n,0},NULL,NULL});

    // find node nodePairI and nodePairJ in L_in
    pair_list_node<T> *nodePairI = L_in->head();
    pair_list_node<T> *nodePairJ = L_in->head();
    for (T k=0; k<i; k++) {
        nodePairI = nodePairI->sc;
    }
    for (T k=0; k<j; k++) {
        nodePairJ = nodePairJ->sc;
    }

    // test if is_unbalanced((p_i,q_i),(p_j,q_j)) == expect
    move_datastructure_test<T> mdt(new interv_seq<T>{interv_pair<T>{0,0}},1,4,2);
    EXPECT_EQ(mdt.is_unbalanced(nodePairI,nodePairJ),expect);

    delete L_in;
}

/**
 * @brief inserts the pairs in I_before into L_in and T_out (and (n,0) into L_in),
 *        calls balance_upto(L_in,T_out,(p_i,q_i),(p_j,q_j),q_j) and
 *        tests if the pairs in L_in and T_out equal the pairs in I_expected
 * 
 * @tparam T (unsigned integer) type of the interval starting positions
 * @param I_before interval sequence I_before[0..k-1]
 * @param I_expected expected interval sequence I_expected[0..k'-1] after call of balance_upto
 * @param n maximum value, n = p_{k-1} + d_{k-1}, k <= n
 * @param i i in [0..k-1]
 * @param j j in [0..k-1]
 */
template <typename T>
void balance_upto_test_case(interv_seq<T> I_before, interv_seq<T> I_expected, T n, T i, T j) {
    // create L_in and T_out
    pair_list<T> *L_in = new pair_list<T>();
    pair_tree<T> *T_out = new pair_tree<T>(
        [](auto *n1, auto *n2){return n1->v.second < n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second > n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second == n2->v.second;}
    );

    // insert the pairs in I_before into L_in and T_out
    for (interv_pair<T> p : I_before) {
        pair_list_node<T> n{p,NULL,NULL};
        L_in->push_back_node(&T_out->insert_or_update(&n)->v);
    }
    // push back pair (n,0) so that interval_length() also works for the last pair
    L_in->push_back_node(new pair_list_node<T>{interv_pair<T>{n,0},NULL,NULL});

    // find node nodePairI and nodePairJ in L_in
    pair_list_node<T> *nodePairI = L_in->head();
    pair_list_node<T> *nodePairJ = L_in->head();
    for (T k=0; k<i; k++) {
        nodePairI = nodePairI->sc;
    }
    for (T k=0; k<j; k++) {
        nodePairJ = nodePairJ->sc;
    }
    move_datastructure_test<T> mdt(new interv_seq<T>{interv_pair<T>{0,0}},1,4,2);

    // balance [q_j, q_j + d_j - 1]
    mdt.balance_upto(L_in,T_out,nodePairI,nodePairJ,nodePairJ->v.second);

    // compare I_expected to L_in and T_out
    EXPECT_EQ(L_in->size(),I_expected.size()+1);
    EXPECT_EQ(T_out->size(),I_expected.size());
    typename pair_list<T>::dll_it it_L_in = L_in->iterator(L_in->head());
    for (interv_pair<T> p : I_expected) {
        EXPECT_EQ(it_L_in.current()->v,p);
        EXPECT_FALSE(T_out->find(it_L_in.current()) == NULL);
        if (it_L_in.has_next()) it_L_in.next();
    }
    // the pair (n,0) should only be in T_out
    EXPECT_EQ(it_L_in.current()->v,(interv_pair<T>{n,0}));

    // delete L_in and T_out
    L_in->remove_node(L_in->tail());
    L_in->disconnect_nodes();
    delete L_in;
    delete T_out;
}

TEST(move_datastructure_test,interval_length_test) {
    interv_seq<uint8_t> I;
    uint8_t n,i;
    uint8_t expect;

    I = {
        interv_pair<uint8_t>(0,1),
        interv_pair<uint8_t>(7,8),
        interv_pair<uint8_t>(49,0)
    };
    n = 51;
    i = 0;
    expect = 7;
    interval_length_test_case<uint8_t>(I,n,i,expect);

    i = 1;
    expect = 42;
    interval_length_test_case<uint8_t>(I,n,i,expect);
}

TEST(move_datastructure_test,is_unbalanced_test) {
    interv_seq<uint8_t> I;
    uint8_t n,i,j;
    bool expect;
    
    // Case 1: The output interval has 1 incoming edge in the permutation graph.
    I = {
        interv_pair<uint8_t>(0,1),
        interv_pair<uint8_t>(1,0),
    };
    n = 2;
    i = 0;
    j = 1;
    expect = false;
    is_unbalanced_test_case<uint8_t>(I,n,i,j,expect);

    // Case 2: The output interval has 3 = a-1 incoming edges in the permutation graph.
    I = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(2,6),
        interv_pair<uint8_t>(3,7),
        interv_pair<uint8_t>(4,8),
        interv_pair<uint8_t>(5,0)
    };
    n = 9;
    i = 0;
    j = 4;
    expect = false;
    is_unbalanced_test_case<uint8_t>(I,n,i,j,expect);

    // Case 3: The output interval has 4 = a incoming edges in the permutation graph.
    I = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(1,5),
        interv_pair<uint8_t>(2,6),
        interv_pair<uint8_t>(3,7),
        interv_pair<uint8_t>(4,8),
        interv_pair<uint8_t>(5,0)
    };
    n = 9;
    i = 0;
    j = 5;
    expect = true;
    is_unbalanced_test_case<uint8_t>(I,n,i,j,expect);
}

TEST(move_datastructure_test,balance_upto_test) {
    /*
        Case 1: q_j = q_u
            Case 1.1: p_j + d_1 >= q_u
                Case 1.1.1: [q_y, q_y + d_y - 1] did not get unbalanced by creating [p_j + d_1, p_j + d_j - 1].
                Case 1.1.2: [q_y, q_y + d_y - 1] got unbalanced by creating [p_j + d_1, p_j + d_j - 1].
            Case 1.2: p_j + d_1 < q_u
                Case 1.2.1: [q_y, q_y + d_y - 1] did not get unbalanced by creating [p_j + d_1, p_j + d_j - 1].
                Case 1.2.2: [q_y, q_y + d_y - 1] got unbalanced by creating [p_j + d_1, p_j + d_j - 1].
        Case 2: q_j != q_u
            Case 2.1: p_j + d_1 >= q_u
                Case 2.1.1: [q_y, q_y + d_y - 1] did not get unbalanced by creating [p_j + d_1, p_j + d_j - 1].
                Case 2.1.2: [q_y, q_y + d_y - 1] got unbalanced by creating [p_j + d_1, p_j + d_j - 1].
            Case 2.2: p_j + d_1 < q_u and p_j + d_1 \in [q_j, q_j + d_j - 1]. In this case [q_y, q_y + d_y - 1] cannot get unbalanced by creating [p_j + d_1, p_j + d_j - 1].
            Case 2.3: p_j + d_1 < q_u and p_j + d_1 \notin [q_j, q_j + d_j - 1]
                Case 2.3.1: [q_y, q_y + d_y - 1] did not get unbalanced by creating [p_j + d_1, p_j + d_j - 1].
                Case 2.3.2: [q_y, q_y + d_y - 1] got unbalanced by creating [p_j + d_1, p_j + d_j - 1].
    */

    interv_seq<uint8_t> I_before;
    interv_seq<uint8_t> I_expected;
    uint8_t n,i,j;

    // Case 1.1.1
    I_before = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(1,5),
        interv_pair<uint8_t>(2,6),
        interv_pair<uint8_t>(3,7),
        interv_pair<uint8_t>(4,0)
    };
    I_expected = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(1,5),
        interv_pair<uint8_t>(2,6),
        interv_pair<uint8_t>(3,7),
        interv_pair<uint8_t>(4,0),
        interv_pair<uint8_t>(6,2)
    };
    n = 8;
    i = 0;
    j = 4;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 1.1.2
    I_before = {
        interv_pair<uint8_t>(0,10),
        interv_pair<uint8_t>(1,11),
        interv_pair<uint8_t>(2,12),
        interv_pair<uint8_t>(3,13),
        interv_pair<uint8_t>(4,0),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,4)
    };
    I_expected = {
        interv_pair<uint8_t>(0,10),
        interv_pair<uint8_t>(1,11),
        interv_pair<uint8_t>(2,12),
        interv_pair<uint8_t>(3,13),
        interv_pair<uint8_t>(4,0),
        interv_pair<uint8_t>(6,2),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,4)
    };
    n = 16;
    i = 0;
    j = 4;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 1.2.1
    I_before = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(4,0),
        interv_pair<uint8_t>(5,1),
        interv_pair<uint8_t>(6,2),
        interv_pair<uint8_t>(7,3)
    };
    I_expected = {
        interv_pair<uint8_t>(0,4),
        interv_pair<uint8_t>(2,6),
        interv_pair<uint8_t>(4,0),
        interv_pair<uint8_t>(5,1),
        interv_pair<uint8_t>(6,2),
        interv_pair<uint8_t>(7,3)
    };
    n = 8;
    i = 1;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 1.2.2
    I_before = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(4,10),
        interv_pair<uint8_t>(5,11),
        interv_pair<uint8_t>(6,12),
        interv_pair<uint8_t>(7,13),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,0)
    };
    I_expected = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(2,8),
        interv_pair<uint8_t>(4,10),
        interv_pair<uint8_t>(5,11),
        interv_pair<uint8_t>(6,12),
        interv_pair<uint8_t>(7,13),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,0),
        interv_pair<uint8_t>(14,4)
    };
    n = 16;
    i = 3;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 2.1.1
    I_before = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(4,10),
        interv_pair<uint8_t>(5,11),
        interv_pair<uint8_t>(6,12),
        interv_pair<uint8_t>(7,13),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,0)
    };
    I_expected = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(2,8),
        interv_pair<uint8_t>(4,10),
        interv_pair<uint8_t>(5,11),
        interv_pair<uint8_t>(6,12),
        interv_pair<uint8_t>(7,13),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(9,15),
        interv_pair<uint8_t>(10,0),
        interv_pair<uint8_t>(14,4)
    };
    n = 16;
    i = 3;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 2.1.2
    I_before = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(4,18),
        interv_pair<uint8_t>(5,19),
        interv_pair<uint8_t>(6,10),
        interv_pair<uint8_t>(7,11),
        interv_pair<uint8_t>(8,12),
        interv_pair<uint8_t>(9,13),
        interv_pair<uint8_t>(10,0),
        interv_pair<uint8_t>(16,20),
        interv_pair<uint8_t>(17,21),
        interv_pair<uint8_t>(18,14)
    };
    I_expected = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(2,8),
        interv_pair<uint8_t>(4,18),
        interv_pair<uint8_t>(5,19),
        interv_pair<uint8_t>(6,10),
        interv_pair<uint8_t>(7,11),
        interv_pair<uint8_t>(8,12),
        interv_pair<uint8_t>(9,13),
        interv_pair<uint8_t>(10,0),
        interv_pair<uint8_t>(14,4),
        interv_pair<uint8_t>(16,20),
        interv_pair<uint8_t>(17,21),
        interv_pair<uint8_t>(18,14)
    };
    n = 22;
    i = 3;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 2.2
    I_before = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(12,0),
        interv_pair<uint8_t>(13,1),
        interv_pair<uint8_t>(14,2),
        interv_pair<uint8_t>(15,3),
        interv_pair<uint8_t>(16,4),
        interv_pair<uint8_t>(17,5)
    };
    I_expected = {
        interv_pair<uint8_t>(0,6),
        interv_pair<uint8_t>(6,12),
        interv_pair<uint8_t>(8,14),
        interv_pair<uint8_t>(10,16),
        interv_pair<uint8_t>(12,0),
        interv_pair<uint8_t>(13,1),
        interv_pair<uint8_t>(14,2),
        interv_pair<uint8_t>(15,3),
        interv_pair<uint8_t>(16,4),
        interv_pair<uint8_t>(17,5)
    };
    n = 18;
    i = 4;
    j = 1;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 2.3.1
    I_before = {
        interv_pair<uint8_t>(0,12),
        interv_pair<uint8_t>(4,6),
        interv_pair<uint8_t>(5,7),
        interv_pair<uint8_t>(6,0),
        interv_pair<uint8_t>(12,8),
        interv_pair<uint8_t>(13,9),
        interv_pair<uint8_t>(14,10),
        interv_pair<uint8_t>(15,11)
    };
    I_expected = {
        interv_pair<uint8_t>(0,12),
        interv_pair<uint8_t>(2,14),
        interv_pair<uint8_t>(4,6),
        interv_pair<uint8_t>(5,7),
        interv_pair<uint8_t>(6,0),
        interv_pair<uint8_t>(10,4),
        interv_pair<uint8_t>(12,8),
        interv_pair<uint8_t>(13,9),
        interv_pair<uint8_t>(14,10),
        interv_pair<uint8_t>(15,11)
    };
    n = 16;
    i = 4;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);

    // Case 2.3.2
    I_before = {
        interv_pair<uint8_t>(0,14),
        interv_pair<uint8_t>(4,18),
        interv_pair<uint8_t>(5,19),
        interv_pair<uint8_t>(6,0),
        interv_pair<uint8_t>(12,20),
        interv_pair<uint8_t>(13,21),
        interv_pair<uint8_t>(14,22),
        interv_pair<uint8_t>(15,23),
        interv_pair<uint8_t>(16,24),
        interv_pair<uint8_t>(17,25),
        interv_pair<uint8_t>(18,6)
    };
    I_expected = {
        interv_pair<uint8_t>(0,14),
        interv_pair<uint8_t>(2,16),
        interv_pair<uint8_t>(4,18),
        interv_pair<uint8_t>(5,19),
        interv_pair<uint8_t>(6,0),
        interv_pair<uint8_t>(10,4),
        interv_pair<uint8_t>(12,20),
        interv_pair<uint8_t>(13,21),
        interv_pair<uint8_t>(14,22),
        interv_pair<uint8_t>(15,23),
        interv_pair<uint8_t>(16,24),
        interv_pair<uint8_t>(17,25),
        interv_pair<uint8_t>(18,6),
        interv_pair<uint8_t>(24,12)
    };
    n = 26;
    i = 6;
    j = 0;
    balance_upto_test_case<uint8_t>(I_before,I_expected,n,i,j);
}

TEST(move_datastructure_test,move_datastructure_constructor_test) {
    
}

/*
    for (T j=0; j<k; j++) {
        T l = 0;
        T r = k-1;
        T m;
        while (l != r) {
            m = (l+r)/2;
            if (D_pair[m].first == D_pair[j].second) {
                l = r = m;
            } else if (D_pair[m].first > D_pair[j].second) {
                r = m;
            } else {
                l = m+1;
            }
        }
        T i = l;

        if (i+a-1 < k) {
            T q_j = D_pair[j].second;
            T d_j = D_pair[j+1].first - D_pair[j].first;

            T p_i = D_pair[i].first;
            T p_ipam1st = D_pair[i+a-1].first;

            EXPECT_FALSE(p_i < q_j + d_j && p_ipam1st < q_j + d_j);
        }
    }
*/