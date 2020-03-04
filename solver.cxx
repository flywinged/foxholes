#include "solver.hxx"

vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* createCombinations(
    Board* board,
    int16_t checksLeft,
    unordered_map<HashableVector*, unordered_set<HashableVector*, VectorHash, VectorEqual>, VectorHash, VectorEqual>* removeMap,
    vector<HashableVector*>* locationsToRemove,
    unordered_set<HashableVector*, VectorHash, VectorEqual> currentCheckLocations,
    vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* combinations,
    int16_t depth
    )
{

    // If the last index is greater than the size of the locations to remove, add the combination and continue
    if (checksLeft == 0)
        combinations->push_back(currentCheckLocations);
    
    else
    {

        vector<HashableVector*> subLocationsToRemove;
        for (int i = 0; i < locationsToRemove->size(); i++)
        {
            int16_t count = 0;
            for (auto v : removeMap->at(locationsToRemove->at(i)))
                if (currentCheckLocations.count(v) == 0)
                    count++;
            
            if (count != 0)
                subLocationsToRemove.push_back(locationsToRemove->at(i));
            
        }

        if (subLocationsToRemove.size() == 0)
        {
            combinations->push_back(currentCheckLocations);
            return combinations;
        }

        std::sort(
            subLocationsToRemove.begin(),
            subLocationsToRemove.end(),
            [removeMap, currentCheckLocations](HashableVector* v1, HashableVector* v2)
            {

                int16_t v1Count = 0;
                for (auto v : removeMap->at(v1))
                    if (!currentCheckLocations.count(v))
                        v1Count++;
                
                int16_t v2Count = 0;
                for (auto v : removeMap->at(v2))
                    if (!currentCheckLocations.count(v))
                        v2Count++;
                
                return v1Count < v2Count;
            }
        );

        int16_t checkableLocations = subLocationsToRemove.size();
        if (checkableLocations > 6)
            checkableLocations = 6;

        // Otherwise, try more locations to determine what other checks can be made.
        for (int i = 0; i < checkableLocations; i++)
        {

            auto indexRemoveMap = removeMap->at(subLocationsToRemove.at(i));

            // Find how many elements in indexRemove map aren't in the locations which are already being checked
            vector<HashableVector*> newLocations = vector<HashableVector*>();

            for (auto location : indexRemoveMap)
                if ( currentCheckLocations.count(location) == 0 )
                    newLocations.push_back((location));

            // If the size of the resulting vector is smaller than the checks we have left, we can check this location as well
            if (newLocations.size() <= checksLeft)
            {

                // Update the current checks and removed indices
                for (int j = 0; j < newLocations.size(); j++)
                    currentCheckLocations.insert(newLocations.at(j));

                // Call the next iteration of the combinations call
                createCombinations(
                    board,
                    checksLeft - newLocations.size(),
                    removeMap,
                    locationsToRemove,
                    currentCheckLocations,
                    combinations,
                    depth + 1
                );

                // Update the current checks and removed indices
                for (int j = 0; j < newLocations.size(); j++)
                    currentCheckLocations.erase(newLocations.at(j));
                                
            }

            // Add the combination if nothing else worked
            else if (i == locationsToRemove->size() - 1)
                combinations->push_back(currentCheckLocations);

        }
    }

    if (depth == 0)
    {
        delete removeMap;
        return combinations;
    }

}

vector<unordered_set<HashableVector*, VectorHash, VectorEqual>>* getCheckCombinations(Board* board, int16_t checks)
{
    
    // Determine the squares that can be moved to from this location
    Board* weightedMovement = board->propogate(true);

    // Get a list of all available places to remove from the next day
    vector<HashableVector*>* locationsToRemove = new vector<HashableVector*>();
    for (auto iterator = weightedMovement->state.begin(); iterator != weightedMovement->state.end(); iterator++)
        if (iterator->second <= checks)
            locationsToRemove->push_back(iterator->first);
    
    // Create a map of these locations to try and remove. Map the removal location to all of the adjacent squares
    unordered_map<HashableVector*, unordered_set<HashableVector*, VectorHash, VectorEqual>, VectorHash, VectorEqual>* removeMap = new unordered_map<HashableVector*, unordered_set<HashableVector*, VectorHash, VectorEqual>, VectorHash, VectorEqual>();

    for (auto iterator = weightedMovement->state.begin(); iterator != weightedMovement->state.end(); iterator++)
    {

        unordered_set<HashableVector*, VectorHash, VectorEqual> removeLocationSet = unordered_set<HashableVector*, VectorHash, VectorEqual>();

        HashableVector* testLocation = new HashableVector(*iterator->first);

        for (int d = 0; d < board->shape->size; d++)
        {

            // Create the positive step
            if (testLocation->values.at(d) < board->shape->values.at(d) - 1)
            {
                testLocation->values.at(d) += 1;
                
                if (board->state.count(testLocation) != 0 && removeLocationSet.count(testLocation) == 0)
                {
                    HashableVector* newVector = new HashableVector(testLocation);
                    removeLocationSet.insert(newVector);
                }

                testLocation->values.at(d) -= 1;
            }
            

            // Create the negative step
            if (testLocation->values.at(d) > 0)
            {
                testLocation->values.at(d) -= 1;
                
                if (board->state.count(testLocation) != 0 && removeLocationSet.count(testLocation) == 0)
                {
                    HashableVector* newVector = new HashableVector(testLocation);
                    removeLocationSet.insert(newVector);
                }

                testLocation->values.at(d) += 1;
            }

        }

        removeMap->insert(pair<HashableVector*, unordered_set<HashableVector*, VectorHash, VectorEqual>>(iterator->first, removeLocationSet));
    }
    
    // Now search through all combinations of the places to check to determine
    return createCombinations(board, checks, removeMap, locationsToRemove);

}

void singleBoard(
    int id,
    Board* board,
    int16_t checksPerDay,
    unordered_set<Board*, BoardHash, BoardEqual>* newBoards,
    unordered_set<size_t>* allBoards,
    bool* inserting
)
{

    int16_t initialBoardSum = board->sum();

    // Get the check combinations for this board
    auto checkCombinations = getCheckCombinations(board, checksPerDay);

    // Check each location and propogate a new board
    for (int i = 0; i < checkCombinations->size(); i++)
    {

        Board* newBoard = new Board(board);
        for (auto iterator = checkCombinations->at(i).begin(); iterator != checkCombinations->at(i).end(); iterator++)
        {
            // Remove the fox from this location on the new board
            newBoard->setBoardValue(*iterator, 0);
        }

        // Propogate the new board
        Board* propogatedNewBoard = newBoard->propogate();

        if (propogatedNewBoard->sum() > initialBoardSum)
        {
            delete newBoard;
            delete propogatedNewBoard;
            continue;
        }

        // Set the hash for the new board
        propogatedNewBoard->getBoardHash();

        // Determine if the new board has already been found. If it hasn't add it to the new boards
        if (allBoards->count(propogatedNewBoard->boardHash) == 0)
        {

            // Account for the history of the new board
            propogatedNewBoard->checkHistory->push_back(checkCombinations->at(i));

            // Put the new board in both the new boards and all boards

            while (*inserting)
                std::this_thread::sleep_for(std::chrono::microseconds(10));

            (*inserting) = true;

            newBoards->insert(propogatedNewBoard);
            allBoards->insert(propogatedNewBoard->boardHash);

            (*inserting) = false;
        }

        else
            delete propogatedNewBoard;
        
        delete newBoard;
        
    }

    delete checkCombinations;
}


void poolSolve(
    vector<Board*>* boardsToUse,
    int16_t checksPerDay,
    unordered_set<Board*, BoardHash, BoardEqual>* newBoards,
    unordered_set<size_t>* allBoards
)
{
    
    int THREADS = 22;
    ctpl::thread_pool pool(THREADS);

    bool inserting = false;

    for (int t = 0; t < boardsToUse->size(); t++)
        pool.push(
            singleBoard,
            boardsToUse->at(t),
            checksPerDay,
            newBoards,
            allBoards,
            &inserting
        );

    while (pool.n_idle() != THREADS)
        std::this_thread::sleep_for((std::chrono::milliseconds(1)));

}

Board* solveForShape(HashableVector* shape, int16_t checksPerDay)
{

    // Create the default board
    Board *initialBoard = createInitialBoard(shape);

    // Create the set of all previously visited board states
    unordered_set<size_t>* allBoards = new unordered_set<size_t>();

    // Create the set of all new board states and all old board states
    unordered_set<Board*, BoardHash, BoardEqual>* newBoards = new unordered_set<Board*, BoardHash, BoardEqual>();
    unordered_set<Board*, BoardHash, BoardEqual>* oldBoards = new unordered_set<Board*, BoardHash, BoardEqual>();

    // Because of the way hashes work, need to initialize each board hash
    initialBoard->getBoardHash();
    oldBoards->insert(initialBoard);
    allBoards->insert(initialBoard->boardHash);

    bool solving = true;
    Board* solvedBoard;

    int16_t loopCounter = 0;

    Board* newBoard;
    vector<Board*>* boardsToUse = new vector<Board*>;

    while (solving)
    {

        // std::cout << "Number of boards to check: " << boards << endl;
        int16_t minimumFoxesLeft = initialBoard->size;
        for (auto board = oldBoards->begin(); board != oldBoards->end(); board++)
        {

            if ((*board)->state.size() < minimumFoxesLeft)
                minimumFoxesLeft = (*board)->state.size();
            
            if ((*board)->state.size() == 0)
            {
                (*board)->solved = true;
                return *board;
            }
        }

        boardsToUse->clear();
        for (auto board = oldBoards->begin(); board != oldBoards->end(); board++)
            if ((int16_t)(*board)->state.size() <= minimumFoxesLeft)
                boardsToUse->push_back((*board));
        
        int16_t boards = boardsToUse->size();

        if (boards == 0)
        {
            std::cout << "No solution found." << endl;
            return solvedBoard;
        }

        std::cout << endl;
        std::cout << "Day: " << loopCounter << endl;
        std::cout << "Boards: " << boards << endl;
        std::cout << "Foxes Left: " << minimumFoxesLeft << endl;
        
        poolSolve(boardsToUse, checksPerDay, newBoards, allBoards);

        delete oldBoards;

        // Now change the set references accordingly
        oldBoards = newBoards;
        newBoards = new unordered_set<Board*, BoardHash, BoardEqual>();
        loopCounter++;

    }

    return initialBoard;

}