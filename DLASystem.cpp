//
//  DLASystem.cpp
//

#include "DLASystem.h"

using std::cout;
using std::endl;
using std::stringstream;

// this function gets called every step,
//   if there is an active particle then it gets moved,
//   if not then add a particle
void DLASystem::Update() {
    if (lastParticleIsActive == 1) {
        moveLastParticle();
    }
    else if (particleList.size() < endNum) {
        addParticleOnAddCircle();
        setParticleActive();
    }
    else {
        running = false;
    }
}


void DLASystem::clearParticles() {
    // delete particles and the particle list
    for (auto & i : particleList) {
        delete i;
    }
    particleList.clear();
}

// remove any existing particles and setup initial condition
void DLASystem::Reset() {
    // stop running
    running = 0;

    clearParticles();

    lastParticleIsActive = 0;

    // set the grid to zero
    for (int i = 0; i < gridSize; i++) {
        for (int j = 0; j < gridSize; j++) {
            grid[i][j] = 0;
        }
    }

    // setup initial condition and parameters
    addCircle = 10;
    killCircle = 2.0 * addCircle;
    clusterRadius = 0.0;
    // add a single particle at the origin
    double pos[] = {0.0, 0.0};
    addParticle(pos);

}

// set the value of a grid cell for a particular position
// note the position has the initial particle at (0,0)
// but this corresponds to the middle of the grid array ie grid[ halfGrid ][ halfGrid ]
void DLASystem::setGrid(const double pos[], int val) {
    int halfGrid = gridSize / 2;
    grid[(int) (pos[0] + halfGrid)][(int) (pos[1] + halfGrid)] = val;
}

// read the grid cell for a given position
int DLASystem::readGrid(const double pos[]) {
    int halfGrid = gridSize / 2;
    return grid[(int) (pos[0] + halfGrid)][(int) (pos[1] + halfGrid)];
}


// add a particle to the system at a specific position
void DLASystem::addParticle(double pos[]) {
    // create a new particle
    auto *p = new Particle(pos);
    // push_back means "add this to the end of the list"
    particleList.push_back(p);

    // pos coordinates should be -gridSize/2 < x < gridSize/2
    setGrid(pos, 1);
}

// add a particle to the system at a random position on the addCircle
// if we hit an occupied site then we do nothing except print a message
// (this should never happen)
void DLASystem::addParticleOnAddCircle() {
    double pos[2];
    double theta = rgen.random01() * 2 * M_PI;
    pos[0] = ceil(addCircle * cos(theta));
    pos[1] = ceil(addCircle * sin(theta));
    if (readGrid(pos) == 0)
        addParticle(pos);
    else
        cout << "FAIL " << pos[0] << " " << pos[1] << endl;
}

// send back the position of a neighbour of a given grid cell
// NOTE: there is no check that the neighbour is inside the grid,
// this has to be done separately...
void DLASystem::setPosNeighbour(double setpos[], const double pos[], int val) {
    switch (val) {
        case 0:
            setpos[0] = pos[0] + 1.0;
            setpos[1] = pos[1];
            break;
        case 1:
            setpos[0] = pos[0] - 1.0;
            setpos[1] = pos[1];
            break;
        case 2:
            setpos[0] = pos[0];
            setpos[1] = pos[1] + 1.0;
            break;
        case 3:
            setpos[0] = pos[0];
            setpos[1] = pos[1] - 1.0;
            break;
    }
}

// when we add a particle to the cluster, we should update the cluster radius
// and the sizes of the addCircle and the killCircle
void DLASystem::updateClusterRadius(double pos[]) {

    double newRadius = distanceFromOrigin(pos);
    if (newRadius > clusterRadius) {
        clusterRadius = newRadius;
        // this is how big addCircle is supposed to be:
        //   either 20% more than cluster radius, or at least 5 bigger.
        double check = clusterRadius * addRatio;
        if (check < clusterRadius + 5)
            check = clusterRadius + 5;
        // if it is smaller, then update everything...
        if (addCircle < check) {
            addCircle = check;
            killCircle = killRatio * addCircle;
        }
    }
}

// make a random move of the last particle in the particleList
void DLASystem::moveLastParticle() {
    int NESW = rgen.randomInt(4);  // pick a random number in the range 0-3, which direction do we hop?
    double newpos[2];

    Particle *lastP = particleList[particleList.size() - 1];

    setPosNeighbour(newpos, lastP->pos, NESW);

    if (distanceFromOrigin(newpos) > killCircle) {
        setGrid(lastP->pos, 0);
        particleList.pop_back();  // remove particle from particleList
        setParticleInactive();
    }
        // check if destination is empty
    else if (readGrid(newpos) == 0) {
        setGrid(lastP->pos, 0);  // set the old grid site to empty
        // update the position
        particleList[particleList.size() - 1]->pos[0] = newpos[0];
        particleList[particleList.size() - 1]->pos[1] = newpos[1];
        setGrid(lastP->pos, 1);  // set the new grid site to be occupied

        // check if we stick
        if (checkStick()) {
            setParticleInactive();  // make the particle inactive (stuck)
            updateClusterRadius(lastP->pos);  // update the cluster radius, addCircle, etc.

            stringstream str;
            str << newpos[0] << "," << newpos[1] << "," << clusterRadius << "\n";
            cluster.push_back(str.str());
        }
    }
    else {
        //Splits collision into two cases, not bump will roll until a physical move is made
        if (not bump) {
            moveLastParticle();
        }
        // bump will attempt to stick after each roll regardless of movement
        else {
            if (checkStick()) {
                setParticleInactive();  // make the particle inactive (stuck)
                updateClusterRadius(lastP->pos);  // update the cluster radius, addCircle, etc.

                stringstream str;
                str << newpos[0] << "," << newpos[1] << "," << clusterRadius << "\n";
                cluster.push_back(str.str());
            }
            else {
                moveLastParticle();
            }
        }
    }
}



// check if the last particle should stick (to a neighbour)
int DLASystem::checkStick() {
    Particle *lastP = particleList[particleList.size() - 1];
    int result = 0;
    // loop over neighbours
    for (int i = 0; i < 4; i++) {
        double checkpos[2];
        setPosNeighbour(checkpos, lastP->pos, i);
        // if the neighbour is occupied...
        if (readGrid(checkpos) == 1){
           if (stickProbability > rgen.random01()) {
               result = 1;
           }
        }
    }
    return result;
}


// constructor
DLASystem::DLASystem(int maxParticles, float probability,bool bump1)
        : clusterRadius(),addCircle(), killCircle(), running(), lastParticleIsActive(), cluster() {
    cout << "creating system, gridSize " << gridSize << endl;
    endNum = maxParticles;
    stickProbability = probability;
    bump = bump1;

    // allocate memory for the grid, remember to free the memory in destructor
    grid = new int *[gridSize];
    for (int i = 0; i < gridSize; i++) {
        grid[i] = new int[gridSize];
    }
    // reset initial parameters
    Reset();

    addRatio = 1.2;   // how much bigger the addCircle should be, compared to cluster radius
    killRatio = 1.7;   // how much bigger is the killCircle, compared to the addCircle

}

// destructor
DLASystem::~DLASystem() {
    // strictly we should not print inside the destructor but never mind...
    cout << "deleting system" << endl;
    // delete the particles
    clearParticles();
    // delete the grid
    for (int i = 0; i < gridSize; i++)
        delete[] grid[i];
    delete[] grid;

}