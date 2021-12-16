#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <functional>

#include "avl_tree.cpp"
#include "dl_list.cpp"

using pair_node = dll_node<std::pair<int,int>>;
using te_node = std::pair<pair_node*,pair_node*>;

class move_datastructure {
    public:
    int k;
    std::vector<std::pair<int,int>> D_pair; // stores the pairs (p_i, q_i)
    std::vector<int> D_index; // stores index j at i, if q_i is in the j-th input interval

    std::function<bool(pair_node*,pair_node*)> ltP = [](auto *n1, auto *n2){return n1->val->first < n2->val->first;};
    std::function<bool(pair_node*,pair_node*)> gtP = [](auto *n1, auto *n2){return n1->val->first > n2->val->first;};
    std::function<bool(pair_node*,pair_node*)> eqP = [](auto *n1, auto *n2){return n1->val->first == n2->val->first;};

    std::function<bool(pair_node*,pair_node*)> ltQ = [](auto *n1, auto *n2){return n1->val->second < n2->val->second;};
    std::function<bool(pair_node*,pair_node*)> gtQ = [](auto *n1, auto *n2){return n1->val->second > n2->val->second;};
    std::function<bool(pair_node*,pair_node*)> eqQ = [](auto *n1, auto *n2){return n1->val->second == n2->val->second;};

    avl_tree<pair_node> T_in = avl_tree<pair_node>(ltP,gtP,eqP); // contains all pairs ordered by p_i
    avl_tree<pair_node> T_out = avl_tree<pair_node>(ltQ,gtQ,eqQ); // contains all pairs ordered by q_i

    std::function<bool(te_node*,te_node*)> ltP_ = [](auto *n1, auto *n2){return n1->second->val->first < n2->second->val->first;};
    std::function<bool(te_node*,te_node*)> gtP_ = [](auto *n1, auto *n2){return n1->second->val->first > n2->second->val->first;};
    std::function<bool(te_node*,te_node*)> eqP_ = [](auto *n1, auto *n2){return n1->second->val->first == n2->second->val->first;};
    
    // contains pairs (pair_node p1, pair_node p2), where p2 is associated with an output interval with at least 4 incoming edges in the permutation graph
    // p1 is the pair associated with the first input interval in the output interval associated with p2
    // the pairs are ordered by the starting position of the output interval associated with p2
    avl_tree<te_node> T_e = avl_tree<te_node>(ltP_,gtP_,eqP_);

    dl_list<std::pair<int,int>> L_pairs = *new dl_list<std::pair<int,int>>();
    
    private:
    void build(std::vector<std::pair<int,int>> pairs) {
        k = pairs.size()-1;
        
        // insert all input pairs as nodes of a doubly linked list into the avl trees T_in and T_out
        for (int i=0; i<k; i++) { // in O(k)
            auto *node = L_pairs.pushBack(&(pairs[i]));
            T_in.insert(node);
            T_out.insert(node);
        }
        L_pairs.pushBack(&(pairs[k]));

        // insert the pairs into T_e, whiches corresponding output intervals have more than 4 incoming edges
        auto *pairCur = L_pairs.getHead();
        while (pairCur->succ != NULL) { // in O(k log k)
            int q_i = pairCur->val->second;
            int q_next = q_i + pairCur->succ->val->first - pairCur->val->first; // q_next = q_i + d_i = q_i + p_{i+1} - p_i
            pair_node *first = has4IncEdges(q_i,q_next-1); // in O(log k)
            if (first != NULL) { 
                T_e.insert(new te_node(first,pairCur));
            }
            pairCur = pairCur->succ;
        }

        std::cout << "|T_e| = " << T_e.size() << std::endl;

        while(!T_e.isEmpty()) { // in O(k log k)
            // find output interval to be cut
            // find first pair (p_j, q_j) associated with the output interval [q_j, q_j + d_j - 1]
            // that has at least 4 incoming edges in the permutation graph
            auto *min = T_e.removeMin(); // in O(log k)
            auto *nodePairJ = min->second;
            int p_j = nodePairJ->val->first;
            int q_j = nodePairJ->val->second;
            int d_j = nodePairJ->succ->val->first - p_j;

            // find first input interval connected to [q_j, q_j + d_j - 1] in the permutation graph
            auto *nodePairX = min->first;
            // the current pair's input interval in nodePairX starts in [q_j, q_j + d_j - 1], but it is possible that
            // there are input intervals which were created that start between it and q_j
            // if they exist, they are directly before nodePairX in L_pairs
            while (nodePairX->pred != NULL && nodePairX->pred->val->first >= q_j) { // in O(1)
                nodePairX = nodePairX->pred;
            }

            // determine interval cut position
            // d_1 = p_{x+2} - q_j is the largest integer, so that [q_j,q_j + d_1 - 1] has 2 incoming edges
            auto *nodeXp2 = nodePairX->succ->succ;
            int p_xp2 = nodeXp2->val->first;
            int d_1 = p_xp2 - q_j;

            // create pair (p_j + d_1, q_j + d_1)
            auto *pairNew = new std::pair<int,int>(p_j + d_1, q_j + d_1);
            // link it between (p_j, q_j) and (p_{j+1}, q_{j+1})
            auto *nodePairNew = L_pairs.insertAfter(nodePairJ,pairNew);
            // insert it's node in L_pairs into the avl trees
            T_in.insert(nodePairNew);
            T_out.insert(nodePairNew);

            // now the output interval [q_j,q_j + d_j - 1] does not exit anymore
            // and the output intervals [q_j,q_j + d_1 - 1] and [q_j + d_1, q_j + d_1 + d_2 - 1] were created, with d_2 = d_j - d_1
            int d_2 = d_j - d_1;

            // now the output interval, p_j + d_1 starts in, possibly has more incoming edges than before
            // find the output interval [q_y, q_y + d_y - 1] containing p_j + d_1 associated with the pair (p_y, q_y)
            auto *nodePairY = T_out.maxElemLessOrEqual(new pair_node {new std::pair<int,int>(0, p_j + d_1)})->v; // in O(log k)
            int p_y = nodePairY->val->first;
            int q_y = nodePairY->val->second;
            int d_y = nodePairY->succ->val->first - p_y;

            // check if [q_y, q_y + d_y - 1] has more than 3 incoming edges
            // and insert ((p_j + d_1, q_j + d_1), (p_y, q_y)) into T_e if it does, since it suffices the requirements of pairs in T_e
            if (has4IncEdges(nodePairNew,q_y + d_y - 1)) { // in O(1)
                T_e.insert(new te_node(nodePairNew,nodePairY)); // in O(log k)
            }

            // the newly added output interval [q_j + d_1, q_j + d_1 + d_2 - 1] associated with the pair (p_j + d_1, q_j + d_1)
            // possibly has more than 4 incoming edges as well, if [q_j, q_j + d_j - 1] had at least 6 incoming edges in the permutation graph
            // before this iteraton or if it equals [q_y, q_y + d_y - 1], but then it has already been dealt with
            if (q_y != q_j + d_1 && has4IncEdges(nodeXp2,q_j + d_j - 1)) {
                T_e.insert(new te_node(nodeXp2,nodePairNew)); // in O(log k)
            }
        }

        k = T_in.size();
        
        // build D_pair
        D_pair.resize(k);
        pairCur = L_pairs.getHead();
        for (int i=0; i<k; i++) { // in O(k)
            D_pair[i] = *pairCur->val;
            pairCur = pairCur->succ;
        }

        // build D_pair
        pairCur = L_pairs.getHead();
        D_index.resize(k);
        for (int i=0; i<k; i++) { // in O(k log k)
            D_index[i] = inputInterval(pairCur->val->second); // find the input interval [p_j,p_j+d_j-1], q_i is in, in O(log k)
            pairCur = pairCur->succ;
        }

        print();
    }

    int lenInpInt(pair_node *pair) { // returns the length of the input interval of a given pair
        return pair->succ->val->first - pair->val->first;
    }

    int inputInterval(int q_i) { // returns maximum integer i, so that p_i <= q_j, in O(k)
        return binSearchMaxLessOrEqual(q_i,0,k-1);
    }

    int binSearchMaxLessOrEqual(int x, int l, int r) { // returns the greatest position y \in [l,r] with value <= x, in O(r-l+1)
        int m = (l+r)/2+1;
        if (l == r) {
            return l;
        } else if (D_pair[m].first > x) {
            return binSearchMaxLessOrEqual(x,l,m-1);
        } else {
            return binSearchMaxLessOrEqual(x,m,r);
        }
    }

    // checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
    // returns the pair associated with the first input interval in [l,r], if it has, else returns NULL
    pair_node* has4IncEdges(int l, int r) { // in O(log k)
        if (r-l < 3) { // if |[l,r]| <= 3, there cannot be 4 intervals starting in [l,r]
            return NULL;
        }
        auto *node = T_in.minElemGreaterOrEqual(new pair_node {new std::pair<int,int>((int) l,0)}); // in O(log k)
        if (node != NULL && node->v->val->first <= r && has4IncEdges(node->v,r)) {
            return node->v;
        }
        return NULL;
    }

    // checks if the output interval [l,r] has at least 4 incoming edges in the permutation graph
    // nodeFirstInpInt must contain the pair associated with the first input interval connected to [l,r]
    bool has4IncEdges(pair_node *nodeFirstInpInt, int r) { // in O(1)
        auto *nodeFourthInpInt = L_pairs.ithSucc(nodeFirstInpInt,3);
        return nodeFourthInpInt != NULL && nodeFourthInpInt->val->first <= r;
    }

    public:
    move_datastructure(std::vector<std::pair<int,int>> pairs) {
        k = pairs.size()-1;
        build(pairs);
    }

    move_datastructure(int n, int k) { // generates k random pairs with maximum value n, in O(n log n)
        if (k <= 0) {
            std::cout << "invalid number of intervals (k <= 0)" << std::endl;
            return;
        }
        if (n < k) {
            std::cout << "invalid max value (n < k)" << std::endl;
            return;
        }
        this->k = k;

        // fill p[1..k-1] with k-1 sorted values in the range [1..n-1] and p[0] = 0, p[k] = n
        std::vector<std::pair<int,int>> pairs(n);
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

    std::pair<int,int> Move(int i, int x) { // in O(1)
        int i_ = D_pair[x].second+(i-D_pair[x].first);
        int x_ = D_index[x];
        while (i_ >= D_pair[x_+1].first) x_++;
        return std::make_pair<int,int> ((int) i_,(int) x_);
    }

    void print() {
        std::cout << "k = " << k << std::endl;

        if (k <= 200) {
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
};

int main(int argc, char *argv[]) {
    if (argc != 3 || !isdigit(*argv[1]) || !isdigit(*argv[2])) {
        std::cout << "invalid input; usage: move_datastructure -n -k (n: max value, k: number of intervals)" << std::endl;
        return -1;
    }

    
    int n = std::stoi(argv[1]);
    int k = std::stoi(argv[2]);
    auto md = move_datastructure(n,k);
    
    /*
    std::vector<std::pair<int,int>> D_pair = {
        std::make_pair<int,int> (0,9),
        std::make_pair<int,int> (1,10),
        std::make_pair<int,int> (2,11),
        std::make_pair<int,int> (6,0),
        std::make_pair<int,int> (13,7),
        std::make_pair<int,int> (15,15)
    };
    auto md = move_datastructure(D_pair);
    
    int i = 2;
    int x = 2;
    auto p = md.Move(i,x);
    std::cout << std::endl << "Move(" << i << "," << x << ") = (" << p.first << "," << p.second << ")" << std::endl;
    */
   
    return 0;
}