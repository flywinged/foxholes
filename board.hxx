#pragma once

#include "iostream"
#include "vector"
#include "unordered_map"
#include "math.h"
#include "algorithm"
#include "unordered_set"

using namespace std;

// Coord struct for managing hash tables
struct HashableVector
{

    int16_t size;
    vector<int16_t> values;

    HashableVector() {}

    HashableVector(int16_t _size)
    {
        size = _size;
        values = vector<int16_t>(size);
    }

    HashableVector(int16_t _size, vector<int16_t> _values)
    {
        size = _size;
        values = _values;
    }

    HashableVector(const HashableVector &hashableVector)
    {
        size = hashableVector.size;
        values = hashableVector.values;
    }

    HashableVector(const HashableVector* hashableVector)
    {
        size = hashableVector->size;
        values = hashableVector->values;
    }

    bool operator==(const HashableVector &other) const
    {
        if (other.size != size)
            return false;
        
        for (int i = 0; i < size; i++)
            if (other.values[i] != values[i])
                return false;
        
        return true;
    }

    int16_t operator^(const HashableVector &other) const
    {
        int16_t distanceSquared = 0;

        for (int d = 0; d < size; d++)
            distanceSquared += (int16_t)pow(values[d] - other.values[d], 2);
        
        return distanceSquared;
    }

    void operator+=(HashableVector* other)
    {
        if (other->size == size)
            for (int i = 0; i < size; i++)
                values.at(i) = values.at(i) + other->values.at(i);

    }

    string show() const
    {
        string returnString = "";

        for (int i = 0; i < size; i++)
        {
            returnString += to_string(values[i]);
            if (i != size - 1)
                returnString += ",";
        }

        return returnString;
    }

};

// HASHES //
struct VectorHash
{
    std::size_t operator()(const HashableVector* hashableVector) const
    {
        // Convert the values into a string
        string hashString = "";
        
        for (int d = 0; d < hashableVector->size; d++)
        {
            hashString += to_string(hashableVector->values[d]);
            if (d != hashableVector->size - 1)
                hashString += "-";
        }

        return std::hash<string>()(hashString);
    }
};

struct VectorEqual
{
    bool operator()(const HashableVector &first, const HashableVector &second) const
    {
        if (second.size != first.size)
            return false;
        
        for (int i = 0; i < first.size; i++)
            if (second.values[i] != first.values[i])
                return false;
        
        return true;
    }
};

class Board
{

    public:

        // Default Constructor
        Board(HashableVector* _shape);

        // Copy constructor
        Board(Board* board);

        // Default Parameters
        HashableVector* shape;
        int16_t size;

        // This is the state of the actual board.
        unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual> state;

        // Function to enumerate through all parts of the state
        void enumerate();

        // History of board check
        vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* checkHistory = new vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>;
        void copyHistory(Board* otherBoard);

        // Propogate the board
        Board* propogate(bool increment = false);

        // Function to show the board
        string show();

        // Get hash for a board
        size_t getBoardHash();

        // Set the value of the board at a specific locations
        void setBoardValue(HashableVector* location, int16_t value);

        // Necessary to determine if the board was solved
        bool solved = false;

        // Necessary for hashing
        bool gotHash = false;
        string hashString = "";
        size_t boardHash;
        bool operator==(const Board &other) const;

        void showCheckPath();

        int16_t sum();

        ~Board();

};

Board* createInitialBoard(HashableVector* shape);

struct BoardHash
{
    std::size_t operator()(const Board* board) const
    {
        return board->boardHash;
    }
};

struct BoardEqual
{
    std::size_t operator()(const Board& first, const Board& second) const
    {
        return first.hashString == second.hashString;
    }
};