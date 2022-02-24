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
#include "mdsb_v2_v3_v4_seq_par.cpp"
#include "mdsb_v3_par.cpp"
#include "mdsb_v3_seq.cpp"
#include "mdsb_v4_par.cpp"

template <typename T>
mdsb<T>::mdsb(mds<T> *md, interv_seq<T> *I, T n, T a, int p, int v, bool log) {
    this->md = md;
    this->n = n;
    this->k = I->size();
    this->a = a;
    this->p = p;

    omp_set_num_threads(p);

    if (v == 1) {
        build_v1(I,log);
    } else {
        build_v2_v3_v4(I,v,log);
    }
}

template <typename T>
mdsb<T>::~mdsb() {
    md = NULL;
}

template <typename T>
void mdsb<T>::build_v2_v3_v4(interv_seq<T> *I, int v, bool log) {
    size_t baseline;
    std::chrono::steady_clock::time_point time;

    if (log) {
        baseline = malloc_count_current() - sizeof(I->at(0))*I->size();
        time = std::chrono::steady_clock::now();
        malloc_count_reset_peak();
        std::cout << std::endl;
    }

    if (log) log_memory_usage<T>(baseline,"building L_in and T_out");

    build_lin_tout(I);
    
    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"balancing");
    }

    if (p > 1) {
        if (v == 2) {
            balance_v2_par();
        } else if (v == 3) {
            balance_v3_par();
        } else {
            balance_v4_par();
        }
    } else {
        if (v == 2) {
            balance_v2_seq();
        } else {
            balance_v3_seq();
        }
    }

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"building D_pair");
    }
    
    build_dpair();

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"deleteing L_in and T_out");
    }

    delete_lin_tout();

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"building D_index");
    }

    build_dindex();

    if (log) {
        time = log_runtime<T>(time);
        log_memory_usage<T>(baseline,"move datastructure built");
    }

    if (log) std::cout << std::endl << "peak memory allocation during build: ~ " << (malloc_count_peak()-baseline)/1000000 << "MB" << std::endl << std::endl;
}