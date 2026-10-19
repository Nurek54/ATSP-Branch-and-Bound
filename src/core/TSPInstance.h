#ifndef TSPINSTANCE_H
#define TSPINSTANCE_H

#include <string>

using namespace std;

class TSPInstance {
public:
    // Konstruktor przyjmujący nazwę pliku
    TSPInstance(const string& filename);

    // Konstruktor przyjmujący macierz
    TSPInstance(int** matrix, int size);

    // Destruktor
    ~TSPInstance();

    const int* const* getDistances() const;  // Zwraca macierz kosztów
    int getCityCount() const;                // Zwraca liczbę miast

private:
    int** costMatrix;  // Macierz kosztów
    int cityCount;     // Liczba miast
    void loadFromFile(const string& filename);  // Wczytanie danych z pliku
};

#endif // TSPINSTANCE_H
