#include "core/TSPInstance.h"
#include <fstream>
#include <stdexcept>
#include <iostream>

using namespace std;

TSPInstance::TSPInstance(const string& filename) {
    loadFromFile(filename);
}

TSPInstance::TSPInstance(int** matrix, int size) {
    cityCount = size;
    costMatrix = new int*[cityCount];
    for (int i = 0; i < cityCount; ++i) {
        costMatrix[i] = new int[cityCount];
        for (int j = 0; j < cityCount; ++j) {
            costMatrix[i][j] = matrix[i][j];
        }
    }
}

TSPInstance::~TSPInstance() {
    for (int i = 0; i < cityCount; ++i) {
        delete[] costMatrix[i];
    }
    delete[] costMatrix;
}

void TSPInstance::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Nie mozna otworzyc pliku: " + filename);
    }

    file >> cityCount;
    if (file.fail()) {
        throw runtime_error("Blad podczas odczytu liczby miast.");
    }
    //cout << "Liczba miast w pliku: " << cityCount << endl;

    costMatrix = new int*[cityCount];
    for (int i = 0; i < cityCount; ++i) {
        costMatrix[i] = new int[cityCount];
    }

    for (int i = 0; i < cityCount; ++i) {
        for (int j = 0; j < cityCount; ++j) {
            file >> costMatrix[i][j];
            if (file.fail()) {
                throw runtime_error("Blad podczas odczytu macierzy kosztow.");
            }
        }
    }

    // Opcjonalnie wyświetl macierz
    /*cout << "Macierz kosztow:" << endl;
    for (int i = 0; i < cityCount; ++i) {
        for (int j = 0; j < cityCount; ++j) {
            cout << costMatrix[i][j] << " ";
        }
        cout << endl;
    }*/
}

const int* const* TSPInstance::getDistances() const {
    return costMatrix;
}

int TSPInstance::getCityCount() const {
    return cityCount;
}
