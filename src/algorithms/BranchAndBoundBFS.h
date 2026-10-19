#ifndef BRANCHANDBOUNDBFS_H
#define BRANCHANDBOUNDBFS_H

#include <string>

using namespace std;

class TSPInstance;
class SaveToCSV;

class BranchAndBoundBFS {
public:
    // Struktura wyników algorytmu
    struct Result {
        int* path;
        int path_length;
        int cost;

        Result() : path(nullptr), path_length(0), cost(0) {}
        Result(int* p, int len, int c) : path(p), path_length(len), cost(c) {}
        ~Result() { delete[] path; }
    };

    // Konstruktor i destruktor
    BranchAndBoundBFS(const int* const* input_matrix, int num_cities_input);
    ~BranchAndBoundBFS();

    // Główna funkcja rozwiązująca problem
    Result solve(const string& matrixType);

private:
    int num_cities;    // Liczba miast
    int** matrix;      // Macierz kosztów
    int* minEdge;      // Minimalne krawędzie dla każdego miasta

    // Funkcja do wstępnej obróbki minimalnych krawędzi
    void preprocessMinEdges();

    BranchAndBoundBFS(const BranchAndBoundBFS&) = delete;
    BranchAndBoundBFS& operator=(const BranchAndBoundBFS&) = delete;

    // Własna implementacja kolejki
    struct Queue {
        struct QueueNode {
            int* path;
            int path_length;
            int cost;
            int bound;
        };

        // Deklaracje metod kolejki
        Queue(int initial_capacity = 1000);
        ~Queue();

        void enqueue(const QueueNode& node);
        QueueNode dequeue();
        bool empty() const;

    private:
        QueueNode* nodes;   // Dynamicznie alokowana tablica węzłów
        int front;          // Indeks frontu
        int rear;           // Indeks tyłu
        int size;           // Aktualny rozmiar kolejki
        int capacity;       // Maksymalna pojemność
    };
};

#endif // BRANCHANDBOUNDBFS_H
