#ifndef ION_HPP
#define ION_HPP

#include <vector>
#include <armadillo>
#include "ionmd.hpp"
#include "laser.hpp"
#include "trap.hpp"
#include "params.hpp"

using arma::vec;
using arma::dot;
using arma::mat;

namespace ionmd {
    typedef std::vector<Laser *> lasers_t;

    class Ion {
    public:
	// Position, velocity, acceleration
	vec x, v, a;

	// Mass and charge (in units of e)
	double m, Z;

	// Pointer to trap
	Trap *trap;

	// Laser cooling parameters
	bool doppler_coolable;
	double doppler_omega0;  // resonance Doppler cooling frequency
	double doppler_width;  // Doppler cooling linewidth
	lasers_t doppler_lasers;

	SimParams *p;

	Ion(SimParams *p, Trap *trap, lasers_t lasers, double m, double Z, vec x0);

	void update(double t, mat forces);

	// Pretty printing
	void print_position();

	// For pre-computing the Coulomb force due to all other ions
	vec coulomb(const std::vector<Ion> ions);

    private:
	// Unique ID
	int id;

	// Forces
	vec doppler_force();
	vec secular_force();
	vec coulomb_force(mat forces);
	vec micromotion_force(double t);
	vec stochastic_force();
    };
}

#endif
