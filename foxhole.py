from typing import Tuple, List, Set

import numpy as np
from copy import deepcopy

from itertools import combinations

from multiprocessing import Pool
from functools import partial

def calculateBoardHash(A: np.ndarray):

    flat = A.flatten()

    hashValue = 0
    for i in range(flat.shape[0]):
        if flat[i] == 1:
            hashValue += 2**i
    return hashValue

class Board:
    '''
    Board state object.

    Parameters
    ----------
    boardShape - tuple of ints describing the how loarge the board is. This board is limited to n-dimensional rectangles.
        Make the int values in decreasing order as this will be the assumpion of the object
    '''

    binaryLists = {}

    @classmethod
    def createBinaryList(cls, n: int):
        '''

        '''

        if n in cls.binaryLists:
            return

        L = []

        for i in range(2**n):
            L.append(np.array([0 if c is "0" else 1 for c in "{0:b}".format(i).rjust(n, "0")]))

        cls.binaryLists[n] = L

    def __init__(self, boardShape: Tuple[int]):
        '''

        '''

        self.board: np.ndarray = np.zeros(boardShape, dtype = np.int)
        self.dimensions: int = len(boardShape)

        self.symettricDimensions = [

        ]

        dimensionList = []
        lastDimension: int = None
        for i in range(len(boardShape)):
            d = boardShape[i]
            if lastDimension is None or d == lastDimension:
                dimensionList.append(i)
            else:
                self.symettricDimensions.append(dimensionList)
                dimensionList = [i]
            
            lastDimension = d

        self.symettricDimensions.append(dimensionList)

        self.hash: int = None

        self.checkHistory = []
    
    def copy(self) -> "Board":
        '''

        '''

        newBoard = Board(self.board.shape)
        newBoard.board = np.copy(self.board, order="F")
        newBoard.checkHistory = deepcopy(self.checkHistory)

        return newBoard

    def propogate(self, increment:bool = False) -> "Board":
        '''

        '''

        newBoard = self.copy()
        newBoard.board.fill(0)

        locations = np.array(np.where(self.board > 0)).T

        for index in locations:
                
            index = np.array(index)
            for d in range(self.dimensions):

                dArray = np.zeros((self.dimensions,), dtype = np.int)
                dArray[d] = 1
                newIndexLow = tuple(index - dArray)
                newIndexHigh= tuple(index + dArray)

                if newIndexLow[d] >= 0:

                    if not increment:
                        newBoard.board[newIndexLow] = 1
                    else:
                        newBoard.board[newIndexLow] += 1

                if newIndexHigh[d] < self.board.shape[d]:

                    if not increment:
                        newBoard.board[newIndexHigh] = 1
                    else:
                        newBoard.board[newIndexHigh] += 1
    
        return newBoard

    def __hash__(self):
        '''

        '''

        Board.createBinaryList(self.dimensions)

        if self.hash is None:

            boardCopy = np.copy(self.board, order="F")
            
            # Determine the distance metric for each corner orientation
            bestCorner, bestCornerValue = None, None
            for corner in self.binaryLists[self.dimensions]:
                cornerJustification = (np.array(boardCopy.shape) - 1) * corner
                
                # Calculate the distance of every location on the grid from the decided corner.
                distances = np.zeros(boardCopy.shape)
                for index, x in np.ndenumerate(boardCopy):
                    if x != 0:
                        distances[index] = np.linalg.norm(np.array(index) - cornerJustification)
                
                cornerValue = np.sum(distances)
                if bestCornerValue is None or cornerValue < bestCornerValue:
                    bestCorner = corner
                    bestCornerValue = cornerValue
            
            # Now that the best corner has been found, flip all the axis on the boardCopy to match
            flipAxis = []
            for i in range(len(corner)):
                if bestCorner[i] == 1:
                    flipAxis.append(i)
            newBoard = np.flip(self.board, axis = tuple(flipAxis))

            # Once the corner has been chosen and everything is oriented correctly, we now need to determine the best rotation of axes for all axes whose dimension is the same size
            newAxes = []
            for L in self.symettricDimensions:
                
                # Order all the symetric dimensions in decreasing order
                dimensionDict = {}
                for d in L:

                    # Calculate the distance of every location on the grid from the decided corner.
                    distances = np.zeros(boardCopy.shape, dtype = np.int)
                    for index, x in np.ndenumerate(newBoard):
                        if x != 0:
                            distances[index] = np.array(index)[d]
                    
                    dimensionDict[d] = np.sum(distances)
                
                L.sort(key = lambda d: dimensionDict[d])

                newAxes += L
            
            newBoard = np.moveaxis(newBoard, list(np.arange(self.dimensions)), newAxes)

            self.hash = calculateBoardHash(newBoard)

        return int(self.hash)
    
    def showPath(self):

        testBoard = Board(self.board.shape)
        for index, _ in np.ndenumerate(testBoard.board):

            if np.sum(np.array(index)) % 2 == 1:
                testBoard.board[index] = 1
        
        for checks in self.checkHistory:

            print()
            print(testBoard)
            print(checks)

            for check in checks:
                testBoard.board[check] = 0
            testBoard = testBoard.propogate()

    def __eq__(self, other: "Board"):
        return self.__hash__() == other.__hash__()

    def __repr__(self):
        return self.board.__str__()


def createCheckCombinations(board: Board, checks: int) -> List[List[np.ndarray]]:
    '''

    '''

    def getRemoveLocations(currentBoard: Board) -> List[np.ndarray]:

        # Determine locations which will have a fox in them in the next iteration if nothing is done
        weightedMovement = currentBoard.propogate(increment=True)
        weightedMovement.board[np.where(weightedMovement.board > checks)] = 0

        removeLocations = list(np.array(np.where(weightedMovement.board > 0)).T)
        return removeLocations

    def getCombinations(currentCombination: Set[Tuple[int]] = None, checksMade: int = 0, checkCombinations: List[Set[Tuple[int]]] = None, depth = 0):     

        if currentCombination is None:
            currentCombination = set()
        
        if checkCombinations is None:
            checkCombinations = []
        
        # Create the board state if all the removeLocations specified by the current combination are removed
        boardCopy = board.copy()

        for check in currentCombination:
            boardCopy.board[check] = 0
        
        # Determine how many squares could be removed
        weightedMovement = boardCopy.propogate(increment=True)
        weightedMovement.board[np.where(weightedMovement.board > checks - checksMade)] = 0

        removeLocations = list(np.array(np.where(weightedMovement.board > 0)).T)
        for i in range(len(removeLocations)):
            removeLocations[i] = tuple(removeLocations[i])

        if len(removeLocations) == 0:
            
            if depth == 0:
                return checkCombinations
            
            checkCombinations.append(currentCombination)
            return

        for location in removeLocations:

            # Determine which squares need to be checks to remove this location
            fakeBoard = Board(boardCopy.board.shape)
            fakeBoard.board[location] = 1

            propogatedFakeBoard = fakeBoard.propogate()

            placesToCheck = propogatedFakeBoard.board * boardCopy.board
            placesToCheck = list(np.array(np.where(placesToCheck > 0)).T)
            for i in range(len(placesToCheck)):
                placesToCheck[i] = tuple(placesToCheck[i])
            placesToCheck = set(placesToCheck)
            
            nextCombination = currentCombination | placesToCheck
            for combination in checkCombinations:
                if nextCombination.issubset(combination) or combination.issubset(nextCombination):
                    break

            else:
            
                getCombinations(
                    currentCombination=nextCombination,
                    checksMade=checksMade + weightedMovement.board[location],
                    checkCombinations=checkCombinations,
                    depth=depth+1
                )
        
        if depth == 0:
            return checkCombinations

    checkCombinations = getCombinations()

    return checkCombinations

import time

def singleStep(singleBoard, checks):
    checkCombinations = createCheckCombinations(singleBoard, checks)

    boards = []

    for checkCombination in checkCombinations:
        newBoard = singleBoard.copy()

        for index in checkCombination:
            newBoard.board[tuple(index)] = 0

        propogatedBoard = newBoard.propogate()
        propogatedBoard.checkHistory.append(checkCombination)
        propogatedBoard.__hash__()
        boards.append(propogatedBoard)
    
    return boards

def solveBoardSize(boardSize: Tuple[int], checks: int, maxLoops: int = 32, leniency: int = 1, log:bool = True) -> Board:
    '''

    '''

    # Create the initial board state accounting for "odd" and "even" holes
    defaultBoard = Board(boardSize)
    for index, _ in np.ndenumerate(defaultBoard.board):

        if np.sum(np.array(index)) % 2 == 1:
            defaultBoard.board[index] = 1
    
    allBoardStates = set([defaultBoard])
    lastBoardStates = set([defaultBoard])
    newBoardStates = set()

    loopCounter = 0
    while len(lastBoardStates) > 0:
        
        newBoardStates = set()

        # print()
        # find the board state with the least values
        boardStates = list(lastBoardStates)
        boardStates.sort(key = lambda x: np.sum(x.board))

        if log:
            print()
            print("Day:", loopCounter)
            print("Number of New Board States:", len(lastBoardStates))
            print("Minimum Number of Potential Squares:", np.sum(boardStates[0].board))

        # NOTE: For speed, but could potentially miss some solutions. It is WAY faster however
        checkBoardStates = [boardState for boardState in boardStates if np.sum(boardState.board) <= np.sum(boardStates[0].board) + leniency]
        lastBoardStates = set(checkBoardStates)

        if np.sum(boardStates[0]) == 0:
            return boardStates[0]
        
        print(1)
        pooledBoardStates = None
        with Pool(16) as p:
            pooledBoardStates = p.map_async(partial(singleStep, checks=checks), lastBoardStates).get()
        print(2)
        
        for boardList in pooledBoardStates:
            for boardState in boardList:

                if boardState is not None and boardState not in allBoardStates:
                    newBoardStates.add(boardState)
                    allBoardStates.add(boardState)

        lastBoardStates = newBoardStates

        loopCounter += 1
        if loopCounter == maxLoops:
            print("Broke via Loop Counter")
            return
    # Once the starting board state is created, now we want to 

# for n in range(10):
#     for checks in range(1, 12):
#         solution = solveBoardSize((n, n), checks)
#         if solution is None:
#             print(f"{n}x{n} - {checks}: inf")
        
#         else:
#             print(f"{n}x{n} - {checks}: {len(solution.checkHistory)}")

t1 = time.time()
solution = solveBoardSize((2,2,2,2,2,2,2,2), 46, maxLoops=64, leniency=3, log=True)
# solution = solveBoardSize((8, 8), 5, log=True, leniency=1)
t2 = time.time()
if solution is not None:
    # solution.showPath()
    print("\t", len(solution.checkHistory), t2 - t1)
else:
    print("\t", "No Solution", t2 - t1)

# for i in range(2, 7):
#     for j in range(2, 7):
#         print()
#         print(f"Grid Size:({i}, {j})")
#         t1 = time.time()
#         solution = solveBoardSize((i,j), 6, maxLoops=64, leniency=3, log=False)
#         t2 = time.time()

#         if solution is not None:
#             # solution.showPath()
#             print("\t", len(solution.checkHistory), t2 - t1)
#         else:
#             print("\t", "No Solution", t2 - t1)

# A = Board((3, 4))
# A.board = np.array([
#     [0, 0, 0],
#     [1, 0, 1],
#     [0, 1, 1],
#     [0, 1, 0]
# ])

# print(A)
# print(A.propogate())

# B = Board((3, 4))
# B.board = np.array([
#     [0, 1, 0],
#     [1, 1, 0],
#     [1, 0, 1],
#     [0, 0, 0]
# ])

# print(A.__hash__())
# print(B.__hash__())