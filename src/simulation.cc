#include <cstring>
#include <algorithm>
#include <cmath>

#include "simulation.hh"
#include "vec.hh"

using namespace std;

UniformMutator::UniformMutator(int min_mutations, int max_mutations, int min_mutate_idx, int max_mutate_idx) 
    : mutate_times(min_mutations,max_mutations), mutate_idx(min_mutate_idx,max_mutate_idx) {

}

void UniformMutator::operator()(Action *src, Action *dest, Simulation *sim) {
    memcpy(dest,src,sizeof(Action)*sim->nsteps);
    int mutations = mutate_times(sim->rng);
    for (int i = 0; i < mutations; i++) {
        dest[mutate_idx(sim->rng)] = (Action)rand_action(sim->rng);
    }
}

Simulation::Simulation(int nsamples_, int nsteps_, double tstep_, double kill_dist_, double prune_percent_, Mutator *mutator_, int seed) : 
    nsamples(nsamples_), nsteps(nsteps_), tstep(tstep_), kill_dist(kill_dist_), prune_percent(prune_percent_), mutate(mutator_), rng(seed) {
    if (!mutate) {
        mutate = new UniformMutator(1,5,0,nsteps);
    }
    
    man_pos.x = 0.0;
    man_pos.y = 0.0;
    run_dir.x = -1.0;
    run_dir.y = 0.0;
    raptor_pos.x = 10.0;
    raptor_pos.y = 0.0;
    
    initFirstGen();
}

Simulation::~Simulation() {
    delete mutate;
    for (int i = 0; i < nsamples; i++) {
        delete actions[i];
        delete man[i].pos_hist;
    }
}

void Simulation::initFirstGen() {
    man.resize(nsamples);
    raptor.resize(nsamples);
    actions.resize(nsamples);
    survived.resize(nsamples);
    prune_order.resize(nsamples);
    
    //start with empty actions to mutate into first gen
    Action *init_actions = new Action[nsteps];
    for (int i = 0; i < nsteps; i++) {
        init_actions[i] = NONE;
    }
    
    for (int i = 0; i < nsamples; i++) { 
        //actions init
        actions[i] = new Action[nsteps];
        (*mutate)(init_actions,actions[i],this);
        
        //pruning init
        prune_order[i] = i;
        survived[i] = true;
        
        //man init
        man[i].pos_hist = new vec[nsteps];
        man[i].vel_hist = NULL;
        man[i].dir_hist = NULL;
        man[i].steps = 0;
        copy(man_pos, man[i].pos);
        man[i].vel.x = 0.0;
        man[i].vel.y = 0.0;
        copy(run_dir, man[i].dir);
        man[i].max_vel = 6.0;
        man[i].accel = 5.2; 
        man[i].drag = 0.8;
        
        //raptor init
        raptor[i].pos_hist = new vec[nsteps];
        raptor[i].vel_hist = NULL;
        raptor[i].dir_hist = NULL;
        raptor[i].steps = 0;
        copy(raptor_pos, raptor[i].pos);
        raptor[i].vel.x = 0.0;
        raptor[i].vel.y = 0.0;
        raptor[i].dir.x = 0.0;
        raptor[i].dir.y = 0.0;
        raptor[i].max_vel = 25.0;
        raptor[i].accel = 20.0;
        raptor[i].drag = 0.8; 
    }
}

void Simulation::stepEntity(Entity &e) {
    vec accel;
    copy(e.dir,accel);
    scale(accel,e.accel);
    linadd(e.vel,e.vel,-e.drag*tstep);
    linadd(e.vel,accel,tstep);
    if (mag(e.vel) >= e.max_vel) {
        unit(e.vel);
        scale(e.vel,e.max_vel);
    }
    linadd(e.pos,e.vel,tstep);
    if (e.pos_hist) { copy(e.pos,e.pos_hist[e.steps]); }
    if (e.vel_hist) { copy(e.vel,e.vel_hist[e.steps]); }
    if (e.dir_hist) { copy(e.dir,e.dir_hist[e.steps]); }
    e.steps++;
}

bool Simulation::runSim(Entity &man, Entity &raptor, Action *actions) {
    for (int i = 0; i < nsteps; i++) {
        //is he dead yet
        if (dist(man.pos,raptor.pos) <= kill_dist) return true;
        
        //perform step action
        switch (actions[i]) {
            case NONE:
                break;
            case LEFT:
                rotate(man.dir,90.0*M_PI/180.0);
                break;
            case RIGHT:
                rotate(man.dir,-90.0*M_PI/180.0);
                break;
        }
        
        //point raptor at man
        raptor.dir = sub(man.pos,raptor.pos);
        unit(raptor.dir);
        
        //advance entities
        stepEntity(man);
        stepEntity(raptor);
        
    }
    return false;
}

void Simulation::resetGeneration() {
    for (int i = 0; i < nsamples; i++) {
        survived[i] = true;
    
        man[i].steps = 0;
        copy(man_pos, man[i].pos);
        man[i].vel.x = 0.0;
        man[i].vel.y = 0.0;
        copy(run_dir, man[i].dir);
        
        raptor[i].steps = 0;
        copy(raptor_pos, raptor[i].pos);
        raptor[i].vel.x = 0.0;
        raptor[i].vel.y = 0.0;
        raptor[i].dir.x = 0.0;
        raptor[i].dir.y = 0.0;
    }
}

void Simulation::pruneGeneration() {
    double mean_steps = 0.0;
    for (int i = 0; i < nsamples; i++) {
        mean_steps += man[i].steps;
    }
    mean_steps /= nsamples;
    double stdev_steps = 0.0;
    for (int i = 0; i < nsamples; i++) {
        stdev_steps += (man[i].steps - mean_steps)*(man[i].steps - mean_steps);
    }
    stdev_steps = sqrt(stdev_steps / nsamples);
    cout << "Mean time: " << tstep*mean_steps << '\n';
    cout << "StDev time: " << tstep*stdev_steps << '\n';
    
    vector<Entity> &man_ = man;
    sort(prune_order.begin(),prune_order.end(),[&man_](int a, int b) {return man_[a].steps < man_[b].steps;});
    
    for (int i = 0; i < prune_percent*nsamples; i++) {
        survived[prune_order[i]] = false;
    }
}

void Simulation::dump(ostream &out) {
    out.precision(5);
    out << fixed;
    for (int n = 0; n < 25; n++) {
        int i = prune_order[nsamples-1-n];
        out << '{';
        out << '{';
        for (int j = 0; j < man[i].steps; j++) {
            out << '{' << man[i].pos_hist[j].x << ',' << man[i].pos_hist[j].y << '}';
            if (j+1 != man[i].steps) out << ",";
        }
        out << "},{";
        for (int j = 0; j < raptor[i].steps; j++) {
            out << '{' << raptor[i].pos_hist[j].x << ',' << raptor[i].pos_hist[j].y << '}';
            if (j+1 != raptor[i].steps) out << ",";
        }
        out << "}}\n";
    }
}

void Simulation::evolveGeneration() {
    int nchildren = 0;
    for (int i = 0; i < nsamples; i++) {
        if (survived[i]) nchildren++;
    }
    nchildren = (int)ceil((double)(nsamples-nchildren)/nchildren);
    
    for (int fill = 0, breed = 0, child = 0; ; fill++, child++) {
        while (fill < nsamples && survived[fill]) { fill++; }
        if (child = nchildren) { while (breed < nsamples && !survived[breed]) { breed++; } }
        if (fill == nsamples || breed == nsamples) break;
        
        (*mutate)(actions[breed],actions[fill],this);
        
    }   
}

void Simulation::stepGeneration() {
    #pragma omp parallel for
    for (int i = 0; i < nsamples; i++) {
        runSim(man[i],raptor[i],actions[i]);
    }
}
