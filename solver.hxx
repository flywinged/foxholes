#pragma once

#include "iostream"
#include "vector"

using namespace std;

#include "board.hxx"
#include "unordered_set"

#include "stpl_stl.h"


Board* solveForShape(HashableVector* shape, int16_t checksPerDay);

vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* getCheckCombinations(Board* board, int16_t checks);

vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* createCombinations(
    Board* board,
    int16_t checksLeft,
    unordered_map<HashableVector*, unordered_set<HashableVector*, VectorHash, VectorEqual>, VectorHash, VectorEqual>* removeMap,
    vector<HashableVector*>* locationsToRemove,
    unordered_set<HashableVector*, VectorHash, VectorEqual> currentCheckLocations = unordered_set<HashableVector*, VectorHash, VectorEqual>(),
    vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* combinations = new vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>(),
    int16_t depth = 0
    );