#include "app/ConfigReader.h"
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

// Konstruktor
ConfigReader::ConfigReader(const string& filename)
        : filename(filename), configEntries(nullptr), numEntries(0), capacityEntries(10) {
    // Inicjalizacja tablicy configEntries z początkową pojemnością
    configEntries = new ConfigEntry[capacityEntries];
}

// Destruktor
ConfigReader::~ConfigReader() {
    // Zwolnienie pamięci dla każdego wpisu
    for (int i = 0; i < numEntries; ++i) {
        delete[] configEntries[i].key;
        delete[] configEntries[i].value;
    }
    // Zwolnienie pamięci tablicy configEntries
    delete[] configEntries;
}

// Funkcja do usuwania białych znaków z początku i końca stringa
string ConfigReader::trim(const string& str) const {
    size_t start = 0;
    while (start < str.length() &&
           (str[start] == ' ' || str[start] == '\t' ||
            str[start] == '\n' || str[start] == '\r')) {
        start++;
    }

    size_t end = str.length();
    while (end > start &&
           (str[end - 1] == ' ' || str[end - 1] == '\t' ||
            str[end - 1] == '\n' || str[end - 1] == '\r')) {
        end--;
    }

    return (start >= end) ? "" : str.substr(start, end - start);
}

// Funkcja pomocnicza do pobierania wartości dla danego klucza
const char* ConfigReader::getValue(const string& key) const {
    for (int i = 0; i < numEntries; ++i) {
        if (key == configEntries[i].key) {
            return configEntries[i].value;
        }
    }
    return nullptr; // Klucz nie został znaleziony
}

// Funkcja parsująca plik konfiguracyjny
bool ConfigReader::parseConfig() {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        return false; // Plik nie został otwarty
    }

    string line;
    while (getline(inFile, line)) {
        // Usuwamy białe znaki
        line = trim(line);

        // Pomijamy puste linie i komentarze
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Szukamy znaku '='
        size_t delimiterPos = line.find('=');
        if (delimiterPos == string::npos) {
            continue; // Pomijamy niepoprawne linie
        }

        // Rozdzielamy klucz i wartość
        string key = trim(line.substr(0, delimiterPos));
        string value = trim(line.substr(delimiterPos + 1));

        // Alokujemy pamięć dla klucza i wartości
        char* key_cstr = new char[key.length() + 1];
        strcpy(key_cstr, key.c_str());

        char* value_cstr = new char[value.length() + 1];
        strcpy(value_cstr, value.c_str());

        // Dodajemy wpis do tablicy configEntries
        if (numEntries >= capacityEntries) {
            // Podwajamy pojemność tablicy
            int newCapacity = capacityEntries * 2;
            ConfigEntry* newEntries = new ConfigEntry[newCapacity];
            for (int i = 0; i < numEntries; ++i) {
                newEntries[i] = configEntries[i];
            }
            delete[] configEntries;
            configEntries = newEntries;
            capacityEntries = newCapacity;
        }

        configEntries[numEntries].key = key_cstr;
        configEntries[numEntries].value = value_cstr;
        numEntries++;
    }

    inFile.close();
    return true;
}

// Funkcja zwracająca algorytmy jako dynamicznie alokowaną tablicę C-stringów
char** ConfigReader::getAlgorithms(int& count) const {
    count = 0;
    const char* algorithmsStr = getValue("algorithms");
    if (algorithmsStr == nullptr) {
        algorithmsStr = getValue("algorithm");
        if (algorithmsStr == nullptr) {
            return nullptr; // Brak algorytmów w konfiguracji
        }
    }

    // Liczenie algorytmów (przecinki)
    int algoCount = 1;
    for (const char* p = algorithmsStr; *p != '\0'; ++p) {
        if (*p == ',') {
            algoCount++;
        }
    }

    // Alokacja pamięci dla algorytmów
    char** algorithms = new char*[algoCount];
    int currentAlgo = 0;

    string algoStr(algorithmsStr);
    istringstream ss(algoStr);
    string item;
    while (getline(ss, item, ',')) {
        item = trim(item);
        algorithms[currentAlgo] = new char[item.length() + 1];
        strcpy(algorithms[currentAlgo], item.c_str());
        currentAlgo++;
    }

    count = algoCount;
    return algorithms;
}

// Funkcje pobierające dane z konfiguracji
string ConfigReader::getDistanceMatrixFile() const {
    const char* value = getValue("distance_matrix_file");
    return value != nullptr ? string(value) : "";
}

bool ConfigReader::getRunSimulation() const {
    const char* value = getValue("run_simulation");
    return value != nullptr && (string(value) == "true" || string(value) == "1");
}

int ConfigReader::getNumMatrices() const {
    const char* value = getValue("num_matrices");
    return value != nullptr ? atoi(value) : 0;
}

int ConfigReader::getMatrixSize() const {
    const char* value = getValue("matrix_size");
    return value != nullptr ? atoi(value) : 0;
}

int ConfigReader::getMaxCost() const {
    const char* value = getValue("max_cost");
    return value != nullptr ? atoi(value) : 0;
}

string ConfigReader::getMatrixType() const {
    const char* value = getValue("matrix_type");
    return value != nullptr ? string(value) : "random";
}
