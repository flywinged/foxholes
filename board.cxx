#include "board.hxx"

unordered_map<HashableVector*, vector<HashableVector*>*, VectorHash, VectorEqual> FLAT_TO_INDEX_MAP = unordered_map<HashableVector*, vector<HashableVector*>*, VectorHash, VectorEqual>();

unordered_map<HashableVector*, unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>, VectorHash, VectorEqual> INDEX_TO_FLAT_MAP = unordered_map<HashableVector*, unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>, VectorHash, VectorEqual>();

unordered_map<HashableVector*, HashableVector*, VectorHash, VectorEqual> DIMENSION_SIZE_MAP = unordered_map<HashableVector*, HashableVector*, VectorHash, VectorEqual>();

unordered_map<HashableVector*, vector<HashableVector*>, VectorHash, VectorEqual> STEPS_MAP = unordered_map<HashableVector*, vector<HashableVector*>, VectorHash, VectorEqual>();

unordered_map<HashableVector*, vector<HashableVector*>, VectorHash, VectorEqual> CORNER_MAP = unordered_map<HashableVector*, vector<HashableVector*>, VectorHash, VectorEqual>();

std::hash<string> STRING_HASHER;

// Basic Constructor
Board::Board(HashableVector* _shape)
{

    // Extract the dimensionality and shapes for the board
    shape = _shape;

    // Use the shape and dimensions to determine the total board size
    size = 1;
    for (int i = 0; i < shape->size; i++)
        size *= shape->values[i];
        
    // If an enumerate map hasn't been created for this size board yet, create it now.
    if (!FLAT_TO_INDEX_MAP.count(shape))
        enumerate();
    
    // Create the state map
    state = unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>();

}

// Copy constructor
Board::Board(Board* board)
{

    shape = board->shape;
    size = board->size;

    state = unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>();

    for (auto location = board->state.begin(); location != board->state.end(); location++)
        setBoardValue(location->first, board->state.at(location->first));

    copyHistory(board);

}

Board::~Board()
{

    for (int i = 0; i < checkHistory->size(); i++)
        for (auto check = checkHistory->at(i).begin(); check != checkHistory->at(i).end(); check++)
            delete (*check);
        
    delete checkHistory;

    for (auto pair : state)
        delete pair.first;
    
}

// Create a board with foxes starting on "odd" initial squares
Board* createInitialBoard(HashableVector* shape)
{

    // Create a null board first
    Board* initialBoard = new Board(shape);

    for (int i = 0; i < initialBoard->size; i++)
    {

        // Grab the appropriate index for this part of the board
        HashableVector* index = FLAT_TO_INDEX_MAP.at(initialBoard->shape)->at(i);

        // Add up all the location indexes for this point to determine if a fox could be there or not at the start
        int dimensionSum = 0;
        for (int d = 0; d < initialBoard->shape->size; d++)
            dimensionSum += index->values[d];
        
        if (dimensionSum % 2 == 1)
            initialBoard->setBoardValue(index, 1);

    }

    return initialBoard;

}

void Board::copyHistory(Board* otherBoard)
{

    for (int i = 0; i < otherBoard->checkHistory->size(); i++)
    {
        unordered_set<HashableVector*, VectorHash, VectorEqual> dayChecks;

        for (auto check = otherBoard->checkHistory->at(i).begin(); check != otherBoard->checkHistory->at(i).end(); check++)
            dayChecks.insert(new HashableVector(*check));
        
        checkHistory->push_back(dayChecks);

    }

}

Board* Board::propogate(bool increment)
{

    // Create the new board
    Board* newBoard = new Board(shape);

    // Copy over the check history
    newBoard->copyHistory(this);

    // For each location in the current board state, try moving the fox in each available step
    for (auto location = state.begin(); location != state.end(); location++)
    {

        HashableVector* testLocation = new HashableVector(*location->first);

        for (int d = 0; d < shape->size; d++)
        {

            // Create the positive step
            if (testLocation->values.at(d) < shape->values.at(d) - 1)
            {
                testLocation->values.at(d) += 1;
                
                if (newBoard->state.count(testLocation) == 0)
                    newBoard->setBoardValue(testLocation, 1);
                
                else if (increment)
                    newBoard->setBoardValue(testLocation, newBoard->state.at(testLocation) + 1);

                testLocation->values.at(d) -= 1;
            }
            

            // Create the negative step
            if (testLocation->values.at(d) > 0)
            {
                testLocation->values.at(d) -= 1;
                
                if (newBoard->state.count(testLocation) == 0)
                    newBoard->setBoardValue(testLocation, 1);
                
                else if (increment)
                    newBoard->setBoardValue(testLocation, newBoard->state.at(testLocation) + 1);

                testLocation->values.at(d) += 1;
            }

        }

        delete testLocation;

    }

    // Return the constructed board
    return newBoard;

}

// Update the board value at a given location
void Board::setBoardValue(HashableVector* location, int16_t value)
{

    HashableVector* newVector = new HashableVector(location);

    if (value == 0)
    {
        if (state.count(newVector) != 0)
        {
            auto it = state.find(newVector);
            state.erase(it->first);
            delete it->first;
        }
        
        delete newVector;
    }
    
    else
    {
        if (state.count(newVector) != 0)
        {
            state.at(newVector) = value;
            delete newVector;
        }
            
        else
            state.insert(pair<HashableVector*, int16_t>(newVector, value));
    }

}

// Determine if two boards are equal
bool Board::operator==(const Board &other) const
{

    // First check that the dimensions are equal
    if (shape->size != other.shape->size)
        return false;
    
    for (int d = 0; d < shape->size; d++)
        if (shape->values.at(d) != other.shape->values.at(d))
            return false;
    
    // If shape is the same, check all the values
    for (int i = 0; i < size; i++)
    {
        HashableVector* location = FLAT_TO_INDEX_MAP.at(shape)->at(i);

        if (state.count(location) == 1 || other.state.count(location) == 1 || state.at(location) != other.state.at(location))
                return false;
        
    }

    return true;

}

// Helper function for getting the board hash
bool compareCornerDistances(vector<int16_t>* vector1, vector<int16_t>* vector2)
{

    for (int i = 0; i < vector1->size(); i++)
    {
        if (vector1->at(i) < vector2->at(i))
            return true;
        else if (vector1->at(i) > vector2->at(i))
            return false;
    }
        
    return false;

}

size_t Board::getBoardHash()
{

    if (gotHash)
        return boardHash;

    // Determine the distance of every point in the board from each corner
    vector<vector<int16_t>*> cornerDistancesList = vector<vector<int16_t>*>(pow(2, shape->size));

    // Create the cornerList
    vector<HashableVector*> cornerList = CORNER_MAP.at(shape);

    // Now that we've created the list of corners, determine the distance of each corner to the point in question
    int16_t cornerIndex = 0;
    for (auto corner = cornerList.begin(); corner != cornerList.end(); corner++)
    {

        // Vector fo the distnaces of each point to the specified corner
        vector<int16_t>* cornerDistances = new vector<int16_t>();

        for (auto point = state.begin(); point != state.end(); point++)
            cornerDistances->push_back(*(point->first)^(**corner));

        // We want to sort the corner distance list afterward to be in ascending order
        std::sort(cornerDistances->begin(), cornerDistances->end());
        
        cornerDistancesList.at(cornerIndex) = cornerDistances;
        cornerIndex++;

    }

    // Sort the corner distance list
    std::sort(cornerDistancesList.begin(), cornerDistancesList.end(), compareCornerDistances);

    // Create the string hash
    string stringHash = "";
    for (auto cornerDistanceList = cornerDistancesList.begin(); cornerDistanceList != cornerDistancesList.end(); cornerDistanceList++)
    {

        for (auto cornerDistance = (*cornerDistanceList)->begin(); cornerDistance != (*cornerDistanceList)->end(); cornerDistance++){

            // Add a special character to indicate a new corner
            if (cornerDistance == (*cornerDistanceList)->begin() && cornerDistanceList != cornerDistancesList.begin())
                stringHash += "|";

            stringHash += to_string(*cornerDistance);
            
            // Only add a comma if we aren't at the end
            if (cornerDistance != (*cornerDistanceList)->end() - 1 && cornerDistanceList != cornerDistancesList.end())
                stringHash += "-";
        }

    }

    for (auto cornerDistanceList : cornerDistancesList)
        delete cornerDistanceList;
    
    boardHash = STRING_HASHER(stringHash);
    hashString = stringHash;
    return boardHash;

}

// Create all indexes possible for a board of this size and dimensions and add those to the FLAT_TO_INDEX_MAP
void Board::enumerate()
{

    // First we need to generate the dimension size map.
    vector<int16_t> dimensionSizesVector;
    for (int d = 0; d < shape->size; d++)
    {

        int16_t dimensionSize = 1;
        for (int j = d + 1; j < shape->size; j++)
            dimensionSize *= shape->values[j];
        
        dimensionSizesVector.push_back(dimensionSize);

    }

    // Convert dimensionSizes into and array
    HashableVector* hashableDimensionVector = new HashableVector(shape->size, dimensionSizesVector);

    // Assign the dimension size to the map
    DIMENSION_SIZE_MAP.insert(pair<HashableVector*, HashableVector*>(shape, hashableDimensionVector));

    // Now create the indices
    vector<HashableVector*>* indices = new vector<HashableVector*>();

    // Each location needs to create a hash based on the size of each dimension
    unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual> indexToFlatMap = unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>();

    for (int i = 0; i < size; i++)
    {

        // Create the individual index at this value
        HashableVector* hashableIndex = new HashableVector(shape->size);

        // Loop through each of the dimensions to determine the hash for i;
        int value = i;
        for (int d = 0; d < shape->size; d++)
        {
            hashableIndex->values[d] = value / DIMENSION_SIZE_MAP.at(shape)->values[d];
            value = value % DIMENSION_SIZE_MAP.at(shape)->values[d];
        }

        indices->push_back(hashableIndex);
        indexToFlatMap.insert(pair<HashableVector*, int16_t>(hashableIndex, i));

    }

    // Now we need to create the steps map
    vector<HashableVector*> steps = vector<HashableVector*>();
    for (int d = 0; d < shape->size; d++)
    {

        // Create the positive step
        HashableVector* stepUp = new HashableVector(shape->size);
        stepUp->values.at(d) = 1;
        steps.push_back(stepUp);

        // Create the negative step
        HashableVector* stepBack = new HashableVector(shape->size);
        stepBack->values.at(d) = -1;
        steps.push_back(stepBack);

    }

    // Create the corners map
    vector<HashableVector*> cornerList = vector<HashableVector*>();
    for (int i = 0; i < pow(2, shape->size); i++)
    {

        HashableVector* corner = new HashableVector(shape->size);

        for (int j = 0; j < shape->size; j++)
            if ((i / (int16_t)pow(2, j)) % 2 != 0)
                corner->values[j] = shape->values[j] - 1;
        
        cornerList.push_back(corner);
        
    }

    // Add all the maps to the global maps
    FLAT_TO_INDEX_MAP.insert(pair<HashableVector*, vector<HashableVector*>*>(new HashableVector(shape), indices));
    INDEX_TO_FLAT_MAP.insert(pair<HashableVector*, unordered_map<HashableVector*, int16_t, VectorHash, VectorEqual>>(new HashableVector(shape), indexToFlatMap));
    STEPS_MAP.insert(pair<HashableVector*, vector<HashableVector*>>(new HashableVector(shape), steps));
    CORNER_MAP.insert(pair<HashableVector*, vector<HashableVector*>>(new HashableVector(shape), cornerList));

}

string Board::show()
{

    string boardString = "\n";

    // To construct the board string, loop through the size of the board.
    for (int i = 0; i < size; i++)
    {
        
        HashableVector* index = FLAT_TO_INDEX_MAP.at(shape)->at(i);

        // For every new dimension which occurs, add a new line to the board string
        if (i != 0)
            for (int d = 0; d < shape->size - 1; d++)
                if (i % DIMENSION_SIZE_MAP.at(shape)->values[d] == 0)
                    boardString += "\n";
        
        if (state.count(index))
            boardString += to_string(state.at(index)) + " ";
        else
            boardString += "0 ";
        
    }

    return boardString + "\n";

}

int16_t Board::sum()
{

    int16_t total = 0;

    for (auto square : state)
        total += square.second;
    
    return total;

}

void Board::showCheckPath()
{

    Board* board = createInitialBoard(shape);

    std::cout << "Solution for board shape: " << shape->show() << endl;

    // Check each location and propogate a new board
    for (int i = 0; i < checkHistory->size(); i++)
    {

        Board *weightedPropogation = board->propogate(true);
        int16_t propogatedTotal = weightedPropogation->sum();
        int16_t actualTotal = board->sum();

        std::cout << endl;
        std::cout << board->show();

        std::cout << "| ";

        for (auto iterator = checkHistory->at(i).begin(); iterator != checkHistory->at(i).end(); iterator++)
        {

            // Remove the fox from this location on the new board
            board->setBoardValue(*iterator, 0);
            std::cout << (*iterator)->show() << " | ";

        }

        std::cout << endl;

        // Show statistics
        std::cout << "Foxes Left: " << actualTotal << endl;
        std::cout << "Movement Weights: " << propogatedTotal << endl;

        // Propogate the new board
        board = board->propogate();

        delete weightedPropogation;

    }

    std::cout << endl;
    std::cout << board->show();
    std::cout << endl;
    std::cout << "It took " << checkHistory->size() << " days to catch the fox!" << endl;
}