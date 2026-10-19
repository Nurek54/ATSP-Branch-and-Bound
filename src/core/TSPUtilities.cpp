#include "core/TSPUtilities.h"
#include <climits>

using namespace std;

// Funkcja obliczająca koszt ścieżki
int Utilities::calculate_cost(const int* path, int path_length, const int* const* distances, bool include_return) {
    int cost = 0;
    for (int i = 0; i < path_length - 1; ++i) {
        if (distances[path[i]][path[i + 1]] == -1) {
            return INT_MAX; // Nie ma połączenia między miastami
        }
        cost += distances[path[i]][path[i + 1]];
    }
    if (include_return && path[path_length - 1] != path[0]) {
        if (distances[path[path_length - 1]][path[0]] == -1) {
            return INT_MAX; // Nie ma powrotu do miasta początkowego
        }
        cost += distances[path[path_length - 1]][path[0]];
    }
    return cost;
}

// Funkcja sprawdzająca, czy miasto jest już w ścieżce
bool Utilities::isCityInPath(const int* path, int path_length, int city) {
    for (int i = 0; i < path_length; ++i) {
        if (path[i] == city) {
            return true;
        }
    }
    return false;
}

// Funkcja obliczająca granicę dolną
int Utilities::calculateLowerBound(const int* path, int path_length, const int* const* distances, const int* minEdge, int num_cities) {
    // Obliczamy koszt bieżącej ścieżki bez powrotu do miasta początkowego
    int current_cost = calculate_cost(path, path_length, distances, false);
    if (current_cost == INT_MAX) {
        return INT_MAX;
    }
    int bound = current_cost;

    int last_node = path[path_length - 1];

    // Zbiór odwiedzonych miast
    bool* visited = new bool[num_cities]();
    for (int i = 0; i < path_length; ++i) {
        visited[path[i]] = true;
    }

    int min_return = (distances[last_node][path[0]] != -1) ? distances[last_node][path[0]] : INT_MAX / 2;

    // Sumujemy minimalne koszty wejścia i wyjścia dla nieodwiedzonych miast
    for (int i = 0; i < num_cities; ++i) {
        if (!visited[i]) {
            int min_out = INT_MAX;
            int min_in = INT_MAX;

            for (int j = 0; j < num_cities; ++j) {
                if (i != j && distances[i][j] != -1 && distances[i][j] < min_out) {
                    min_out = distances[i][j];
                }
                if (i != j && distances[j][i] != -1 && distances[j][i] < min_in) {
                    min_in = distances[j][i];
                }
            }

            if (min_out == INT_MAX) min_out = minEdge[i];
            if (min_in == INT_MAX) min_in = minEdge[i];

            bound += (min_out + min_in);
        }
    }

    bound = (bound + min_return + 1) / 2;

    delete[] visited;

    return bound;
}
