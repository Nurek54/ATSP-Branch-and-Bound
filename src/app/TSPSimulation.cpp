#include "app/TSPSimulation.h"
#include "core/TSPInstance.h"
#include "algorithms/BranchAndBoundBFS.h"
#include "algorithms/BranchAndBoundDFS.h"
#include "algorithms/BranchAndBoundBestFirst.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

TSPSimulation::TSPSimulation(int numMatrices, int matrixSize, int maxCost, char** algorithms, int algorithmCount, const string& matrixType)
        : numMatrices(numMatrices), matrixSize(matrixSize), maxCost(maxCost), algorithms(algorithms), algorithmCount(algorithmCount), matrixType(matrixType) {
    srand(static_cast<unsigned int>(time(nullptr)));
}

TSPSimulation::~TSPSimulation() {
}

void TSPSimulation::runSimulation() {
    cout << "Symulacja: " << numMatrices << " macierzy " << matrixSize << "x" << matrixSize << " (" << matrixType << ")" << endl;
    for (int m = 0; m < numMatrices; ++m) {
        cout << "\rMacierz " << (m + 1) << " / " << numMatrices << flush;
        // Tworzymy macierz odległości
        int** matrix = new int*[matrixSize];
        for (int i = 0; i < matrixSize; ++i) {
            matrix[i] = new int[matrixSize];
        }

        // Wybieramy metodę generowania macierzy
        if (matrixType == "symmetric") {
            generateSymmetricMatrix(matrix);
        } else {
            generateRandomMatrix(matrix);
        }

        TSPInstance instance(matrix, matrixSize);
        const int* const* distances = instance.getDistances();

        for (int i = 0; i < algorithmCount; ++i) {
            string algorithm(algorithms[i]);

            if (algorithm == "BFS") {
                BranchAndBoundBFS solver(distances, matrixSize);
                auto result = solver.solve(matrixType);
            } else if (algorithm == "DFS") {
                BranchAndBoundDFS solver(distances, matrixSize);
                auto result = solver.solve(matrixType);
            } else if (algorithm == "BEST_FIRST") {
                BranchAndBoundBestFirst solver(distances, matrixSize);
                auto result = solver.solve(instance, matrixType);
            } else {
                cout << "Nieznany algorytm: " << algorithm << endl;
            }
        }

        // Zwalniamy pamięć
        for (int i = 0; i < matrixSize; ++i) {
            delete[] matrix[i];
        }
        delete[] matrix;
    }
    cout << "\nWyniki dopisane do plikow BranchAndBound*Results.csv" << endl;
}

void TSPSimulation::generateRandomMatrix(int** matrix) {
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = 0; j < matrixSize; ++j) {
            if (i == j) {
                matrix[i][j] = -1; // Brak połączenia do samego siebie
            } else {
                matrix[i][j] = rand() % maxCost + 1; // Losowa wartość od 1 do maxCost
            }
        }
    }
}

void TSPSimulation::generateSymmetricMatrix(int** matrix) {
    for (int i = 0; i < matrixSize; ++i) {
        for (int j = i; j < matrixSize; ++j) {
            if (i == j) {
                matrix[i][j] = -1; // Brak połączenia do samego siebie
            } else {
                int cost = rand() % maxCost + 1; // Losowa wartość od 1 do maxCost
                matrix[i][j] = cost;
                matrix[j][i] = cost; // Symetryczność
            }
        }
    }
}
