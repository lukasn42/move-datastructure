#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <functional>

#include "move_datastructure.h"

template class dll_node<interval_pair>;
template class dl_list<interval_pair>;

template struct avl_node<dll_node<interval_pair>>;
template class avl_tree<dll_node<interval_pair>>;

template class avl_node<t_e_node>;
template class avl_tree<t_e_node>;

t_e_node::t_e_node(list_node *nodeInp, list_node *nodeOutp) {
    this->nodeInp = nodeInp;
    this->nodeOutp = nodeOutp;
};

t_e_node::~t_e_node() {
    nodeInp = nodeOutp = NULL;
};

void move_datastructure::build(std::vector<interval_pair> pairs) {
    avl_tree<list_node> T_in( // contains all pairs ordered by p_i
        [](list_node *n1, list_node *n2){return n1->v->first < n2->v->first;},
        [](list_node *n1, list_node *n2){return n1->v->first > n2->v->first;},
        [](list_node *n1, list_node *n2){return n1->v->first == n2->v->first;}
    );
    avl_tree<list_node> T_out( // contains all pairs ordered by q_i
        [](list_node *n1, list_node *n2){return n1->v->second < n2->v->second;},
        [](list_node *n1, list_node *n2){return n1->v->second > n2->v->second;},
        [](list_node *n1, list_node *n2){return n1->v->second == n2->v->second;}
    );
    
    // contains pairs (list_node p1, list_node p2), where p2 is associated with an output interval with at least 4 incoming edges in the permutation graph
    // p1 is the pair associated with the first input interval in the output interval associated with p2
    // the pairs are ordered by the starting position of the output interval associated with p2
    avl_tree<t_e_node> T_e(
        [](t_e_node *n1, t_e_node *n2){return n1->nodeOutp->v->first < n2->nodeOutp->v->first;},
        [](t_e_node *n1, t_e_node *n2){return n1->nodeOutp->v->first > n2->nodeOutp->v->first;},
        [](t_e_node *n1, t_e_node *n2){return n1->nodeOutp->v->first == n2->nodeOutp->v->first;}
    );

    // stores all pairs ordered by the starting position of their input interval
    dl_list<interval_pair> L_pairs;

    k = pairs.size()-1;
    std::cout << "k = " << k << std::endl;

    // insert all input pairs as nodes of a doubly linked list into the avl trees T_in and T_out
    for (int i=0; i<k; i++) { // in O(k log k)
        auto *n = L_pairs.pushBack(&(pairs[i]));
        T_in.insert(n,false); // in O(log k)
        T_out.insert(n,false); // in O(log k)
    }
    L_pairs.pushBack(&(pairs[k]));

    // insert the pairs into T_e, whiches corresponding output intervals have more than 4 incoming edges
    auto *nodeCur = L_pairs.head();
    while (nodeCur->sc != NULL) { // in O(k log k)
        int q_i = nodeCur->v->second;
        int q_next = q_i + intervalLength(nodeCur); // q_next = q_i + d_i = q_i + p_{i+1} - p_i
        list_node *nodeFir = has4IncEdges(&T_in,&L_pairs,q_i,q_next-1); // in O(log k)
        if (nodeFir != NULL) {
            T_e.insert(new t_e_node(nodeFir,nodeCur),true); // in O(log k)
        }
        nodeCur = nodeCur->sc;
    }

    std::cout << "|T_e| = " << T_e.size() << std::endl;

    while(!T_e.isEmpty()) { // in O(k log k)
        // find output interval to be cut
        // find first pair (p_j, q_j) associated with the output interval [q_j, q_j + d_j - 1]
        // that has at least 4 incoming edges in the permutation graph
        auto *min = T_e.removeMin(); // in O(log k)
        auto *nodePairX = min->nodeInp;
        auto *nodePairJ = min->nodeOutp;
        delete min;

        // find first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
        int p_j = nodePairJ->v->first;
        int q_j = nodePairJ->v->second;
        int d_j = intervalLength(nodePairJ);

        // determine interval cut position
        // d_1 = p_{x+2} - q_j is the largest integer, so that [q_j,q_j + d_1 - 1] has 2 incoming edges
        auto *nodePairXp2 = nodePairX->sc->sc;
        int d_1 = nodePairXp2->v->first - q_j;

        // create pair (p_j + d_1, q_j + d_1) and link it between (p_j, q_j) and (p_{j+1}, q_{j+1})
        auto *nodePairNew = L_pairs.insertAfter(nodePairJ,new interval_pair (p_j + d_1, q_j + d_1));
        // insert it's node in L_pairs into the avl trees
        T_in.insert(nodePairNew,false); // in O(log k)
        T_out.insert(nodePairNew,false); // in O(log k)

        // now the output interval [q_j,q_j + d_j - 1] does not exist anymore
        // and the output intervals [q_j,q_j + d_1 - 1] and [q_j + d_1, q_j + d_j - 1] were created

        // now the output interval, p_j + d_1 starts in, possibly has more than 3 incoming edges
        // find the output interval [q_y, q_y + d_y - 1] containing p_j + d_1 associated with the pair (p_y, q_y)
        interval_pair pairComp {0, p_j + d_1};
        list_node nodeComp {&pairComp};
        auto *nodePairY = T_out.maxElemLessOrEqual(&nodeComp); // in O(log k)
        int q_y = nodePairY->v->second;
        int d_y = intervalLength(nodePairY);

        // find the first input interval in the output interval [q_y, q_y + d_y - 1]
        auto *nodePairZ = nodePairNew;
        while (nodePairZ->pr != NULL && nodePairZ->pr->v->first >= q_y) { // in O(1)
            nodePairZ = nodePairZ->pr;
        }

        // check if [q_y, q_y + d_y - 1] has more than 3 incoming edges
        // and insert ((p_j + d_1, q_j + d_1), (p_y, q_y)) into T_e if it does, since it suffices the requirements of pairs in T_e
        if (has4IncEdges(&L_pairs,nodePairZ,q_y + d_y - 1)) { // in O(1)
            auto *newNode = new t_e_node(nodePairZ,nodePairY);
            T_e.insert(newNode,true); // in O(log k)
        }

        // the newly added output interval [q_j + d_1, q_j + d_j - 1] associated with the pair (p_j + d_1, q_j + d_1)
        // possibly has more than 4 incoming edges as well, if [q_j, q_j + d_j - 1] had at least 6 incoming edges in the permutation graph
        // before this iteraton or if it equals [q_y, q_y + d_y - 1], but then it has already been dealt with
        if (q_y != q_j + d_1 && has4IncEdges(&L_pairs,nodePairXp2,q_j + d_j - 1)) { // in O(1)
            auto *newNode = new t_e_node(nodePairXp2,nodePairNew);
            T_e.insert(newNode,true); // in O(log k)
        }
    }

    k = L_pairs.size()-1;

    // build D_pair
    D_pair.resize(k+1);
    nodeCur = L_pairs.head();
    for (int i=0; i<k; i++) { // in O(k)
        D_pair[i] = *nodeCur->v;
        nodeCur = nodeCur->sc;
    }
    D_pair[k] = *nodeCur->v;

    // build D_pair
    D_index.resize(k);
    for (int i=0; i<k; i++) { // in O(k log k)
        // find the input interval [p_j, p_j + d_j - 1], q_i is in, in O(log k)
        // find the maximum integer j \in [0,k-1], so that D_pair[j].first >= D_pair[i].second
        int l = 0;
        int r = k-1;
        int m;
        while (l != r) {
            m = (l+r)/2+1;
            if (D_pair[m].first == D_pair[i].second) {
                l = r = m;
            } else if (D_pair[m].first > D_pair[i].second) {
                r = m-1;
            } else {
                l = m;
            }
        }
        D_index[i] = D_pair[m].first;
    }

    print();
}

int move_datastructure::intervalLength(list_node *p) { // returns the length of the input interval of a given pair
    return p->sc->v->first - p->v->first;
}

// checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
// returns the pair associated with the first input interval in [l,r], if it has, else returns NULL
list_node* move_datastructure::has4IncEdges(avl_tree<list_node> *T_in, dl_list<interval_pair> *L_pairs, int l, int r) { // in O(log k)
    if (r-l < 3) { // if |[l,r]| <= 3, there cannot be 4 intervals starting in [l,r]
        return NULL;
    }
    interval_pair pairComp {l,0};
    list_node nodeComp {&pairComp};
    auto *n = T_in->minElemGreaterOrEqual(&nodeComp); // in O(log k)
    if (n != NULL && n->v->first <= r && has4IncEdges(L_pairs,n,r)) {
        return n;
    }
    return NULL;
}

// checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
// nodeFirstInpInt must contain the pair associated with the first input interval connected to [l,r]
bool move_datastructure::has4IncEdges(dl_list<interval_pair> *L_pairs, list_node *nodeFirstInpInt, int r) { // in O(1)
    auto *nodeFourthInpInt = L_pairs->iterateSuccessor(nodeFirstInpInt,3);
    return nodeFourthInpInt != NULL && nodeFourthInpInt->v->first <= r;
}

move_datastructure::move_datastructure(std::vector<interval_pair> pairs) {
    k = pairs.size()-1;
    build(pairs);
}

move_datastructure::move_datastructure(int n, int k) { // generates k random pairs with maximum value n, in O(n log n)
    if (k <= 0) {
        std::cout << "invid number of intervals (k <= 0)" << std::endl;
        return;
    }
    if (n < k) {
        std::cout << "invid max vue (n < k)" << std::endl;
        return;
    }
    this->k = k;

    // fill p[1..k-1] with k-1 sorted values in the range [1..n-1] and p[0] = 0, p[k] = n
    std::vector<interval_pair> pairs(n);
    for (int i=1; i<n; i++) {
        pairs[i].first = i;
    }
    std::random_shuffle(pairs.begin()+1,pairs.end());
    pairs.resize(k+1);
    pairs[0].first = 0;
    pairs[k].first = n;
    std::sort(pairs.begin()+1,pairs.end()-1);

    // generate random permutation pi of the values [0..k-1]
    std::vector<int> pi(k);
    for (int i=0; i<k; i++) {
        pi[i] = i;
    }
    std::random_shuffle(pi.begin(),pi.end());
    
    // calculate output interval start positions according to pi
    pairs[pi[0]].second = 0;
    pairs[k].second = n;
    for (int i=1; i<k; i++) {
        pairs[pi[i]].second = pairs[pi[i-1]].second+pairs[pi[i-1]+1].first-pairs[pi[i-1]].first;
    }

    build(pairs);
}

interval_pair move_datastructure::Move(int i, int x) { // in O(1)
    int i_ = D_pair[x].second+(i-D_pair[x].first);
    int x_ = D_index[x];
    while (i_ < D_pair[x_].first) x_++;
    return interval_pair {i_,x_};
}

void move_datastructure::print() {
    std::cout << "k = " << k << std::endl;

    if (k <= 20) {
        std::cout << std::endl;

        std::cout << "p = (";
        for (int i=0; i<k-1; i++) std::cout << D_pair[i].first << ",";
        std::cout << D_pair[k-1].first << ")" << std::endl;

        std::cout << "q = (";
        for (int i=0; i<k-1; i++) std::cout << D_pair[i].second << ",";
        std::cout << D_pair[k-1].second << ")" << std::endl;

        std::cout << "index = (";
        for (int i=0; i<k-1; i++) std::cout << D_index[i] << ",";
        std::cout << D_index[k-1] << ")" << std::endl;

        std::cout << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3 || !isdigit(*argv[1]) || !isdigit(*argv[2])) {
        std::cout << "invalid input; usage: move_datastructure -n -k (n: max value, k: number of intervals)" << std::endl;
        return -1;
    }

    
    int n = std::stoi(argv[1]);
    int k = std::stoi(argv[2]);
    auto md = move_datastructure(n,k);
    
    /*
    std::vector<interval_pair> D_pair = {
        new interval_pair(0,9),
        new interval_pair(1,10),
        new interval_pair(2,11),
        new interval_pair(6,0),
        new interval_pair(13,7),
        new interval_pair(15,15)
    };
    auto md = move_datastructure(D_pair);
    
    int i = 2;
    int x = 2;
    auto p = md.Move(i,x);
    std::cout << std::endl << "Move(" << i << "," << x << ") = (" << p.first << "," << p.second << ")" << std::endl;
    */
   
    return 0;
}