#include <iostream>
#include <cmath>
#include <atomic>

#include <armadillo>

#include <ionmd/ion.hpp>
#include <ionmd/util.hpp>
#include <ionmd/constants.hpp>

using std::cout;
using arma::vec;
using arma::mat;
using arma::normalise;

using namespace ionmd;


Ion::Ion(params_ptr params, trap_ptr trap, double m, double Z)
    : charge(Z * constants::q_e), v(3), a(3), m(m), Z(Z)
{
    this->p = params;
    this->trap = trap;
    this->v.zeros();
    this->a.zeros();
}


Ion::Ion(params_ptr params, trap_ptr trap, const double m, const double Z, vec x0)
    : Ion(params, trap, m, Z)
{
    this->x = x0;
}


Ion::Ion(params_ptr params, trap_ptr trap, lasers_ptr lasers,
         double m, double Z, vec x0)
    : Ion(params, trap, m, Z)
{
    this->lasers = lasers;
    this->x = x0;
}


const vec Ion::update(const double &t, const mat &forces,
                      const unsigned int &index)
{
    vec dx = v*p->dt + 0.5*a*pow(p->dt, 2);
    x += dx;

    vec F = secular_force()
        + micromotion_force(t)
        + coulomb_force(forces, index)
        + stochastic_force()
        + doppler_force();

    auto accel = F / m;
    v += 0.5*(a + accel)*p->dt;
    a = accel;

    return x;
}


vec Ion::doppler_force() {
    vec F = arma::zeros<vec>(3);

    if (p->doppler_enabled)
    {
        for (const auto &laser: lasers) {
            F = laser->F0 * laser->wave_vector - laser->beta * v;
        }
    }

    return F;
}


const vec Ion::coulomb(const std::vector<Ion> &ions)
{
    vec F = arma::zeros<vec>(3);

    for (const auto &other: ions)
    {
        if (this == &other) {
            continue;
        }
        vec r = other.x - this->x;
        F += (other.charge * r / pow(arma::norm(r), 3));
    }

    return constants::OOFPEN * this->charge * F;
}


vec Ion::coulomb_force(const mat &forces, const unsigned int &index)
{
    if (p->coulomb_enabled) {
        return forces.col(index);
    }
    else {
        return arma::zeros<vec>(3);
    }
}


vec Ion::secular_force()
{
    vec F = arma::zeros<vec>(3);

    if (p->secular_enabled)
    {
        const double A = charge*pow(trap->V_rf, 2)/(m*pow(trap->omega_rf, 2)*pow(trap->r0, 4));
        const double B = trap->kappa*trap->U_ec/(2*pow(trap->z0, 2));
        F[0] = -2 * charge * (A - B) * x[0];
        F[1] = -2 * charge * (A - B) * x[1];
        F[2] = -4 * charge * B * x[2];
    }
    return F;
}


vec Ion::micromotion_force(double t)
{
    vec F = arma::zeros<vec>(3);

    if (p->micromotion_enabled) {
        // FIXME
        return F;
    }
    else {
        return F;
    }
}


vec Ion::stochastic_force()
{
    vec F = arma::zeros<vec>(3);

    if (p->stochastic_enabled) {
        // FIXME
        return F;
    }
    else {
        return F;
    }

    // FIXME
    // double v;
    // vec direction = {uniform(rng), uniform(rng), uniform(rng)};
    // vec hat = normalise(direction);
    // v = 0; // sqrt(2*kB*p->gamma_col*p->dt/this->m);
    // F = this->m*v*hat/p->dt;
    // return F;
}
