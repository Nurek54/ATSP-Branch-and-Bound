#include "algorithms/BranchAndBoundBestFirst.h"
#include "io/SaveToCSV.h"
#include "core/TSPUtilities.h"
#include "core/MemoryUsage.h"
#include <climits>
#include <chrono>
#include <iostream>

using namespace std;

// Konstruktor klasy Node
BranchAndBoundBestFirst::Node::Node(int num_cities) {
    path = new int[num_cities + 1]; // Prealokacja ścieżki
    path_length = 0;
    current_city = -1;
    current_cost = 0;
    estimated_total_cost = 0;
}

// Destruktor klasy Node
BranchAndBoundBestFirst::Node::~Node() {
    delete[] path;
}

// Konstruktor klasy BranchAndBoundBestFirst
BranchAndBoundBestFirst::BranchAndBoundBestFirst(const int* const* input_matrix, int num_cities_input)
        : num_cities(num_cities_input) {
    // Alokujemy dynamicznie macierz odległości
    matrix = new int*[num_cities];
    for (int i = 0; i < num_cities; ++i) {
        matrix[i] = new int[num_cities];
        for (int j = 0; j < num_cities; ++j) {
            matrix[i][j] = input_matrix[i][j];
        }
    }
    // Inicjalizujemy minEdge
    minEdge = new int[num_cities];
    preprocessMinEdges();
}

// Destruktor klasy BranchAndBoundBestFirst
BranchAndBoundBestFirst::~BranchAndBoundBestFirst() {
    for (int i = 0; i < num_cities; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
    delete[] minEdge;
}

// Funkcja preprocessMinEdges
void BranchAndBoundBestFirst::preprocessMinEdges() {
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

// Funkcja insert do zarządzania kopcem
void BranchAndBoundBestFirst::insert(Node**& heap, int& heapSize, int& heapCapacity, Node* node) {
    if (heapSize >= heapCapacity) {
        int new_capacity = heapCapacity * 2;
        Node** new_heap = new Node*[new_capacity];
        for (int i = 0; i < heapSize; ++i) {
            new_heap[i] = heap[i];
        }
        delete[] heap;
        heap = new_heap;
        heapCapacity = new_capacity;
    }

    heap[heapSize] = node;
    int idx = heapSize;
    ++heapSize;

    // Przesuwanie elementu w górę kopca
    while (idx > 0 && (*heap[idx]).estimated_total_cost < (*heap[(idx - 1) / 2]).estimated_total_cost) {
        Node* temp = heap[idx];
        heap[idx] = heap[(idx - 1) / 2];
        heap[(idx - 1) / 2] = temp;
        idx = (idx - 1) / 2;
    }
}

// Funkcja remove do zarządzania kopcem
BranchAndBoundBestFirst::Node* BranchAndBoundBestFirst::remove(Node**& heap, int& heapSize) {
    if (heapSize == 0) {
        return nullptr;
    }

    Node* node = heap[0];
    heap[0] = heap[heapSize - 1];
    --heapSize;

    int idx = 0;
    while (true) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;

        if (left < heapSize && (*heap[left]).estimated_total_cost < (*heap[smallest]).estimated_total_cost) {
            smallest = left;
        }
        if (right < heapSize && (*heap[right]).estimated_total_cost < (*heap[smallest]).estimated_total_cost) {
            smallest = right;
        }
        if (smallest == idx) break;

        Node* temp = heap[idx];
        heap[idx] = heap[smallest];
        heap[smallest] = temp;

        idx = smallest;
    }

    return node;
}

// Główna funkcja solve
BranchAndBoundBestFirst::Result BranchAndBoundBestFirst::solve(const TSPInstance& instance, const string& matrixType) {
    int best_cost = INT_MAX;
    int* best_path = nullptr;
    int best_path_length = 0;

    // Inicjalizacja kopca
    int heapCapacity = 1000;
    int heapSize = 0;
    Node** heap = new Node*[heapCapacity];

    // Tworzenie węzła początkowego
    Node* root = new Node(num_cities);
    (*root).path_length = 1;
    (*root).path[0] = 0; // Startujemy z miasta 0
    (*root).current_city = 0;
    (*root).current_cost = 0;
    (*root).estimated_total_cost = Utilities::calculateLowerBound((*root).path, (*root).path_length, matrix, minEdge, num_cities);

    insert(heap, heapSize, heapCapacity, root);

    auto start_time = chrono::high_resolution_clock::now();

    while (heapSize > 0) {
        Node* current_node = remove(heap, heapSize);

        // Przycinanie gałęzi
        if ((*current_node).estimated_total_cost >= best_cost) {
            delete current_node;
            continue;
        }

        // Sprawdzamy, czy odwiedziliśmy wszystkie miasta
        if ((*current_node).path_length == num_cities) {
            // Dodajemy powrót do miasta startowego
            int* complete_path = new int[(*current_node).path_length + 1];
            for (int i = 0; i < (*current_node).path_length; ++i) {
                complete_path[i] = (*current_node).path[i];
            }
            complete_path[(*current_node).path_length] = (*current_node).path[0];

            // Obliczamy całkowity koszt ścieżki
            int total_cost = Utilities::calculate_cost(complete_path, (*current_node).path_length + 1, matrix, true);

            if (total_cost < best_cost) {
                best_cost = total_cost;
                delete[] best_path;
                best_path_length = (*current_node).path_length + 1;
                best_path = new int[best_path_length];
                for (int i = 0; i < best_path_length; ++i) {
                    best_path[i] = complete_path[i];
                }
            }

            delete[] complete_path;
            delete current_node;
            continue;
        }

        // Generowanie dzieci węzła
        for (int i = 0; i < num_cities; ++i) {
            if (!Utilities::isCityInPath((*current_node).path, (*current_node).path_length, i) && matrix[(*current_node).current_city][i] != -1) {
                Node* child = new Node(num_cities);
                // Kopiowanie ścieżki
                for (int j = 0; j < (*current_node).path_length; ++j) {
                    (*child).path[j] = (*current_node).path[j];
                }
                (*child).path[(*current_node).path_length] = i;
                (*child).path_length = (*current_node).path_length + 1;
                (*child).current_city = i;
                (*child).current_cost = (*current_node).current_cost + matrix[(*current_node).current_city][i];

                (*child).estimated_total_cost = Utilities::calculateLowerBound((*child).path, (*child).path_length, matrix, minEdge, num_cities);

                // Przycinanie gałęzi jeśli granica dolna jest mniejsza niż aktualny min_cost
                if ((*child).estimated_total_cost < best_cost) {
                    insert(heap, heapSize, heapCapacity, child);
                } else {
                    delete child;
                }
            }
        }

        delete current_node;
    }

    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> seconds = end_time - start_time;
    chrono::duration<double, milli> milliseconds = end_time - start_time;
    chrono::duration<double, nano> nanoseconds = end_time - start_time;

    size_t memoryUsageKB = getCurrentMemoryUsageKB();

    SaveToCSV save("BranchAndBoundBestFirstResults.csv");
    save.saveResults("BranchAndBoundBestFirst", matrixType, seconds, milliseconds, nanoseconds, memoryUsageKB, best_path, best_path_length, best_cost);

    // Czyszczenie pamięci
    for (int i = 0; i < heapSize; ++i) {
        delete heap[i];
    }
    delete[] heap;

    return Result(best_path, best_path_length, best_cost);
}
