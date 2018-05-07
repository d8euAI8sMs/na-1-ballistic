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
#include "region.h"
#include <map>

kScienceNSHeader()

/// <summary>
///
///     The class represents a single-valued function.
///
/// </summary>
class Function
{

public:

	/// <summary>
	///
	///     Returns value associated with the given x.
	///
	/// <param>
	///     If no value is associated with the given x,
	///     quiet NaN must be returned.
	/// </param>
	///
	/// </summary>
	virtual double operator()(double x) = 0;

	virtual ~Function()
	{
	}

};

/// <summary>
///
///     Represents a function defined inside the certain region.
///
/// </summary>
class RegionedFunction :
	virtual public Function
{

	Function *delegated;
	const Region *region;
	const double defaultValue;

public:

	RegionedFunction
		(
		Function *delegated,
		Region *region,
		double defaultValue = std::numeric_limits<double>::quiet_NaN()
		) :
		delegated(delegated), region(region), defaultValue(defaultValue)
	{
	}

public:

	/// <summary>
	///
	///     Returns value associated with the given x.
	///
	/// <param>
	///     If x is outside of the specified region,
	///     <c>defaultValue must be returned</c>
	/// </param>
	///
	/// </summary>
	virtual double operator()(double x)
	{
		if (region->contains(x)) return (*delegated)(x);
		return defaultValue;
	}

};

/// <summary>
///
///     The class represents a function that caches its values.
///
/// </summary>
class CacheableFunction :
	virtual public Function
{

	Function *delegated;

	const unsigned int maxSize;
	const Region *cacheableRegion;
	const double error;
	std::map<Point, double> cache;

public:

	CacheableFunction
		(
		Function *delegated,
		double error = double_error,
		unsigned int maxSize = 100,
		Region *cacheableRegion = nullptr
		) :
		delegated(delegated), maxSize(maxSize), cacheableRegion(cacheableRegion), error(error)
	{
	}

public:

	virtual void clearCache()
	{
		cache.clear();
	}

	/// <summary>
	///
	///     Returns value associated with the given x.

	/// <param>
	///     If cache contains value for key in range [x-error, x+error],
	///     value from cache will be returned. Otherwise, the delegated
	///     function will be called and its return value will be cached.
	/// </param>
	///
	/// <param>
	///     If <c>cacheableRegion</c> is provided, caching is only
	///     applied to those x that are contained in that region.
	/// </param>
	///
	/// </summary>
	virtual double operator()(double x)
	{
		if (cacheableRegion == nullptr || cacheableRegion->contains(x))
		{
			std::map<Point, double>::iterator it = cache.find(Point(x, error));
			if (it != cache.end())
			{
				return it->second;
			}
		}

		double val = (*delegated)(x);

		if (cache.size() < maxSize) cache[Point(x, error)] = val;

		return val;
	}

};

/// <summary>
///
///     The simple function is a wrapper for any C++ function
///     that takes double as argument and returns double.
///
/// </summary>
class SimpleFunction :
	virtual public Function
{

	double(*function)(double x);

public:

	SimpleFunction(double(*function)(double x)) : function(function)
	{
	}

public:

	virtual double operator()(double x)
	{
		return function(x);
	}

};

kScienceNSFooter()