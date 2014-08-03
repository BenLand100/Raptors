#ifndef SIMULATION_HH
#define SIMULATION_HH

#include <iostream>
#include <vector>
#include <random>

#include "vec.hh"

typedef struct {
  //in/temp variables
  vec pos, vel, dir;
  //in params
  double max_vel, accel, drag;
  //out params
  vec *pos_hist, *vel_hist, *dir_hist;
  unsigned int steps;
} Entity;

enum Action { NONE, LEFT, RIGHT, /* not a valid action */ NUM_ACTIONS };
static std::uniform_int_distribution<int> rand_action(0,NUM_ACTIONS);

class Simulation;

class Mutator {
    public:
        virtual void operator()(Action *src, Action *dest, Simulation *sim) = 0;
};

class UniformMutator : public Mutator {
    public:
        UniformMutator(int min_mutations, int max_mutations, int min_mutate_idx, int max_mutate_idx);
        virtual void operator()(Action *src, Action *dest, Simulation *sim);
        
    protected:
        std::uniform_int_distribution<int> mutate_times;
        std::uniform_int_distribution<int> mutate_idx;
};

class Simulation {
    
    public:
        Simulation(int nsamples_, int nsteps_, double tstep_, Mutator &init, double kill_dist_  = 0.1, double prune_percent_ = 0.995, int seed = 8675309);
        ~Simulation();
        
        void stepGeneration();
        void selectGeneration();
        void pruneGeneration();
        void evolveGeneration(Mutator &mutator);
        void resetGeneration();
        
        void dump(std::ostream &out);
        
        const int nsamples, nsteps;
        const double tstep, kill_dist;
        std::mt19937 rng;
        
    protected:
        const double prune_percent;
        vec man_pos, raptor_pos, run_dir;
        std::vector<Entity> man, raptor;
        std::vector<Action*> actions;
        std::vector<bool> survived;
        std::vector<int> prune_order;
        
        void initFirstGen(Mutator &init);
        
        void stepEntity(Entity &e);
        bool runSim(Entity &man, Entity &raptor, Action *actions);
        
};

#endif
