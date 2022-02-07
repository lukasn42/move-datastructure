#include <iostream>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <functional>
#include <chrono>
#include <math.h>
#include <omp.h>

#include "../extern/ips4o/include/ips4o.hpp"

extern "C" {
    #include "../extern/libsais/src/libsais.h"
    #include "../extern/libsais/src/libsais.c"
}

#include "../include/mds.hpp"
#include "../src/mds.cpp"

std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time, std::string message) {
    std::cout << message << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-time).count() << " ms" << std::endl;
    return std::chrono::steady_clock::now();
}

void log_invalid_input() {
    std::cout << "invalid input, usage: a b p v file" << std::endl;
    std::cout << "    a: minimum number of incoming edges in the permutation graph, an output interval must have, to get cut, 4<=a" << std::endl;
    std::cout << "    b: number of remaining incoming edges after an output interval is cut, 2<=b<a-1" << std::endl;
    std::cout << "    p: number of threads to use (1 for v=1, 1<=p<=n for 2/3)" << std::endl;
    std::cout << "    v: build method version (1/2/3)" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        log_invalid_input();
        return -1;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int p = atoi(argv[3]);
    int v = atoi(argv[4]);

    if (!(
        2 <= b && b < a-1 &&
        1 <= p && p <= omp_get_max_threads() &&
        ((v == 1 && p == 1) || v == 2 || v == 3)
    )) {
        log_invalid_input();
        return -1;
    }

    omp_set_num_threads(p);
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();

    // read file into T
    std::ifstream file(argv[5]);
    if (!file.good()) {
        std::cout << "invalid input: could not read textfile" << std::endl;
        return -1;
    }
    file.seekg(0,std::ios::end);
    int32_t n = file.tellg()+(std::streamsize)+1;
    if (n < p) {
        std::cout << "invalid input: n < p" << std::endl;
        return -1;
    }
    file.seekg(0,std::ios::beg);
    uint8_t *T = new uint8_t[n];
    file.read((char*)T,n-1);
    file.close();
    *(T+n-1) = 0;
    time = log_runtime(time,"file read");

    // calculate SA
    int32_t *SA = new int32_t[n];
    if (p > 1) {
        libsais_omp(T,SA,n,0,NULL,p);
    } else {
        libsais(T,SA,n,0,NULL);
    }
    time = log_runtime(time,"SA calculated in");

    // calculate rlbwt and count each characters number of occurences in occ
    std::vector<std::pair<uint32_t,uint8_t>> rlbwt; // rlbwt[i] = (l[i],L[i]), with i \in [0..r-1]
    rlbwt.reserve(n);
    std::vector<uint32_t> occ(256,0);
    uint8_t c = *SA == 0 ? *(T+n-1) : *(T+*SA-1);
    uint8_t c_;
    rlbwt.push_back(std::make_pair(0,c));
    for (int i=1; i<n; i++) {
        c_ = *(SA+i) == 0 ? *(T+n-1) : *(T+*(SA+i)-1);
        if (c_ != c) {
            occ[c] += i-rlbwt[rlbwt.size()-1].first;
            c = c_;
            rlbwt.push_back(std::make_pair(i,c));
        }
    }
    delete T;
    T = NULL;
    rlbwt.shrink_to_fit();
    uint32_t r = rlbwt.size();
    occ[c] += n-rlbwt[r-1].first;
    // calculate C
    std::vector<uint32_t> C(256); // C[c] stores the number of occurences of characters c_ < c in bwt
    C[0] = 0;
    for (uint32_t i=1; i<256; i++) {
        C[i] = C[i-1] + occ[i-1];
    }
    std::cout << "n = " << n << ", r = " << r << ", r/n ~ " << std::ceil(r/(double) n * 1000.0)/1000.0 << std::endl;
    time = log_runtime(time,"rlbwt and C calculated");

    {
        // calculate I_SA[0..r-1] with I_SA[i] = (SA[r_i],phi(SA[r_i])) = (SA[r_i],SA[r_i-1]) and i \in [0..r-1]
        std::vector<std::pair<uint32_t,uint32_t>> *I_SA = new std::vector<std::pair<uint32_t,uint32_t>>(r);
        I_SA->at(0) = std::make_pair(*SA,*(SA+n-1));
        #pragma omp parallel for num_threads(p)
        for (uint32_t i=1; i<r; i++) {
            I_SA->at(i) = std::make_pair(*(SA+rlbwt[i].first),*(SA+rlbwt[i].first-1));
        }
        delete SA;
        SA = NULL;
        time = log_runtime(time,"I_SA calculated");

        // sort I_SA by input interval starting position
        auto comp = [](auto p1, auto p2){return p1.first < p2.first;};
        if (p > 1) {
            ips4o::parallel::sort(I_SA->begin(),I_SA->end(),comp);
        } else {
            ips4o::sort(I_SA->begin(),I_SA->end(),comp);
        }
        time = log_runtime(time,"I_SA sorted");
        
        // build F_I_SA from I_SA
        mds<uint32_t> F_I_SA(I_SA,n,a,b,p,v,true);
        I_SA = NULL;
        time = log_runtime(time,"F_I_SA calculated");
        std::cout << "k+t = " << F_I_SA.intervals() << ", (k+t)/k ~ " << std::ceil(F_I_SA.intervals()/(double) r * 1000.0)/1000.0 << std::endl;
    }

    // calculate I_LF[0..r-1], with I_LF[i] = (l[i],LF(l[i])), with i \in [0..r-1]
    // and LF(l[i]) = C[c] + rank(bwt,c,l[i]) - 1 = C[c] + rank(bwt,c,l[i]-1) = C[c] + C_[c], with c = L[i]
    // at the start of an iteration C_[c] stores the number of occurences of c in bwt[0..l[i]-1], for all c \in [0..255] and i \in [0..r-1]
    std::vector<uint32_t> C_(256,0);
    std::vector<std::pair<uint32_t,uint32_t>> *I_LF = new std::vector<std::pair<uint32_t,uint32_t>>(r);
    for (uint32_t i=0; i<r; i++) {
        c = rlbwt[i].second;
        I_LF->at(i) = std::make_pair(rlbwt[i].first,C[c]+C_[c]);
        C_[c] += rlbwt[i+1].first - rlbwt[i].first;
    }
    time = log_runtime(time,"I_LF calculated");

    // build F_I_LF from I_LF
    mds<uint32_t> F_I_LF(I_LF,n,a,b,p,v,true);
    I_LF = NULL;
    time = log_runtime(time,"F_I_LF calculated");
    std::cout << "k+t = " << F_I_LF.intervals() << ", (k+t)/k ~ " << std::ceil(F_I_LF.intervals()/(double) r * 1000.0)/1000.0 << std::endl;
    uint32_t r_o = r;
    r = F_I_LF.intervals();

    // build bwtr with the newly added runs
    std::vector<uint8_t> bwtr(r);
    uint32_t i_o = 0;
    for (uint32_t i=0; i<r; i++) {
        if (i_o < r_o && F_I_LF.pair(i).first >= rlbwt[i_o+1].first) {
            i_o++;
        }
        bwtr[i] = rlbwt[i_o].second;
    }
    time = log_runtime(time,"bwtr built");
    
    // reconstruct T
    std::vector<uint8_t> T_new(n);
    T_new[n-1] = 0;
    std::pair<uint32_t,uint32_t> move_pair = std::make_pair(0,0);
    for (int i=n-2; i>=0; i--) {
        T_new[i] = bwtr[move_pair.second];
        F_I_LF.move(move_pair);
    }
    time = log_runtime(time,"T reconstructed");

    return 0;
}