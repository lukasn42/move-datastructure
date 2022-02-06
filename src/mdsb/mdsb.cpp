#include <omp.h>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

#include "../../include/mdsb/mdsb.hpp"

extern "C" {
    #include "../../extern/malloc_count/malloc_count.h"
    #include "../../extern/malloc_count/malloc_count.c"
}

#include "../../include/misc/log.hpp"
#include "../misc/log.cpp"

#include "mdsb_v1.cpp"
#include "mdsb_v2_par.cpp"
#include "mdsb_v2_seq.cpp"
#include "mdsb_v2_v3_seq_par.cpp"
#include "mdsb_v3_par.cpp"
#include "mdsb_v3_seq.cpp"

template <typename T>
mdsb<T>::mdsb(mds<T> *md, interv_seq<T> *I, T n, T a, T b, int p, int v) {
    this->md = md;
    this->n = n;
    this->k = I->size();
    this->a = a;
    this->b = b;
    this->p = p;

    omp_set_num_threads(p);

    if (v == 1) {
        build_v1(I);
    } else {
        build_v2_v3(I,v);
    }
}

template <typename T>
mdsb<T>::~mdsb() {
    md = NULL;
}

template <typename T>
void mdsb<T>::build_v2_v3(interv_seq<T> *I, int v) {
    size_t baseline = malloc_count_current() - sizeof(I->at(0))*I->size();
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
    malloc_count_reset_peak();
    std::cout << std::endl;

    log_memory_usage<T>(baseline,"building L_in and T_out");

    build_lin_tout(I);
    
    time = log_runtime<T>(time);
    log_memory_usage<T>(baseline,"balancing");

    if (p > 1) {
        if (v == 2) {
            balance_v2_par();
        } else {
            balance_v3_par();
        }
    } else {
        if (v == 2) {
            balance_v2_seq();
        } else {
            balance_v3_seq();
        }
    }

    time = log_runtime<T>(time);
    log_memory_usage<T>(baseline,"building D_pair");
    
    build_dpair();

    time = log_runtime<T>(time);
    log_memory_usage<T>(baseline,"deleteing L_in and T_out");

    delete_lin_tout();

    time = log_runtime<T>(time);
    log_memory_usage<T>(baseline,"building D_index");

    build_dindex();

    time = log_runtime<T>(time);
    log_memory_usage<T>(baseline,"move datastructure built");

    std::cout << std::endl << "peak memory allocation during build: ~ " << (malloc_count_peak()-baseline)/1000000 << "MB" << std::endl;
    std::cout << std::endl;
}