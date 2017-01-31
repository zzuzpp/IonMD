#include <iostream>
#include <cmath>
#include <vector>
#include <thread>

#include <omp.h>

#include "ionmd.hpp"

namespace ionmd {

using arma::vec;
using arma::mat;
using std::cout;
using std::cerr;
using std::endl;


Simulation::Simulation() {
    status = SimStatus::IDLE;
}


Simulation::Simulation(SimParams p, Trap trap)
    : Simulation()
{
    this->p = std::make_shared<SimParams>(p);
    this->trap = std::make_shared<Trap>(trap);
}


Simulation::Simulation(SimParams p, Trap trap, std::vector<Ion> ions)
    : Simulation(p, trap)
{
    cout << ions.size();
    for (auto ion: ions) {
        this->ions.push_back(ion);
    }
    cout << this->ions.size();
}


mat Simulation::precompute_coulomb() {
    int i = 0;
    mat Flist(3, ions.size());
    #pragma omp parallel for
    for (auto ion: ions) {
        // FIXME
	// Flist.col(i) = ion.coulomb(ions);
	i++;
    }
    return Flist;
}


void Simulation::set_params(SimParams new_params) {
    if (status != SimStatus::RUNNING) {
        p = std::make_shared<SimParams>(new_params);
    }
    else {
        std::cerr << "Can't change parameters while simulation is running!\n";
    }
}



void Simulation::set_trap(Trap new_trap) {
    if (status != SimStatus::RUNNING) {
        trap = std::make_shared<Trap>(new_trap);
    }
    else {
        std::cerr << "Can't set a new trap while simulation is running!\n";
    }
}


Ion Simulation::make_ion(const double &m, const double &Z,
                         const std::vector<double> &x0)
{
    return Ion(p, trap, m, Z, x0);
}


void Simulation::set_ions(std::vector<Ion> ions) {
    if (status != SimStatus::RUNNING) {
        ions.clear();

        for (auto ion: ions) {
            ions.push_back(ion);
        }
    }
    else {
        std::cerr << "Can't set new ions while simulation is running!\n";
    }
}


void Simulation::run() {
    // Initialize RNG
    // std::mersenne_twister_engine<double> rng;

    // Set number of threads for multiprocessing
    // TOOD: convert to OpenCL and use GPU when available
    const int num_threads = std::thread::hardware_concurrency();
    omp_set_num_threads(num_threads);

    // Storage of pre-computed Coulomb force data
    mat coulomb_forces(3, p->num_ions);
    coulomb_forces.zeros();

    // FIXME: Initialization should happen before here
    // TODO: Initialize CCD
    // TODO: Initialize lasers

    // TODO: recording initialization

    // Run simulation
    int index = 0;
    int t_10 = (int)(p->t_max/p->dt) / 10;
    cout << "Simulating..." << endl;
    status = SimStatus::RUNNING;

    for (double t = 0; t < p->t_max; t += p->dt) {
        // Calculate Coulomb forces
        if (p->coulomb_enabled)
            coulomb_forces = precompute_coulomb();

	// Progress update
	if (index % t_10 == 0 && p->verbosity != 0) {
	    cout << int(10*index/t_10) << "% complete; "
		 << "t = " << t << "\n";
	}

        // Update each ion
	for (auto ion: ions) {
            // TODO: Record data

	    ion.update(t, coulomb_forces);
            cout << ion.x[0] << " " << ion.x[1] << " " << ion.x[2] << "\n";

	    // TODO: Check bounds
        }

        index++;
    }

    status = SimStatus::FINISHED;
}

}  // namespace ionmd
