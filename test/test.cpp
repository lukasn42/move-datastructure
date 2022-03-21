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
#include <limits>
#include <sstream>
#include <filesystem>

#include <ips4o.hpp>

extern "C" {
    #include <malloc_count.h>
}

extern "C" {
    #include <libsais.h>
    #include <libsais64.h>
}

#include <mds.hpp>
#include <mds.cpp>

std::chrono::steady_clock::time_point log_runtime(std::chrono::steady_clock::time_point time, std::string message) {
    std::cout << message << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-time).count() << " ms" << std::endl;
    return std::chrono::steady_clock::now();
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 6)
{
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << a_value;
    return out.str();
}

void log_invalid_input() {
    std::cout << "invalid input, usage: a p v t (m)" << std::endl;
    std::cout << "    a: balancing parameter, restricts size increase to the factor (1+1/(a-1))" << std::endl;
    std::cout << "    p: number of threads to use (1 for v=1/2, 1<=p<=n for v=3, 2<=p<=n for v=4)" << std::endl;
    std::cout << "    v: build method version (1/2/3/4)" << std::endl;
    std::cout << "    t: text file" << std::endl;
    std::cout << "    m: (optional) writes runtime and memory usage measurements to a file" << std::endl;
}

template<typename INT_T>
void test(std::string &T, INT_T n, int a, int p, int v, std::chrono::steady_clock::time_point time, std::string text_file_name, std::ofstream *measurement_file = NULL) {
    std::vector<INT_T>SA(n);
    if (std::is_same<INT_T,uint32_t>::value) {
        if (p > 1) {
            libsais_omp((uint8_t*)&T[0],(int32_t*)&SA[0],(int32_t)n,0,NULL,p);
        } else {
            libsais((uint8_t*)&T[0],(int32_t*)&SA[0],(int32_t)n,0,NULL);
        }
    } else {
        if (p > 1) {
            libsais64_omp((uint8_t*)&T[0],(int64_t*)&SA[0],(int64_t)n,0,NULL,p);
        } else {
            libsais64((uint8_t*)&T[0],(int64_t*)&SA[0],(int64_t)n,0,NULL);
        }
    }
    time = log_runtime(time,"SA calculated in");

    std::string bwt;
    bwt.resize(n);
    #pragma omp parallel for num_threads(p)
    for (INT_T i=0; i<n; i++) {
        bwt[i] = SA[i] == 0 ? T[n-1] : T[SA[i]-1];
    }
    T.clear();
    time = log_runtime(time,"bwt calculated in");

    std::vector<INT_T> C(256,0);
    for (uint8_t c : bwt) {
        C[c]++;
    }
    for (int i=255; i>0; i--) {
        C[i] = C[i-1];
    }
    C[0] = 0;
    for (int i=1; i<256; i++) {
        C[i] += C[i-1];
    }
    time = log_runtime(time,"C calculated in");

    mds<INT_T> M_LF,M_phi;
    INT_T r;

    {
        std::vector<std::pair<INT_T,INT_T>> *I_LF = new std::vector<std::pair<INT_T,INT_T>>();
        I_LF->reserve(n/16);
        {
            std::vector<INT_T> C_(256,0);
            INT_T l = 0;
            uint8_t c = bwt[0];
            I_LF->emplace_back(std::make_pair(0,C[c]));
            for (INT_T i=1; i<n; i++) {
                if (bwt[i] != c) {
                    C_[c] += i-l;
                    l = i;
                    c = bwt[i];
                    I_LF->emplace_back(std::make_pair(i,C[c]+C_[c]));
                }
            }
        }
        I_LF->shrink_to_fit();
        C.clear();
        r = I_LF->size();
        time = log_runtime(time,"I_LF calculated in");


        if (measurement_file != NULL) {
            *measurement_file << "RESULT text=" << text_file_name << " type=M_LF" << " a=" << a << " p=" << p << " v=" << v;
        }
        M_LF = mds<INT_T>(I_LF,n,a,p,v,true,measurement_file);

        std::string growth_factor = to_string_with_precision(std::ceil(M_LF.intervals()/(double) r * 1000.0)/1000.0,3);
        if (measurement_file != NULL) {
            *measurement_file << " growth_factor=" << growth_factor << std::endl;
        }
        time = log_runtime(time,"M_LF (" + growth_factor + ") calculated in");
        r = M_LF.intervals();
    }

    {
        std::vector<std::pair<INT_T,INT_T>> *I_phi = new std::vector<std::pair<INT_T,INT_T>>(r);
        I_phi->at(0) = std::make_pair(SA[0],SA[n-1]);
        #pragma omp parallel for num_threads(p)
        for (INT_T i=1; i<r; i++) {
            I_phi->at(i) = std::make_pair(SA[M_LF.pair(i).first],SA[M_LF.pair(i).first-1]);
        }
        auto comp = [](auto p1, auto p2){return p1.first < p2.first;};
        if (p > 1) {
            ips4o::parallel::sort(I_phi->begin(),I_phi->end(),comp);
        } else {
            ips4o::sort(I_phi->begin(),I_phi->end(),comp);
        }
        time = log_runtime(time,"I_phi' calculated in");

        if (measurement_file != NULL) {
            *measurement_file << "RESULT text=" << text_file_name << " type=M_phi" << " a=" << a << " p=" << p << " v=" << v;
        }
        M_phi = mds<INT_T>(I_phi,n,a,p,v,true,measurement_file);
        
        std::string growth_factor = to_string_with_precision(std::ceil(M_phi.intervals()/(double) r * 1000.0)/1000.0,3);
        if (measurement_file != NULL) {
            *measurement_file << " growth_factor=" << growth_factor << std::endl;
        }
        time = log_runtime(time,"M_phi (" + growth_factor + ") calculated in");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 5 || 6 < argc) {
        log_invalid_input();
        return -1;
    }

    int a = atoi(argv[1]);
    int p = atoi(argv[2]);
    int v = atoi(argv[3]);
    std::string text_file_name = argv[4];
    bool measure = argc == 6;
    std::ofstream measurement_file;

    if (measure) {
        measurement_file.open(argv[5],std::filesystem::exists(argv[5]) ? std::ios::app : std::ios::out);
    }

    if (!(
        (!measure || measurement_file.good()) &&
        2 <= a &&
        1 <= p && p <= omp_get_max_threads() &&
        ((v == 1 && p == 1) || (v == 2 && p == 1) || v == 3 || (v == 4 && 2 <= p))
    )) {
        log_invalid_input();
        return -1;
    }

    omp_set_num_threads(p);
    std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();

    std::ifstream file(text_file_name);
    text_file_name = text_file_name.substr(text_file_name.find_last_of("/\\") + 1);
    if (!file.good()) {
        std::cout << "invalid input: could not read textfile" << std::endl;
        return -1;
    }
    file.seekg(0,std::ios::end);
    uint64_t n = file.tellg()+(std::streamsize)+1;
    if (n < (uint64_t) p) {
        std::cout << "invalid input: n < p" << std::endl;
        return -1;
    }
    file.seekg(0,std::ios::beg);
    std::string T;
    T.resize(n);
    file.read((char*)&T[0],n-1);
    file.close();
    T[n-1] = 1;
    time = log_runtime(time,"file read");

    if (n <= UINT_MAX) {
        test<uint32_t>(T,n,a,p,v,time,text_file_name,(measure ? &measurement_file : NULL));
    } else {
        test<uint64_t>(T,n,a,p,v,time,text_file_name,(measure ? &measurement_file : NULL));
    }

    if (measure) {
        measurement_file.close();
    }

    return 0;
}