/*
Copyright(C) 2015 Alexander Vasilevsky aka kalaider

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "kScience.h"
#include "point.h"
#include <vector>

kScienceNSHeader()

/// <summary>
///
///     The class represents a region of reals.
///
/// </summary>
class Region {

public:

	/// <summary>
	///
	///     Checks if the given element is element of the region.
	///
	/// </summary>
	virtual bool contains(double x) const = 0;

};

/// <summary>
///
///     The class combines multiple regions using AND or OR rule.
///
/// </summary>
class CompoundRegion :
	virtual public Region
{

public:

	static enum class Rule
	{
		AND, OR
	};

private:

	const Rule rule;
	std::vector<Region *> regions;

public:

	CompoundRegion(Rule rule = Rule::OR) : rule(rule)
	{
	}

	std::vector<Region *> & getRegions()
	{
		return regions;
	}

public:

	virtual bool contains(double x) const
	{
		std::vector<Region *>::const_iterator it = regions.begin();
		switch (rule)
		{
		case Rule::AND:
			for (; it < regions.end(); it++)
			{
				if (!(*it)->contains(x)) return false;
			}
			return true;
		case Rule::OR:
		default:
			for (; it < regions.end(); it++)
			{
				if ((*it)->contains(x)) return true;
			}
			return false;
		}
	}

};

/// <summary>
///
///     The simple [a, b] or (a, b) region (depending on sign of error).
///
/// </summary>
class SimpleRegion :
	virtual public Region
{

	const double xmin, xmax;
	const double error;

public:

	SimpleRegion
		(
		double xmin = -std::numeric_limits<double>::infinity(),
		double xmax = std::numeric_limits<double>::infinity(),
		double error = double_error) :
		xmin(xmin), xmax(xmax), error(error)
	{
	}

public:

	virtual bool contains(double x) const
	{
		if (xmin - x > error) return false;
		if (x - xmax > error) return false;
		return true;
	}

};

/// <summary>
///
///     The simple [x0-error, x0+error] region.
///
/// </summary>
class PointRegion :
	virtual public Region
{

public:

	/// <summary>
	///
	///     Produces region with n + 1 equidistant points.
	///
	/// </summary>
	static CompoundRegion * forStep
		(
		double x0,
		double h,
		int n,
		double error = double_error
		)
	{
		CompoundRegion *region = new CompoundRegion();
		region->getRegions().resize(n + 1);
		for (int i = 0; i <= n; i++)
		{
			region->getRegions().at(i) = new PointRegion(x0 + h * i, error);
		}
	}

	/// <summary>
	///
	///     Produces region with n + 1 equidistant points,
	///     including <c>a</c> and <c>b</c>.
	///
	/// </summary>
	static CompoundRegion * forNPoints
		(
		double a,
		double b,
		int n,
		double error = double_error
		)
	{
		return forStep(a, (b - a) / n, n, error);
	}

private:

	const Point point;

public:

	PointRegion(double x, double error = double_error) :
		point(x, error)
	{
	}

public:

	virtual bool contains(double x) const
	{
		return point == x;
	}

};

kScienceNSFooter()