#pragma once

/**
 * The worker thread generates the current coordinates,
 * velocity and energy of the missile.
 * 
 * Though the synchronization of access is necessary, it
 * it not mandatory in such a simple case.
 */

#include "physics.h"
#include <Windows.h>

kScienceNSHeader() namespace worker
{
	using namespace phys;
	using namespace dsolve;

	double default_intersection_error = 1e-3;

	typedef struct
	{
		long sleep; // The time in milliseconds the thread must sleep for

		physics phys; // The system of differential equations (SODE)

		double dt; // The step for the SODE parameter (time)
		int step; // How many dt 's included in interval between notifications
		double target_neighborhood;
		bool ignore_earth; // Is Earth existence must be ignored
		preconditions state0; // The initial location and velocity

		// The function that is to be called each time
		// the SODE parameter is changed
		void(*notify)(state st, fin_state fin);

		bool stop;
	} exchange;


	/**
	 * The method determines an intersection point
	 * of the missile trajectory and the Earth surface.
	 * 
	 * It searches for such dt (delta of time) which
	 * gives the solution that meets the possible error.
	 * 
	 * The function uses dichotomy method.
	 * 
	 * @param ex    The thread settings
	 * @param t     The "current" time
	 * @param s0    The "previous" state
	 * @param s     The "current" state
	 * @param error The possible mismatch
	 *              (distance between the missile and the Earth's surface)
	 * 
	 * @return true if the required precision achieved, false otherwise
	 */
	bool intersection
		(
		exchange *ex,
		double __outp &t, // after bump
		solution s0, // before bump
		solution __outp &s, // after bump
		double error
		)
	{

		// Check for trivial case first

		if (R - norm(s.x) < error) // R >= r
		{
			return true;
		}

		if (norm(s0.x) - R < error) // R <= r
		{
			s = s0;
			t -= ex->dt;
			return true;
		}

		// Calculate non-trivial case using dichotomy method
		// applied to dt

		double t0 = t - ex->dt;
		double dt0 = 0, dt = ex->dt;
		double r;
		while(true)
		{
			double x = (dt0 + dt) / 2;
			s = runge_cutta(ex->phys.acceleration, t0, x, s0.x, s0.dx);
			r = norm(s.x);
			if (R > r)
			{
				dt = x;
				if (R - r < error)
				{
					t = t0 + dt;
					return true;
				}
			}
			else dt0 = x;

			// To avoid infinite loop
			if ((dt - dt0) < 1e-15) return false; 
		}
	}

	/**
	 * The method is a worker thread main method.
	 * 
	 * The method performs the following operations:
	 * 
	 *     1. Calculates the current state of the system
	 *        and notifies the exchange about that new state
	 *        
	 *     2. Checks if the final state is met and notifies
	 *        the exchange if that check succeeds
	 * 
	 * The detailed contract of the method is complemented
	 * by a number of notes:
	 * 
	 *     1) The method notifies the exchange at occurrence
	 *        of the final state or at each n-th plain state change (step),
	 *        where n is exchange::step parameter
	 *        
	 *     2) The missile-Earth intersection is determines with the
	 *        default_intersection_error error. If the error is too
	 *        small and the required precision cannot be met,
	 *        the method generates debug assertion failure.
	 *        
	 *     3) The thread sleeps exchange::sleep milliseconds after
	 *        each plain state change notification. It doesn't sleep
	 *        after final state notification.
	 *        
	 *     4) The exchange::stop parameter is used to gracefully
	 *        shutdown the thread.
	 *
	 *     5) The exchange::stop parameter is used to gracefully
	 *        shutdown the thread.
	 *        
	 *     6) After the final state notification the method returns 0
	 *        and the thread stops.
	 */
	UINT worker_thread(LPVOID pParam)
	{
		// Extract exchange object
		exchange *ex = ((exchange*)pParam);

		// Read preconditions
		double t = 0;
		v3 r = ex->state0.source;
		v3 v = ex->state0.velocity;

		int step = 0;

		solution s0;

		// Calculate the current state
		do
		{
			++step;

			solution s = runge_cutta(ex->phys.acceleration, t, ex->dt, r, v);

			v = s.dx;
			r = s.x;

			// Check for bump
			if (step != 1 && !ex->ignore_earth)
			{
				// If intersection occurred
				if (norm(s0.x) >= R && norm(s.x) <= R)
				{
					bool success = intersection(ex, t, s0, s, default_intersection_error);

					ASSERT(success);

					double cosine = ((s.x * ex->state0.target) / norm(s.x)) / norm(ex->state0.target);
					// Bug caused by less of precision: in some cases
					// cosine may appear with value greater than 1,
					// e.g. 1.0000000000000002 (real example)
					// That bug causes inadequate result when the target
					// and source point are the same and velocity is
					// normal to the Earth's surface at that point.
					if (cosine > 1) cosine = 1;
					if (cosine < -1) cosine = -1;

					double alpha = acos(cosine);

					double arc = R * alpha;
					
					if (arc <= ex->target_neighborhood)
					{
						ex->notify({ t, s.x, s.dx, ex->phys.energy(s.x, s.dx) }, { true, true, arc });
						return 0;
					}
					else
					{
						ex->notify({ t, s.x, s.dx, ex->phys.energy(s.x, s.dx) }, { true, false, arc });
						return 0;
					}
				}
			}

			// Notify state change
			if (step % ex->step == 0)
			{
				ex->notify({ t, r, v, ex->phys.energy(r, v) }, { false });
				Sleep(ex->sleep);
			}

			t += ex->dt;
			s0 = s;

		} while (!ex->stop);

		return 0;
	}

} kScienceNSFooter()