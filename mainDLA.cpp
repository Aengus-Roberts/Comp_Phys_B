#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "DLASystem.h"

using std::cout;
using std::endl;
using std::getline;

// this is a global pointer, which is how we access the system itself
DLASystem *sys;

void runProgram(int seed, float stickProbability, int maxParticles, bool bump) {
    // create the system
    sys = new DLASystem(maxParticles, stickProbability, bump);

    // this is the seed for the random numbers
    cout << "setting seed " << seed << endl;
    sys->setSeed(seed);
    sys->running = true;

    while (sys->running) {
        sys->Update();
    }
    // after system has been modelled output to a file with seed-prob-maxP as title
    stringstream title;
    title << "data/" << seed << "-" << stickProbability << "-" << maxParticles<< "-" << bump << ".csv";
    ofstream runOutput(title.str());
    // iterates through cluster to write to file
    for (int i = 0; i < maxParticles; i++) {
        runOutput << (sys->cluster[i]);
    }
    runOutput.close();
}

int main() {
    // variables required to obtain values from setup.csv
    string values, word;
    bool bump;
    int seed, maxParticles, column;
    float stickProbability;

    ifstream MyReadFile("setup.csv");
    // iterates through lines in setup file
    while (getline(MyReadFile, values)) {
        std::stringstream s(values);
        column = 0;
        //reads through line to get seed, stickProbability, maxParticles and assigns to variables
        while (getline(s, word, ',')) {
            if (column == 0) {
                seed = stoi(word);
            } else if (column == 1) {
                stickProbability = stof(word);
            } else if (column == 2) {
                maxParticles = stoi(word);
            }
            else if (column == 3) {
                istringstream(word) >> bump;
            }
            column += 1;
        }
        runProgram(seed, stickProbability, maxParticles, bump);
    }
    //closing file
    MyReadFile.close();
    return 0;
}



