// Pomiar trzech wariantow branch and bound na losowych macierzach (asymetrycznych i symetrycznych).
// Czas jest odczytywany z CSV, ktory zapisuje sam algorytm. Optimum liczy programowanie dynamiczne
// (Held-Karp), wiec od razu widac, czy wynik B&B jest optymalny.
// Wynik: CSV na stdout, kolumny algorithm,matrix_type,n,instance,ms,cost,optimum
//
// Uzycie: benchmark [nMax=12] [instancje=10] [bfsMax=10]
#include "algorithms/BranchAndBoundBFS.h"
#include "algorithms/BranchAndBoundBestFirst.h"
#include "algorithms/BranchAndBoundDFS.h"
#include "core/TSPInstance.h"

#include <algorithm>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;

// Ten sam schemat co TSPSimulation: koszty 1..100, przekatna -1
static int** randomMatrix(int n, bool symmetric, unsigned seed) {
    mt19937 rng(seed);
    uniform_int_distribution<int> cost(1, 100);
    int** m = new int*[n];
    for (int i = 0; i < n; ++i) m[i] = new int[n];
    for (int i = 0; i < n; ++i)
        for (int j = symmetric ? i : 0; j < n; ++j) {
            int v = (i == j) ? -1 : cost(rng);
            m[i][j] = v;
            if (symmetric) m[j][i] = v;
        }
    return m;
}

// Dokladne optimum, O(n^2 * 2^n)
static int heldKarp(int** m, int n) {
    const int INF = INT_MAX / 2;
    size_t full = size_t(1) << n;
    vector<int> dp(full * n, INF);
    dp[1 * n + 0] = 0;
    for (size_t s = 1; s < full; s += 2)
        for (int j = 0; j < n; ++j) {
            int v = dp[s * n + j];
            if (v >= INF || !(s >> j & 1)) continue;
            for (int k = 0; k < n; ++k)
                if (!(s >> k & 1)) {
                    size_t t = s | (size_t(1) << k);
                    dp[t * n + k] = min(dp[t * n + k], v + m[j][k]);
                }
        }
    int best = INT_MAX;
    for (int j = 1; j < n; ++j) best = min(best, dp[(full - 1) * n + j] + m[j][0]);
    return best;
}

// Ostatni wiersz: "Algorytm | Typ | s | ms | ns | KB | trasa | koszt", ms z przecinkiem dziesietnym
static double lastMilliseconds(const string& csv) {
    ifstream in(csv);
    string line, last;
    while (getline(in, line))
        if (!line.empty()) last = line;
    size_t p = 0;
    for (int i = 0; i < 3; ++i) p = last.find(" | ", p) + 3;
    string ms = last.substr(p, last.find(" | ", p) - p);
    replace(ms.begin(), ms.end(), ',', '.');
    return stod(ms);
}

int main(int argc, char** argv) {
    int nMax = argc > 1 ? stoi(argv[1]) : 12;
    int instances = argc > 2 ? stoi(argv[2]) : 10;
    int bfsMax = argc > 3 ? stoi(argv[3]) : 10;

    fs::path work = fs::temp_directory_path() / "atsp-bnb-benchmark";
    fs::create_directories(work);
    fs::current_path(work);

    cout << "algorithm,matrix_type,n,instance,ms,cost,optimum\n";
    for (const string type : {"random", "symmetric"}) {
        for (int n = 4; n <= nMax; ++n) {
            for (int k = 0; k < instances; ++k) {
                int** m = randomMatrix(n, type == "symmetric", 1000u * n + k);
                TSPInstance inst(m, n);
                int opt = heldKarp(m, n);
                for (const char* f : {"BranchAndBoundBFSResults.csv", "BranchAndBoundDFSResults.csv", "BranchAndBoundBestFirstResults.csv"})
                    fs::remove(f);

                if (n <= bfsMax) {
                    BranchAndBoundBFS bfs(inst.getDistances(), n);
                    int cost = bfs.solve(type).cost;
                    printf("bfs,%s,%d,%d,%.6f,%d,%d\n", type.c_str(), n, k, lastMilliseconds("BranchAndBoundBFSResults.csv"), cost, opt);
                }
                {
                    BranchAndBoundDFS dfs(inst.getDistances(), n);
                    int cost = dfs.solve(type).cost;
                    printf("dfs,%s,%d,%d,%.6f,%d,%d\n", type.c_str(), n, k, lastMilliseconds("BranchAndBoundDFSResults.csv"), cost, opt);
                }
                {
                    BranchAndBoundBestFirst best(inst.getDistances(), n);
                    int cost = best.solve(inst, type).cost;
                    printf("best_first,%s,%d,%d,%.6f,%d,%d\n", type.c_str(), n, k, lastMilliseconds("BranchAndBoundBestFirstResults.csv"), cost, opt);
                }
                fflush(stdout);
                for (int i = 0; i < n; ++i) delete[] m[i];
                delete[] m;
            }
            cerr << type << " n=" << n << " gotowe\n";
        }
    }
    return 0;
}
