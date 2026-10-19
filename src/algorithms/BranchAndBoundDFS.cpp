#include "algorithms/BranchAndBoundDFS.h"
#include "io/SaveToCSV.h"
#include "core/TSPUtilities.h"
#include "core/MemoryUsage.h"
#include <chrono>
#include <climits>
#include <iostream>

using namespace std;

BranchAndBoundDFS::BranchAndBoundDFS(const int* const* input_matrix, int num_cities_input)
        : num_cities(num_cities_input) {
    matrix = new int*[num_cities];
    for (int i = 0; i < num_cities; ++i) {
        matrix[i] = new int[num_cities];
        for (int j = 0; j < num_cities; ++j) {
            matrix[i][j] = input_matrix[i][j];
        }
    }
    minEdge = new int[num_cities];
    preprocessMinEdges();
}

BranchAndBoundDFS::~BranchAndBoundDFS() {
    for (int i = 0; i < num_cities; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
    delete[] minEdge;
}

void BranchAndBoundDFS::preprocessMinEdges() {
    for (int i = 0; i < num_cities; ++i) {
        int min_value = INT_MAX;
        for (int j = 0; j < num_cities; ++j) {
            if (i != j && matrix[i][j] != -1 && matrix[i][j] < min_value) {
                min_value = matrix[i][j];
            }
        }
        minEdge[i] = (min_value != INT_MAX) ? min_value : 0;
    }
}

void BranchAndBoundDFS::dfs(int* current_path,
                            int current_path_length,
                            int current_cost,
                            int current_bound,
                            int& min_cost,
                            int*& best_path,
                            int& best_path_length) {
    if (current_bound >= min_cost) {
        delete[] current_path;
        return;
    }

    if (current_path_length == num_cities) {
        int* complete_path = new int[current_path_length + 1];
        for (int i = 0; i < current_path_length; ++i) {
            complete_path[i] = current_path[i];
        }
        complete_path[current_path_length] = current_path[0];

        int total_cost = Utilities::calculate_cost(complete_path, current_path_length + 1, matrix, true);
        if (total_cost < min_cost) {
            min_cost = total_cost;
            delete[] best_path;
            best_path_length = current_path_length + 1;
            best_path = complete_path;
        } else {
            delete[] complete_path;
        }
        delete[] current_path;
        return;
    }

    for (int i = 0; i < num_cities; ++i) {
        if (!Utilities::isCityInPath(current_path, current_path_length, i)) {
            int* child_path = new int[current_path_length + 1];
            for (int j = 0; j < current_path_length; ++j) {
                child_path[j] = current_path[j];
            }
            child_path[current_path_length] = i;

            int temp_cost = Utilities::calculate_cost(child_path, current_path_length + 1, matrix, false);
            int child_bound = Utilities::calculateLowerBound(child_path, current_path_length + 1, matrix, minEdge, num_cities);

            if (child_bound < min_cost) {
                dfs(child_path, current_path_length + 1, temp_cost, child_bound, min_cost, best_path, best_path_length);
            } else {
                delete[] child_path;
            }
        }
    }
    delete[] current_path;
}

BranchAndBoundDFS::Result BranchAndBoundDFS::solve(const string& matrixType) {
    int* root_path = new int[1];
    root_path[0] = 0;

    int root_bound = Utilities::calculateLowerBound(root_path, 1, matrix, minEdge, num_cities);

    int min_cost = INT_MAX;
    int* best_path = nullptr;
    int best_path_length = 0;

    auto start_time = chrono::high_resolution_clock::now();

    dfs(root_path, 1, 0, root_bound, min_cost, best_path, best_path_length);

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> seconds = end_time - start_time;
    chrono::duration<double, milli> milliseconds = end_time - start_time;
    chrono::duration<double, nano> nanoseconds = end_time - start_time;

    size_t memoryUsageKB = getCurrentMemoryUsageKB();

    SaveToCSV save("BranchAndBoundDFSResults.csv");
    save.saveResults("BranchAndBoundDFS", matrixType, seconds, milliseconds, nanoseconds, memoryUsageKB, best_path, best_path_length, min_cost);

    Result result;
    result.path = best_path;
    result.path_length = best_path_length;
    result.cost = min_cost;

    return result;
}
