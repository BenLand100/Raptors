#include <iostream>
#include <fstream>
#include <sstream>

#include "simulation.hh"

using namespace std;

#define NSAMPLES 5000
#define NSTEPS 10000
#define TINC 0.01
#define KILLDIST 0.1

int main(int argc, char **argv) {
    UniformMutator mutate(1,5,0,NSTEPS-1);
    Simulation sim(NSAMPLES,NSTEPS,TINC,mutate,KILLDIST);
    for (int i = 0; i < 1000; i++) {
        cout << "Step: " << i << '\n';
        sim.stepGeneration();
        cout << "Prune...\n";
        sim.pruneGeneration();
        ofstream dat;
        stringstream f;
        f << "data/" << i << ".list";
        dat.open(f.str());
        sim.dump(dat);
        dat.close();
        cout << "Evolve...\n";
        sim.evolveGeneration(mutate);
        cout << "Reset...\n";
        sim.resetGeneration();
    }
}
