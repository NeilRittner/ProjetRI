#pragma once
#define NBFILES 39
#define D 0.85
#define CONVERGENCE 0.0001
#include <string>
#include <vector>
#include <map>

void IndexData();
std::vector<double> getPageRanks();
std::vector<std::string> getResumes();
std::map<std::string, std::vector<int>> getInvertedIndex();
std::vector<std::string> getUrls();
