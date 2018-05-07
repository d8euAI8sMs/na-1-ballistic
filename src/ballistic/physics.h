#pragma once

/**
 * Defines the global physics.
 */

#include "common.h"
#include "dsolve.h"

kScienceNSHeader() namespace phys
{
	using dsolve::dfunc;

	typedef struct 
	{
		v3 source, target;
		v3 velocity;

	} preconditions;

	typedef struct
	{
		double time;
		v3 r, v, energy;
	} state;

	typedef struct
	{
		bool reached;
		bool success;
		double distance;
	} fin_state;

	typedef struct
	{
		// Returns the complete energy (potential, kinetic, summary)
		typedef v3(*energy_func)(v3 &x, v3 &dx);

		dfunc acceleration;
		energy_func energy;
	} physics;

	typedef struct
	{
		preconditions state0;
		physics phys;

	} preconditionsex;

	bool operator==(physics &first, physics &second)
	{
		return first.acceleration == second.acceleration && first.energy == second.energy;
	}

	double R = 6370e3; // Earth's radius
	double G = 6.67e-11; // Gravitational constant
	double M = 5.97e24; // Earth's weight
	double W = 7.2921e-5; // Earth's angular velocity

} kScienceNSFooter()