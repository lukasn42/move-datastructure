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

#include "move_datastructure.h"
#include "move_datastructure.cpp"

extern "C" {
    #include "build/_deps/libsais-src/src/libsais.h"
    #include "build/_deps/libsais-src/src/libsais.c"
}

#include "build/_deps/malloc_count-src/memprofile.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "invalid input, usage: -textfile" << std::endl;
        return -1;
    }

    std::chrono::steady_clock::time_point time;

    // read file into text
    time = std::chrono::steady_clock::now();
    std::string text;
    std::string line;
    std::ifstream file(argv[1]);
    if (!file.good()) {
        std::cout << "invalid input: could not read textfile" << std::endl;
        return -1;
    }
    do {
        getline(file,line);
        if (!line.empty()) {
            text.append(line);
        }
    } while (!file.eof());
    file.close();
    text = text + (char) 32;
    // get text size
    int64_t n = text.size();
    // copy text into bwt
    uint8_t *T = new uint8_t[n];
    strcpy((char *) T,text.c_str());
    std::cout << "file read in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // calculate SA
    time = std::chrono::steady_clock::now();
    int32_t *SA = new int32_t[n];
    libsais(T,SA,n,0,NULL);
    std::cout << "SA calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // calculate bwt from T and SA
    time = std::chrono::steady_clock::now();
    uint8_t *bwt = new uint8_t[n];
    for (int i=0; i<n; i++) {
        if (*(SA+i) == 0) {
            *(bwt+i) = *(T+n-1);
        } else {
            *(bwt+i) = *(T+*(SA+i)-1);
        }
    }
    std::cout << "bwt calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // calculate rlbwt and count each characters number of occurences in occ
    time = std::chrono::steady_clock::now();
    std::vector<std::pair<uint32_t,uint8_t>> rlbwt; // rlbwt[i] = (l[i],L[i]), with i \in [0..r-1]
    rlbwt.reserve(n);
    std::vector<uint32_t> occ(256,0);
    uint8_t c = *bwt;
    rlbwt.push_back(std::make_pair(0,c));
    for (uint32_t i=1; i<n; i++) {
        if (*(bwt+i) != c) {
            occ[c] += i-rlbwt[rlbwt.size()-1].first;
            c = *(bwt+i);
            rlbwt.push_back(std::make_pair(i,c));
        }
    }
    rlbwt.shrink_to_fit();
    delete bwt;
    uint32_t r = rlbwt.size();
    occ[c] += n-rlbwt[r-1].first;
    // calculate C
    std::vector<uint32_t> C(256); // C[c] stores the number of occurences of characters c_ < c in bwt
    C[0] = 0;
    for (uint32_t i=1; i<256; i++) {
        C[i] = C[i-1] + occ[i-1];
    }
    std::cout << "rlbwt and C calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms";
    std::cout << ", n = " << n << ", r = " << r << ", r/n ~ " << std::ceil(r/(double) n * 1000.0)/1000.0 << std::endl;

    // calculate I_LF[0..r-1], with I_LF[i] = (l[i],LF(l[i])), with i \in [0..r-1]
    // and LF(l[i]) = C[c] + rank(bwt,c,l[i]) - 1 = C[c] + rank(bwt,c,l[i]-1) = C[c] + C_[c], with c = L[i]
    time = std::chrono::steady_clock::now();
    std::vector<uint32_t> C_(256,0); // at the start of an iteration C_[c] stores the number of occurences of c in bwt[0..l[i]-1], for all c \in [0..255] and i \in [0..r-1]
    auto *I_LF = new std::vector<std::pair<uint32_t,uint32_t>>(r);
    for (uint32_t i=0; i<r; i++) {
        c = rlbwt[i].second;
        I_LF->at(i) = std::make_pair(rlbwt[i].first,C[c]+C_[c]);
        C_[c] += rlbwt[i+1].first - rlbwt[i].first;
    }
    std::cout << "I_LF calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // build F_I_LF from I_LF
    //auto mp = new MemProfile("memprofile_I_LF.txt",0.1,1024);
    time = std::chrono::steady_clock::now();
    auto F_I_LF = move_datastructure<uint32_t>(I_LF,n,4,2);
    //delete mp;
    std::cout << "F_I_LF calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms";
    std::cout << ", k+t = " << F_I_LF.size() << ", (k+t)/k ~ " << std::ceil(F_I_LF.size()/(double) r * 1000.0)/1000.0 << std::endl;

    // calculate I_SA[0..r-1] with I_SA[i] = (u_i,phi^{-1}(u_i)) and u_i = SA[l[i]+|L_i|-1] and i \in [0..r-2] and u_{r-1} = SA[n-1]
    time = std::chrono::steady_clock::now();
    auto *I_SA = new std::vector<std::pair<uint32_t,uint32_t>>(r);
    for (uint32_t i=0; i<r-1; i++) {
        I_SA->at(i) = std::make_pair(*(SA+rlbwt[i+1].first-1),*(SA+rlbwt[i+1].first));
    }
    I_SA->at(r-1) = std::make_pair(*(SA+n-1),*SA);
    delete SA;
    std::cout << "I_SA calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // sort I_SA by u_i
    time = std::chrono::steady_clock::now();
    std::sort(I_SA->begin(),I_SA->end(),[](auto p1, auto p2){return p1.first < p2.first;});
    std::cout << "I_SA sorted in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms" << std::endl;

    // build F_I_SA from I_SA
    //mp = new MemProfile("memprofile_I_SA.txt",0.1,1024);
    time = std::chrono::steady_clock::now();
    auto F_I_SA = move_datastructure<uint32_t>(I_SA,n,4,2);
    //delete mp;
    std::cout << "F_I_SA calculated in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - time).count() << "ms";
    std::cout << ", k+t = " << F_I_SA.size() << ", (k+t)/k ~ " << std::ceil(F_I_SA.size()/(double) r * 1000.0)/1000.0 << std::endl;

    return 0;
}