#pragma once

#include <GLUT/glut.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>

#include <math.h>
#include <random>
#include <string>
#include <sstream>
#include <format>

#include "Particle.h"
#include "rnd.h"

using namespace std;

class DLASystem {
private:
    // these are private variables and functions that the user will not see

    // list of particles
    vector<Particle *> particleList;

    // delete particles and clear the particle list
    void clearParticles();

    // size of cluster
    double clusterRadius;
    // these are related to the DLA algorithm
    double addCircle;
    double killCircle;

    // size of grid
    static const int gridSize = 1600;
    int **grid;  // this will be a 2d array that stores whether each site is occupied


    // random number generator, class name is rnd, instance is rgen
    rnd rgen;

    // output file (not used at the moment)
    ofstream logfile;

    // number of particles at which the simulation will stop
    // (the value is set in constructor)
    int endNum;
    float stickProbability;

    // the values of these variables are set in the constructor
    double addRatio;    // how much bigger the addCircle should be, compared to cluster radius
    double killRatio;   // how much bigger is the killCircle, compared to the addCircle


public:
    // these are public variables and functions

    // update the system: if there is an active particle then move it,
    // else create a new particle (on the adding circle)
    void Update();

    // is the simulation running (1) or paused (0) ?
    int running;

    // lastParticleIsActive is +1 if there is an active particle in the system, otherwise 0
    int lastParticleIsActive;

    //a vector of particle co-ords and clusterRadius at the time of adding
    vector<string> cluster;

    //parameter to define collision logic for probabilistic case
    bool bump;

    // constructor
    DLASystem(int maxParticles, float stickProbability, bool bump);

    // destructor
    ~DLASystem();

    // delete all particles and reset
    void Reset();

    // this sets the seed for the random numbers
    void setSeed(int s) { rgen.setSeed(s); }

    // if pos is outside the cluster radius then set clusterRadius to be the distance to pos.
    void updateClusterRadius(double pos[]);

    // set and read grid entries associated with a given position
    void setGrid(const double pos[], int val);

    int readGrid(const double pos[]);

    // return the distance of a given point from the origin
    static double distanceFromOrigin(const double pos[]) {
        return sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
    }

    // set whether there is an active particle in the system or not
    void setParticleActive() { lastParticleIsActive = 1; }

    void setParticleInactive() { lastParticleIsActive = 0; }

    // add a particle at pos
    void addParticle(double pos[]);

    // add a particle at a random point on the addCircle
    void addParticleOnAddCircle();

    // assign setpos to the position of a neighbour of pos
    // which neighbour we look at is determined by val (=0,1,2,3)
    static void setPosNeighbour(double setpos[], const double pos[], int val);

    // this attempts to move the last particle in the List to a random neighbour
    // if the neighbour is occupied then nothing happens
    // the function also checks if the moving particle should stick.
    void moveLastParticle();

    // check whether the last particle should stick
    // currently it sticks whenever it touches another particle
    int checkStick();
};
