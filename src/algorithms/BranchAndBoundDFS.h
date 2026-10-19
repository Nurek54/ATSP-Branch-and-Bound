#ifndef BRANCHANDBOUNDDFS_H
#define BRANCHANDBOUNDDFS_H

#include <string>

using namespace std;

class BranchAndBoundDFS {
public:
    BranchAndBoundDFS(const int* const* input_matrix, int num_cities_input);
    ~BranchAndBoundDFS();

    struct Result {
        int* path;
        int path_length;
        int cost;

        Result() : path(nullptr), path_length(0), cost(0) {}
        Result(int* p, int len, int c) : path(p), path_length(len), cost(c) {}
        ~Result() { delete[] path; }
    };

    Result solve(const string& matrixType);

private:
    int num_cities;
    int** matrix;
    int* minEdge; // Minimalne krawędzie dla każdego miasta

    void preprocessMinEdges();

    void dfs(int* current_path,
             int current_path_length,
             int current_cost,
             int current_bound,
             int& min_cost,
             int*& best_path,
             int& best_path_length);
};

#endif // BRANCHANDBOUNDDFS_H
