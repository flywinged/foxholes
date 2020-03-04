#include "iostream"
#include "vector"

using namespace std;

#include "solver.hxx"

int main(int argc, char** argv)
{

    int16_t dimensions = std::stoi(argv[1]);
    vector<int16_t> dimensionSizes(dimensions);
    int i;
    for (i; i < dimensions; i++)
        dimensionSizes.at(i) = std::stoi(argv[i + 2]);

    int16_t checks = std::stoi(argv[i + 2]);

    HashableVector* A = new HashableVector(dimensions, dimensionSizes);

    Board* solvedBoard = solveForShape(A, checks);

    if (solvedBoard->solved)
        solvedBoard->showCheckPath();

    // std::cout << solvedBoard->

    // Board b = createInitialBoard(&A);
    
    // Board* c = b.propogate(true);
    // Board* d = c->propogate();

    // // b.getBoardHash();

    // std::cout << b.show() << endl;
    // std::cout << c->show() << endl;
    // std::cout << d->show() << endl;
}