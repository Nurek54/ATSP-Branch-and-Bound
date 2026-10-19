#ifndef TSPUTILITIES_H
#define TSPUTILITIES_H

#include <string>

using namespace std;

class Utilities {
public:
    static int calculate_cost(const int* path, int path_length, const int* const* distances, bool include_return = true);
    static bool isCityInPath(const int* path, int path_length, int city);
    static int calculateLowerBound(const int* path, int path_length, const int* const* distances, const int* minEdge, int num_cities);
};

#endif // TSPUTILITIES_H
