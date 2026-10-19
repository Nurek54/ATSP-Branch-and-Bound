#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>

using namespace std;

class ConfigReader {
public:
    ConfigReader(const string& filename);
    ~ConfigReader();
    bool parseConfig();
    char** getAlgorithms(int& count) const;
    string getDistanceMatrixFile() const;
    bool getRunSimulation() const;
    int getNumMatrices() const;
    int getMatrixSize() const;
    int getMaxCost() const;
    string getMatrixType() const;

private:
    string filename;

    struct ConfigEntry {
        char* key;
        char* value;
    };

    ConfigEntry* configEntries;
    int numEntries;
    int capacityEntries;

    string trim(const string& str) const;
    const char* getValue(const string& key) const;

    ConfigReader(const ConfigReader&) = delete;
    ConfigReader& operator=(const ConfigReader&) = delete;
};

#endif // CONFIGREADER_H
