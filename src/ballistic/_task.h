#pragma once

/**
 * Defines the task specific physics.
 */

#include "physics.h"

kScienceNSHeader() namespace task
{
	using namespace phys;
	using namespace dsolve;

	inline v3 simple_acc(double t, v3 &r, v3 &v)
	{
	    return -G * M * r / norm(r) / norm(r) / norm(r);
	}
	    
	inline v3 air_res_acc(double t, v3 &r, v3 &v)
	{
	    return -G * M * r / norm(r) / norm(r) / norm(r) - 1e-4 /* k/m */ * v;
	}
	    
	inline v3 earth_rot_acc(double t, v3 &r, v3 &v)
	{
		static const v3 w = { 0, 0, W }, rn = { r.x, r.y, 0 };
	    return -G * M * r / norm(r) / norm(r) / norm(r) + w * w * rn + 2 * (v ^ w);
	}

	inline v3 earth_rot_air_res_acc(double t, v3 &r, v3 &v)
	{
		static const v3 w = { 0, 0, W }, rn = { r.x, r.y, 0 };
		return -G * M * r / norm(r) / norm(r) / norm(r) - W * W * rn + 2 * (v ^ w) - 1e-4 /* k/m */ * v;
	}

	inline v3 energy(v3 &r, v3 &v)
	{
		double potential = -G * M / norm(r);
		double kinetic = norm(v) * norm(v) / 2;
		return{ potential, kinetic, potential + kinetic };
	}

	physics simple_physics = { simple_acc, energy };
	physics air_res_physics = { air_res_acc, energy };
	physics earth_rot_physics = { earth_rot_acc, energy };
	physics earth_rot_air_res_physics = { earth_rot_air_res_acc, energy };

	const double target_neighborhood = 10000;

} kScienceNSFooter()