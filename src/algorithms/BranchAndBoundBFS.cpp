#include "algorithms/BranchAndBoundBFS.h"
#include "io/SaveToCSV.h"
#include "core/TSPUtilities.h"
#include "core/MemoryUsage.h"
#include <chrono>
#include <climits>
#include <iostream>

using namespace std;

// Implementacja konstruktora klasy BranchAndBoundBFS
BranchAndBoundBFS::BranchAndBoundBFS(const int* const* input_matrix, int num_cities_input)
        : num_cities(num_cities_input) {
    // Alokacja macierzy kosztów
    matrix = new int*[num_cities];
    for (int i = 0; i < num_cities; ++i) {
        matrix[i] = new int[num_cities];
        for (int j = 0; j < num_cities; ++j) {
            matrix[i][j] = input_matrix[i][j];
        }
    }

    // Inicjalizacja tablicy minimalnych krawędzi
    minEdge = new int[num_cities];
    preprocessMinEdges();
}

// Implementacja destruktora klasy BranchAndBoundBFS
BranchAndBoundBFS::~BranchAndBoundBFS() {
    for (int i = 0; i < num_cities; ++i) {
        delete[] matrix[i];
    }
    delete[] matrix;
    delete[] minEdge;
}

// Funkcja preprocessMinEdges - oblicza minimalne krawędzie dla każdego miasta
void BranchAndBoundBFS::preprocessMinEdges() {
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

// Konstruktor kolejki
BranchAndBoundBFS::Queue::Queue(int initial_capacity)
        : front(0), rear(initial_capacity - 1), size(0), capacity(initial_capacity) {
    nodes = new QueueNode[capacity];
}

// Destruktor kolejki
BranchAndBoundBFS::Queue::~Queue() {
    for (int i = 0; i < size; ++i) {
        int index = (front + i) % capacity;
        if (nodes[index].path != nullptr) {
            delete[] nodes[index].path;
        }
    }
    delete[] nodes;
}

// Funkcja enqueue - dodaje węzeł do kolejki
void BranchAndBoundBFS::Queue::enqueue(const QueueNode& node) {
    if (size >= capacity) {
        // Zwiększenie pojemności kolejki (podwajanie)
        int new_capacity = capacity * 2;
        QueueNode* new_nodes = new QueueNode[new_capacity];
        for (int i = 0; i < size; ++i) {
            int old_index = (front + i) % capacity;
            new_nodes[i] = nodes[old_index];
        }
        delete[] nodes;
        nodes = new_nodes;
        front = 0;
        rear = size - 1;
        capacity = new_capacity;
    }
    rear = (rear + 1) % capacity;
    nodes[rear] = node;
    size++;
}

// Funkcja dequeue - usuwa i zwraca węzeł z frontu kolejki
BranchAndBoundBFS::Queue::QueueNode BranchAndBoundBFS::Queue::dequeue() {
    QueueNode node = {nullptr, 0, 0, 0};
    if (empty()) {
        // Można rzucić wyjątek lub obsłużyć pustą kolejkę inaczej
        return node;
    }
    node = nodes[front];
    front = (front + 1) % capacity;
    size--;
    return node;
}

// Funkcja sprawdzająca, czy kolejka jest pusta
bool BranchAndBoundBFS::Queue::empty() const {
    return size == 0;
}

// Główna funkcja solve
BranchAndBoundBFS::Result BranchAndBoundBFS::solve(const string& matrixType) {
    // Inicjalizacja własnej kolejki
    Queue q;

    // Tworzenie węzła początkowego
    int* root_path = new int[1];
    root_path[0] = 0; // Startujemy z miasta 0

    // Obliczanie dolnej granicy dla węzła początkowego
    int root_bound = Utilities::calculateLowerBound(root_path, 1, matrix, minEdge, num_cities);

    // Dodanie węzła początkowego do kolejki
    Queue::QueueNode root_node = {root_path, 1, 0, root_bound};
    q.enqueue(root_node);

    // Inicjalizacja zmiennych do przechowywania najlepszej ścieżki
    int min_cost = INT_MAX;
    int* best_path = nullptr;
    int best_path_length = 0;

    auto start_time = chrono::high_resolution_clock::now();

    // Główna pętla algorytmu BFS
    while (!q.empty()) {
        // Pobieramy węzeł z kolejki
        Queue::QueueNode current = q.dequeue();

        // Obsługa pustego węzła (jeśli kolejka była pusta)
        if (current.path == nullptr && current.path_length == 0 && current.cost == 0 && current.bound == 0) {
            continue;
        }

        int* current_path = current.path;
        int current_path_length = current.path_length;
        int current_cost = current.cost;
        int current_bound = current.bound;

        // Przycinanie gałęzi
        if (current_bound >= min_cost) {
            delete[] current_path;
            continue;
        }

        // Sprawdzamy, czy odwiedziliśmy wszystkie miasta
        if (current_path_length == num_cities) {
            // Dodajemy powrót do miasta startowego, aby zamknąć cykl
            int* complete_path = new int[current_path_length + 1];
            for (int i = 0; i < current_path_length; ++i) {
                complete_path[i] = current_path[i];
            }
            complete_path[current_path_length] = current_path[0];

            // Obliczamy całkowity koszt ścieżki
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
            continue;
        }

        // Generowanie dzieci węzła
        for (int i = 0; i < num_cities; ++i) {
            // Sprawdzamy, czy miasto i jest już w ścieżce i czy istnieje połączenie
            if (!Utilities::isCityInPath(current_path, current_path_length, i) &&
                matrix[current_path[current_path_length - 1]][i] != -1) {

                // Tworzymy nową ścieżkę dla dziecka
                int* child_path = new int[current_path_length + 1];
                for (int j = 0; j < current_path_length; ++j) {
                    child_path[j] = current_path[j];
                }
                child_path[current_path_length] = i;

                // Obliczamy nowy koszt ścieżki
                int temp_cost = current_cost + matrix[current_path[current_path_length - 1]][i];

                // Obliczamy nową dolną granicę dla dziecka
                int child_bound = Utilities::calculateLowerBound(child_path, current_path_length + 1, matrix, minEdge, num_cities);

                // Przycinamy gałęzie, jeśli dolna granica jest mniejsza niż obecny minimalny koszt
                if (child_bound < min_cost) {
                    Queue::QueueNode child_node = {child_path, current_path_length + 1, temp_cost, child_bound};
                    q.enqueue(child_node);
                } else {
                    delete[] child_path;
                }
            }
        }

        // Usuwamy bieżącą ścieżkę, aby uniknąć wycieków pamięci
        delete[] current_path;
    }

    // Dodajemy koszt powrotu do miasta początkowego dla najlepszej ścieżki (jeśli istnieje)
    if (best_path != nullptr) {
        // Sprawdzamy, czy ostatnie miasto nie jest już miastem początkowym
        if (best_path_length > 0 && best_path[best_path_length - 1] != best_path[0]) {
            int* complete_path = new int[best_path_length + 1];
            for (int i = 0; i < best_path_length; ++i) {
                complete_path[i] = best_path[i];
            }
            complete_path[best_path_length] = best_path[0];
            delete[] best_path;
            best_path = complete_path;
            best_path_length += 1;
        }

        // Aktualizacja minimalnego kosztu z powrotem do miasta startowego
        int final_cost = Utilities::calculate_cost(best_path, best_path_length, matrix, true);
        min_cost = final_cost;
    }

    // Zapisujemy czas zakończenia algorytmu
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> seconds = end_time - start_time;
    chrono::duration<double, milli> milliseconds = end_time - start_time;
    chrono::duration<double, nano> nanoseconds = end_time - start_time;

    size_t memoryUsageKB = getCurrentMemoryUsageKB();

    SaveToCSV save("BranchAndBoundBFSResults.csv");
    save.saveResults("BranchAndBoundBFS", matrixType, seconds, milliseconds, nanoseconds, memoryUsageKB, best_path, best_path_length, min_cost);

    Result result;
    result.path = best_path;
    result.path_length = best_path_length;
    result.cost = min_cost;

    return result;
}
