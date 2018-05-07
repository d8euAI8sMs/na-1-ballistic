/*
Copyright(C) 2016 Alexander Vasilevsky aka kalaider

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
#include "math.h"

#include <ostream>

kScienceMathNSHeader()

class sfloat
{

public:

	long long mantissa;
	int       exponent;
	double    real;

public:

	sfloat(const sfloat &other)
		: mantissa(other.mantissa)
		, exponent(other.exponent)
		, real(other.real)
	{
	}

	sfloat(long long mantissa, int exponent)
		: mantissa(mantissa)
		, exponent(exponent)
		, real(static_cast<double>(mantissa))
	{
		shift(exponent);
	}

	sfloat(double real, int n_of_digits)
		: real(real)
		, mantissa(0)
		, exponent(0)
	{
		if (abs(real) < 1.e-15) return;
		int exp = static_cast<int>(floor(log10(abs(real))));
		shift(-exp + n_of_digits - 1);
		this->mantissa = static_cast<long long>(round(this->real));
		this->exponent = exp - n_of_digits + 1;
		while (this->mantissa != 0 && this->mantissa % 10 == 0)
		{
			this->exponent += 1;
			this->mantissa /= 10;
		}
		this->real = static_cast<double>(this->mantissa);
		shift(exponent);
	}

	int exp() const
	{
		int e = 0;
		long long m = this->mantissa;
		while (m != 0)
		{
			m /= 10;
			e++;
		}
		return e + this->exponent;
	}

	sfloat & operator *= (long long n)
	{
		this->mantissa *= n;
		this->real *= n;
		return *this;
	}

	sfloat & operator /= (long long n)
	{
		this->mantissa /= n;
		this->real /= n;
		return *this;
	}

	sfloat & operator += (const sfloat &other)
	{
		long long other_m = other.mantissa;
		int exp1 = this->exponent;
		int exp2 = other.exponent;
		unsigned int diff = abs(exp1 - exp2);
		if (exp1 < exp2)
		{
			for (unsigned int e = 0; e < diff; e++)
			{
				other_m *= 10;
			}
			exp2 -= diff;
		}
		else
		{
			for (unsigned int e = 0; e < diff; e++)
			{
				this->mantissa *= 10;
			}
			exp1 -= diff;
		}
		
		this->mantissa += other_m;
		this->real = static_cast<double>(mantissa);
		this->exponent = exp1;
		while (this->mantissa != 0 && this->mantissa % 10 == 0)
		{
			this->exponent += 1;
			this->mantissa /= 10;
		}
		shift(exp1);

		return *this;
	}

	sfloat & operator <<= (int exponent)
	{
		shift(exponent);
		this->exponent += exponent;
		return *this;
	}

	sfloat & operator >>= (int exponent)
	{
		shift(-exponent);
		this->exponent -= exponent;
		return *this;
	}

	bool operator == (const sfloat &other)
	{
		return (this->mantissa == other.mantissa) && (this->exponent == other.exponent);
	}

private:

	void shift(int exponent)
	{
		unsigned int ex = abs(exponent);
		for (unsigned int e = 0; e < ex; e++)
		{
			if (exponent > 0) this->real *= 10;
			else              this->real /= 10;
		}
	}
};

kScienceMathNSFooter()