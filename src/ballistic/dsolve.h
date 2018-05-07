#pragma once

/**
 * Implements Runge-Cutta method of differential equation solving.
 */

#include "common.h"

kScienceNSHeader() namespace dsolve
{

	typedef v3(*dfunc)(double p, v3 &x, v3 &dx);

	typedef struct {
		v3 x, dx;
	} solution;

	typedef struct {
		v3 k1, k2, k3, k4;
	} coefs;

	inline coefs get_coefs(dfunc fn, double p, double h, v3 &x, v3 &dx)
	{
		coefs c;
		c.k1 = fn(p, x, dx) * h;
		c.k2 = fn(p + h / 2, x + dx * h / 2, dx + c.k1 / 2) * h;
		c.k3 = fn(p + h / 2, x + dx * h / 2 + c.k1 / 4 * h, dx + c.k2 / 2) * h;
		c.k4 = fn(p + h, x + dx * h + c.k2 / 2 * h, dx + c.k3) * h;
		return c;
	}

	// solves the passed vector differential equation
	// fn - the vector function
	// p - the scalar parameter
	// h - the scalar parameter step
	// x - the initial condition
	// dx - the initial condition for the derivative
	inline solution runge_cutta(dfunc fn, double p, double h, v3 &x, v3 &dx)
	{
		coefs c = get_coefs(fn, p, h, x, dx);
		return
		{
			x + dx * h + (c.k1 + c.k2 + c.k3) / 6 * h,
			dx + (c.k1 + 2 * c.k2 + 2 * c.k3 + c.k4) / 6
		};
	}

} kScienceNSFooter()
