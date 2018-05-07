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

kScienceNSHeader()

/// <summary>
///
///     The class represents a point with some neighborhood around it.
///     
/// <para>
///     Is usable to compare doubles.
/// </para>
///
/// </summary>
class Point {

public:

	double x;
	double error;

	Point(double x, double error = double_error) :
		x(x), error(abs(error))
	{
	}

	bool operator<(const Point &other) const
	{
		if (x < other.x && *this != other) return true;
		return false;
	}
	bool operator>(const Point &other) const
	{
		if (x > other.x && *this != other) return true;
		return false;
	}
	bool operator<=(const Point &other) const
	{
		if (x <= other.x || *this == other) return true;
		return false;
	}
	bool operator>=(const Point &other) const
	{
		if (x >= other.x || *this == other) return true;
		return false;
	}
	bool operator==(const Point &other) const
	{
		return abs(x - other.x) < error + other.error;
	}
	bool operator!=(const Point &other) const
	{
		return !(other == *this);
	}
};

kScienceNSFooter()