#include <omp.h>
#include <chrono>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <functional>

#include <mdsb.hpp>

extern "C" {
    #include <malloc_count.h>
}

#include <log.hpp>
#include <log.cpp>

#include "mdsb_v1.cpp"
#include "mdsb_v2.cpp"
#include "mdsb_v2_v3_v4_seq_par.cpp"
#include "mdsb_v3_par.cpp"
#include "mdsb_v3_seq.cpp"
#include "mdsb_v4_par.cpp"

template <typename T>
mdsb<T>::mdsb(mds<T> *md, interv_seq<T> *I, T n, T a, int p, int v, bool log, std::ostream *os) {
    this->md = md;
    this->n = n;
    this->k = I->size();
    this->a = a;
    this->p = p;

    omp_set_num_threads(p);

    if (v == 1) {
        build_v1(I,log,os);
    } else {
        build_v2_v3_v4(I,v,log,os);
    }

    #ifndef NDEBUG
    verify_correctness();
    #endif
}

template <typename T>
mdsb<T>::~mdsb() {
    md = NULL;
}

template <typename T>
void mdsb<T>::build_v2_v3_v4(interv_seq<T> *I, int v, bool log, std::ostream *os) {
    size_t baseline;
    std::chrono::steady_clock::time_point time;

    if (log) {
        baseline = malloc_count_current() - sizeof(I->at(0))*I->size();
        time = std::chrono::steady_clock::now();
        malloc_count_reset_peak();
        std::cout << std::endl;
    }

    if (log) log_memory_usage(baseline,"building L_in and T_out");

    build_lin_tout(I);
    
    if (log) {
        if (os != NULL) {
            *os << " phase_1=" << time_diff(time);
        }
        time = log_runtime(time);
        log_memory_usage(baseline,"balancing");
    }

    if (p > 1) {
        if (v == 3) {
            balance_v3_par();
        } else {
            balance_v4_par();
        }
    } else {
        if (v == 2) {
            balance_v2();
        } else {
            balance_v3_seq();
        }
    }

    if (log) {
        if (os != NULL) {
            *os << " phase_2=" << time_diff(time);
        }
        time = log_runtime(time);
        log_memory_usage(baseline,"building D_pair");
    }
    
    build_dpair();

    if (log) {
        if (os != NULL) {
            *os << " phase_3=" << time_diff(time);
        }
        time = log_runtime(time);
        log_memory_usage(baseline,"building D_index");
    }

    build_dindex();

    if (log) {
        if (os != NULL) {
            *os << " phase_4=" << time_diff(time);
        }
        time = log_runtime(time);
        if (os != NULL) {
            *os << " memory_usage=" << (malloc_count_peak()-baseline)/1000000;
        }
        log_memory_usage(baseline,"move datastructure built");
    }

    if (log) std::cout << std::endl << "peak memory allocation during build: ~ " << (malloc_count_peak()-baseline)/1000000 << "MB" << std::endl << std::endl;
}

template <typename T>
void mdsb<T>::verify_correctness() {
    std::cout << "verifying correctness of the interval sequence:" << std::endl;
    bool correct = true;

    #pragma omp parallel for num_threads(p)
    for (T i=1; i<k; i++) {
        if (md->D_pair[i].first <= md->D_pair[i-1].first) {
            if (correct) {
                correct = false;
            }

            std::cout << std::endl << "wrong pair:" << std::endl;

            std::cout << "p_i = " << md->D_pair[i].first << ", p_{i-1} = " << md->D_pair[i-1].first << std::endl;
        }
    }

    #pragma omp parallel for num_threads(p)
    for (T j=0; j<k; j++) {
        T l = 0;
        T r = k-1;
        T m;
        while (l != r) {
            m = (l+r)/2;
             if (md->D_pair[m].first >= md->D_pair[j].second) {
                r = m;
            } else {
                l = m+1;
            }
        }
        T i = l;

        if (i+2*a-1 < k) {
            T q_j = md->D_pair[j].second;
            T d_j = md->D_pair[j+1].first - md->D_pair[j].first;

            T p_i = md->D_pair[i].first;
            T p_ip2am1 = md->D_pair[i+2*a-1].first;

            if (p_i < q_j + d_j && p_ip2am1 < q_j + d_j) {
                if (correct) {
                    correct = false;
                }
                std::cout << std::endl << "unbalanced output interval:" << std::endl;

                std::cout << "q_j = " << q_j << ", d_j = " << d_j << ", q_j + d_j - 1 = " << q_j + d_j - 1 << std::endl;
                std::cout << "p_i = " << p_i << ", p_{i+2a-1} = " << p_ip2am1 << std::endl;
            }
        }
    }

    std::cout << "the interval sequence has " << (correct ? "" : "not ") << "been built correctly" << std::endl << std::endl;
}