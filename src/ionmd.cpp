#include <cstdio>
#include <iostream>
#include <boost/format.hpp>
#include <cmath>
#include <vector>
#include <thread>

#include "H5Cpp.h"
#include <omp.h>

#include <armadillo>

#include "ionmd.hpp"
#include "ion.hpp"
#include "trap.hpp"

using arma::vec;
using arma::mat;
using std::cout;
using std::cerr;
using namespace ionmd;

/**
   Precomputes all Coulomb interactions (that way they can be applied
   all at once instead of having one ion updated before the Coulomb
   interaction is computed for the rest).

   TODO: convert to OpenCL
*/
mat precompute_coulomb(std::vector<Ion> ions) {
    int i = 0;
    mat Flist(3, ions.size());
    #pragma omp parallel for
    for (auto ion: ions) {
	Flist.col(i) = ion.coulomb(ions);
	i++;
    }
    return Flist;
}

/**
   Main entry point to run simulations.

   TODO: exception handling (e.g., see HDF5 C++ examples)
   TODO: return codes
   TODO: messaging to interrupt
 */
int simulate(const SimParams &p, const Trap &trap) {
    // Initialize RNG
    std::mersenne_twister_engine rng;

    // Set number of threads for multiprocessing
    // TOOD: convert to OpenCL and use GPU when available
    const int num_threads = std::thread::hardware_concurrency();
    omp_set_num_threads(num_threads);

    // Storage of pre-computed Coulomb force data
    mat coulomb_forces = arama(3, p->num_ions);
    coulomb_forces.zeros();

    // Initialize CCD
    // TODO: reimplement

    // Initialize ions
    float M = 0;
    std::vec<Ion> ions;
    const vec x0 = arma::zeros<vec>(3);
    for (unsigned int i = 0; i < p.num_ions; i++) {
	// TODO: place on grid
	// TODO: figure out how to specify mass and charge in params
        ions.push_back(Ion(&p, &trap, lasers, 40, 1, x0));
    }

    // Data recording initialization
    auto *output_file = new H5::H5File(p.output_filename, H5::H5F_ACC_TRUNC);

    // Run simulation
    int index = 0;
    int t_10 = (int)(p.t_max/p.dt) / 10;
    printf("Simulating...\n");

    for(double t = 0; t < p.t_max; t += p.dt) {
        // Calculate Coulomb forces
        if (p.coulomb_enabled)
            coulomb_forces = precompute_coulomb(ions);

	// Progress update
	if (index % t_10 == 0 && p.verbosity != 0) {
	    cout << int(10*index/t_10) << "% complete; "
		 << "t = " << t << "\n";
	}

        // Update each ion
	for (auto ion: ions) {
            // TODO: Record data

	    ion.update(t, coulomb_forces);

	    // TODO: Check bounds
        }

        index++;
    }

    // TODO: Save data

    // Cleanup
    delete output_file;

    return 0;
}
