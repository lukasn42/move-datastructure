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

TEST(move_datastructure_test,interval_length_test) {
    auto *L_in = new dl_list<interv_pair<uint32_t>>();

    interv_seq<uint32_t> I = {
        interv_pair<uint32_t>(0,1),
        interv_pair<uint32_t>(7,8),
        interv_pair<uint32_t>(49,0)
    };
    // build L_in from I
    for (auto p : I) {
        L_in->push_back_node(new pair_list_node<uint32_t>{p,NULL,NULL});
    }
    // append pair (n,0) so that interval_length() also works for the last pair
    pair_list_node<uint32_t> n{interv_pair<uint32_t>{51,0},NULL,NULL}; 
    L_in->push_back_node(&n);

    // the length the input interval [0,7]/output interval [1,8] should equal 7
    EXPECT_EQ(move_datastructure_test<uint32_t>::interval_length(L_in->head()),7);

    // the length the input interval [7,49]/output interval [8,50] should equal 42
    EXPECT_EQ(move_datastructure_test<uint32_t>::interval_length(L_in->head()->sc),42);

    delete L_in;
}

TEST(move_datastructure_test,is_unbalanced_test) {
    auto *mdt = new move_datastructure_test<uint32_t>(new interv_seq<uint32_t>{interv_pair<uint32_t>{0,0}},1,4,2);

    auto *L_in = new pair_list<uint32_t>();

    interv_seq<uint32_t> I = {
        interv_pair<uint32_t>(0,4),
        interv_pair<uint32_t>(2,6),
        interv_pair<uint32_t>(3,7),
        interv_pair<uint32_t>(4,8),
        interv_pair<uint32_t>(5,0)
    };
    // build L_in from I
    for (auto p : I) {
        L_in->push_back_node(new pair_list_node<uint32_t>{p,NULL,NULL});
    }
    // append pair (n,0) so that interval_length() also works for the last pair
    pair_list_node<uint32_t> n{interv_pair<uint32_t>{9,0},NULL,NULL}; 
    L_in->push_back_node(&n);

    EXPECT_FALSE(mdt->is_unbalanced(L_in->head(),L_in->tail()->pr));

    L_in->insert_node_after(L_in->head(),new pair_list_node<uint32_t>{interv_pair<uint32_t>{1,5},NULL,NULL});

    EXPECT_TRUE(mdt->is_unbalanced(L_in->head(),L_in->tail()->pr));

    delete mdt;
    delete L_in;
}

TEST(move_datastructure_test,balance_upto_test) {
    auto *mdt = new move_datastructure_test<uint32_t>(new interv_seq<uint32_t>{interv_pair<uint32_t>{0,0}},1,4,2);

    auto *T_out = new pair_tree<uint32_t>(
        [](auto *n1, auto *n2){return n1->v.second < n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second > n2->v.second;},
        [](auto *n1, auto *n2){return n1->v.second == n2->v.second;}
    );
    auto *L_in = new pair_list<uint32_t>();

    interv_seq<uint32_t> I = {
        interv_pair<uint32_t>(0,4),
        interv_pair<uint32_t>(1,5),
        interv_pair<uint32_t>(2,6),
        interv_pair<uint32_t>(3,7),
        interv_pair<uint32_t>(4,0)
    };
    // build L_in and T_out from I
    for (auto p : I) {
        pair_list_node<uint32_t> n{p,NULL,NULL};
        L_in->push_back_node(&T_out->insert(&n)->v);
    }
    // append pair (n,0) so that interval_length() also works for the last pair
    pair_list_node<uint32_t> n{interv_pair<uint32_t>{8,0},NULL,NULL}; 
    L_in->push_back_node(&n);

    mdt->balance_upto(L_in,T_out,L_in->head(),L_in->tail()->pr,L_in->tail()->pr->v.second);

    // the output interval [0,3] should be balanced by creating the pair (6,2)
    // after that there is no unbalanced output interval
    interv_seq<uint32_t> I_expected = {
        interv_pair<uint32_t>(0,4),
        interv_pair<uint32_t>(1,5),
        interv_pair<uint32_t>(2,6),
        interv_pair<uint32_t>(3,7),
        interv_pair<uint32_t>(4,0),
        interv_pair<uint32_t>(6,2)
    };
    // check if L_in and T_out have been adjusted correctly
    EXPECT_EQ(L_in->size(),7);
    EXPECT_EQ(T_out->size(),6);
    auto it_L_in = L_in->iterator(L_in->head());
    for (auto p : I_expected) {
        EXPECT_EQ(it_L_in.current()->v,p);
        EXPECT_FALSE(T_out->find(it_L_in.current()) == NULL);
        if (it_L_in.has_next()) it_L_in.next();
    }
    EXPECT_EQ(it_L_in.current()->v,n.v);

    // andere Testfälle

    delete mdt;
    L_in->disconnect_from_nodes();
    delete L_in;
    delete T_out;
}

/*
    for (int j=0; j<k; j++) {
        int l = 0;
        int r = k-1;
        int m;
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
        int i = l;

        if (i+a-1 < k) {
            int q_j = D_pair[j].second;
            int d_j = D_pair[j+1].first - D_pair[j].first;

            int p_i = D_pair[i].first;
            int p_ipam1st = D_pair[i+a-1].first;

            EXPECT_FALSE(p_i < q_j + d_j && p_ipam1st < q_j + d_j);
        }
    }
*/