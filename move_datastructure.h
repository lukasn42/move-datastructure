#pragma once

#include "avl_tree.h"
#include "dl_list.h"

using interval_pair = std::pair<int,int>;
using list_node = dll_node<interval_pair>;

struct t_e_node {
    list_node *nodeInp;
    list_node *nodeOutp;

    t_e_node(list_node *nodeInp, list_node *nodeOutp);
    ~t_e_node();
};

class move_datastructure {
    private:
    int k;
    std::vector<interval_pair> D_pair; // stores the pairs (p_i, q_i)
    std::vector<int> D_index; // stores index j at i, if q_i is in the j-th input interval
    
    void build(std::vector<interval_pair> pairs);
    int intervalLength(list_node *pair); // returns the length of the input interval of a given pair
    list_node* has4IncEdges(avl_tree<list_node> *T_in, dl_list<interval_pair> *L_pairs, int l, int r);
    // in O(log k); checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
    // returns the pair associated with the first input interval in [l,r], if it has, else returns NULL
    bool has4IncEdges(dl_list<interval_pair> *L_pairs, list_node *nodeFirstInpInt, int r);
    // in O(1); checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
    // nodeFirstInpInt must contain the pair associated with the first input interval connected to [l,r]

    public:
    move_datastructure(std::vector<interval_pair> pairs);
    move_datastructure(int n, int k);
    interval_pair Move(int i, int x);
    void print();
};

int main(int argc, char *argv[]);