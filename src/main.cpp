#include <fstream>
#include <iostream>
#include <stdexcept>
#include "app/ConfigReader.h"
#include "core/TSPInstance.h"
#include "algorithms/BranchAndBoundBFS.h"
#include "algorithms/BranchAndBoundDFS.h"
#include "algorithms/BranchAndBoundBestFirst.h"
#include "app/TSPSimulation.h"

#ifndef ATSP_CONFIG_FILE
#define ATSP_CONFIG_FILE "config/config.txt"
#endif
#ifndef ATSP_DATA_DIR
#define ATSP_DATA_DIR "data"
#endif

using namespace std;

static bool fileExists(const string& path) {
    ifstream f(path);
    return f.good();
}

// Plik z macierza: najpierw wzgledem katalogu roboczego, potem w folderze data/ projektu
static string resolveMatrixFile(const string& name) {
    if (fileExists(name)) {
        return name;
    }
    string inData = string(ATSP_DATA_DIR) + "/" + name;
    return fileExists(inData) ? inData : name;
}

// Wypisuje koszt i trase, czas i pamiec trafiaja do pliku CSV
template <typename Result>
static void printResult(const string& name, const Result& result, const string& csvFile) {
    if (result.path == nullptr || result.path_length == 0) {
        cout << name << ": nie znaleziono trasy\n";
        return;
    }
    cout << name << ": koszt " << result.cost << "\n";
    cout << "  Trasa: ";
    for (int i = 0; i < result.path_length; ++i) {
        cout << result.path[i] << (i + 1 < result.path_length ? " -> " : "\n");
    }
    cout << "  Czas i pamiec dopisane do " << csvFile << "\n";
}

int main(int argc, char** argv) {
    // Konfiguracja: sciezka z argumentu, config.txt w katalogu roboczym albo config/config.txt projektu
    string configFile = argc > 1 ? argv[1] : (fileExists("config.txt") ? "config.txt" : ATSP_CONFIG_FILE);
    ConfigReader config(configFile);
    if (!config.parseConfig()) {
        cout << "Nie mozna wczytac pliku konfiguracyjnego: " << configFile << endl;
        return 1;
    }
    cout << "Konfiguracja: " << configFile << endl;

    // Pobierz algorytmy
    int algorithmCount = 0;
    char** algorithms = config.getAlgorithms(algorithmCount);
    if (algorithms == nullptr || algorithmCount == 0) {
        cout << "Nie podano algorytmow w konfiguracji." << endl;
        return 1;
    }

    bool runSimulation = config.getRunSimulation();
    string matrixType = config.getMatrixType(); // Pobierz typ macierzy (random/symmetric)
    int exitCode = 0;

    if (runSimulation) {
        // Uruchom symulację
        int numMatrices = config.getNumMatrices();
        int matrixSize = config.getMatrixSize();
        int maxCost = config.getMaxCost();

        TSPSimulation simulation(numMatrices, matrixSize, maxCost, algorithms, algorithmCount, matrixType);
        simulation.runSimulation();
    } else {
        // Wczytaj macierz odległości
        string matrixFile = config.getDistanceMatrixFile();
        if (matrixFile.empty()) {
            cout << "Nie podano pliku z macierza odleglosci w konfiguracji." << endl;
            exitCode = 1;
        } else {
            try {
                matrixFile = resolveMatrixFile(matrixFile);
                TSPInstance instance(matrixFile);
                const int* const* distances = instance.getDistances();
                int numCities = instance.getCityCount();
                cout << "Macierz: " << matrixFile << " (" << numCities << " miast)\n\n";

                // Iterujemy po wybranych algorytmach
                for (int i = 0; i < algorithmCount; ++i) {
                    string algorithm(algorithms[i]);

                    if (algorithm == "BFS") {
                        BranchAndBoundBFS solver(distances, numCities);
                        printResult("BFS", solver.solve(matrixType), "BranchAndBoundBFSResults.csv");
                    } else if (algorithm == "DFS") {
                        BranchAndBoundDFS solver(distances, numCities);
                        printResult("DFS", solver.solve(matrixType), "BranchAndBoundDFSResults.csv");
                    } else if (algorithm == "BEST_FIRST") {
                        BranchAndBoundBestFirst solver(distances, numCities);
                        printResult("Best-First", solver.solve(instance, matrixType), "BranchAndBoundBestFirstResults.csv");
                    } else {
                        cout << "Nieznany algorytm: " << algorithm << endl;
                    }
                }
            } catch (const exception& e) {
                cout << e.what() << endl;
                exitCode = 1;
            }
        }
    }

    // Zwolnij pamięć alokowaną dla algorytmów
    for (int i = 0; i < algorithmCount; ++i) {
        delete[] algorithms[i];
    }
    delete[] algorithms;

    return exitCode;
}
