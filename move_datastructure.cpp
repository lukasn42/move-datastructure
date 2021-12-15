#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

#include "avl_tree.cpp"

struct pairP {
    int p;
    int q;
    operator std::string() const {
        return "(" + std::to_string(p) + "," + std::to_string(q) + ")";
    }
    bool operator==(pairP otherpairP) {
        return otherpairP.p == p;
    };
    bool operator<(pairP otherpairP) {
        return otherpairP.p > p;
    };
    bool operator>(pairP otherpairP) {
        return !operator<(otherpairP) && !operator==(otherpairP);
    }
    bool operator>=(pairP otherpairP) {
        return !operator<(otherpairP);
    }
    bool operator<=(pairP otherpairP) {
        return !operator>(otherpairP);
    }
};

struct pairQ {
    int p;
    int q;
    bool operator==(pairQ otherpairQ) {
        return otherpairQ.q == q;
    };
    bool operator<(pairQ otherpairQ) {
        return otherpairQ.q > q;
    };
    bool operator>(pairQ otherpairQ) {
        return !operator<(otherpairQ) && !operator==(otherpairQ);
    }
    bool operator>=(pairQ otherpairQ) {
        return !operator<(otherpairQ);
    }
    bool operator<=(pairQ otherpairQ) {
        return !operator>(otherpairQ);
    }
};

class move_datastructure {
    public:
    int k;
    std::vector<std::pair<int,int>> D_pair; // stores the pairs (p_i, q_i)
    std::vector<int> D_index; // stores index j at i, if q_i is in the j-th input interval

    avl_tree<pairP> T_in = avl_tree<pairP>();
    avl_tree<pairQ> T_out = avl_tree<pairQ>();
    avl_tree<pairP> T_e = avl_tree<pairP>();
    
    private:
    int build(std::vector<std::pair<int,int>> pairs) {
        for (auto p : pairs) {
            T_in.insert(pairP {p.first,p.second});
            T_out.insert(pairQ {p.first,p.second});
        }
        for (int i=0; i<k; i++) {
            int p_i = pairs[i].first;
            int q_i = pairs[i].second;
            int q_next = q_i+pairs[i+1].first-p_i;
            if (has4IncEdges(q_i,q_next)) {
                T_e.insert(pairP {p_i,q_i});
            }
        }

        k = T_in.size()-1;
        D_pair.resize(k);
        D_index.resize(k);
        avl_node<pairP>* n = T_in.minimum();
        for (int i=0; i<k; i++) {
            D_pair[i] = std::make_pair<int,int> ((int) n->v.p,(int) n->v.q);
            D_index[i] = 0;
            n = T_in.next(n);
        }

        print();

        while(!T_e.isEmpty()) {
            avl_node<pairP>* nodeJ = T_e.minimum();
            avl_node<pairQ>* nodeJ_ = T_out.find(pairQ {nodeJ->v.p,nodeJ->v.q});
            avl_node<pairP>* nodeX = T_in.minElemNotLessThan(pairP {nodeJ->v.q});
            int d = T_in.next(nodeX)->v.p-nodeJ->v.q+1;
            int d_j = T_out.next(nodeJ_)->v.p-nodeJ_->v.p;

            T_in.insert(pairP {nodeJ->v.p+d,nodeJ->v.q+d});
            T_out.insert(pairQ {nodeJ->v.p+d,nodeJ->v.q+d});

            // (i)
            T_e.remove(nodeJ->v);

            // (ii)
            avl_node<pairQ>* nodeY = T_out.maxElemNotGreaterThan(pairQ {0,nodeJ_->v.p+d});

            // (iii)
            std::vector<std::pair<int,int>> IntervalsToCheck = {
                std::make_pair<int,int> ((int) nodeJ_->v.q,     (int) nodeJ_->v.q+d-1           ),
                std::make_pair<int,int> ((int) nodeJ_->v.q+d,   (int) nodeJ_->v.q+d_j-1         ),
                std::make_pair<int,int> ((int) nodeY->v.q,      (int) T_out.next(nodeY)->v.q-1  )
            };
            for(std::pair<int,int> interval : IntervalsToCheck) {
                if (has4IncEdges(interval.first, interval.second)) {
                    T_e.insert(pairP {interval.first,interval.second});
                }
            }
        }

        k = T_in.size()-1;
        D_pair.resize(k);
        D_index.resize(k);
        n = T_in.minimum();
        for (int i=0; i<k; i++) {
            D_pair[i] = std::make_pair<int,int> ((int) n->v.p,(int) n->v.q);
            D_index[i] = 0;
            n = T_in.next(n);
        }

        print();

        return 0;
    }

    bool has4IncEdges(int q_1, int q_2) {
        avl_node<pairP>* first = T_in.minElemNotLessThan(pairP {q_1});
        if (first->v.p < q_2) {
            avl_node<pairP>* fourth = T_in.next(T_in.next(T_in.next(first)));
            if (fourth != NULL && fourth->v.p < q_2) {
                return true;
            }
        }
        return false;
    }

    public:
    move_datastructure(std::vector<std::pair<int,int>> pairs) {
        k = pairs.size()-1;
        build(pairs);
    }

    move_datastructure(int n, int k) {
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

    std::pair<int,int> Move(int i, int x) {
        int i_ = D_pair[x].second+(i-D_pair[x].first);
        int x_ = D_index[x];
        while (i_ >= D_pair[x_+1].first) x_++;
        return std::make_pair<int,int> ((int) i_,(int) x_);
    }

    int print() {
        std::cout << "k = " << k << std::endl;

        if (k <= 10) {
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

        return 0;
    }
};

int main(int argc, char *argv[]) {
    if (argc != 3 || !isdigit(*argv[1]) || !isdigit(*argv[2])) {
        std::cout << "invalid input; usage: move_datastructure -n -k (n: max value, k: number of intervals)" << std::endl;
        return -1;
    }

    /*
    int n = std::stoi(argv[1]);
    int k = std::stoi(argv[2]);
    auto md = move_datastructure(n,k);
    */

    std::vector<std::pair<int,int>> D_pair = {
        std::make_pair<int,int> (0,9),
        std::make_pair<int,int> (1,10),
        std::make_pair<int,int> (2,11),
        std::make_pair<int,int> (6,0),
        std::make_pair<int,int> (13,7),
        std::make_pair<int,int> (15,15)
    };
    auto md = move_datastructure(D_pair);
    
    /*
    int i = 2;
    int x = 2;
    auto p = md.Move(i,x);
    std::cout << std::endl << "Move(" << i << "," << x << ") = (" << p.first << "," << p.second << ")" << std::endl;
    */
   
    return 0;
}