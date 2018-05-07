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
#include <cinttypes>

kScienceMathNSHeader()

/************************************************************************/
/*                     Basic Definitions                                */
/************************************************************************/

typedef int_least32_t raw_t;
typedef int_least64_t wraw_t;
typedef uint_least32_t base_t;

extern const base_t max_base_t;
extern const char *default_alphabet;

class num_struct
{

private:

	base_t *data;
    size_t size;
	size_t offset;

public:

	int sign;
	int exponent;

public:

	num_struct();
	num_struct(const num_struct &other);
	virtual ~num_struct();

public:

	inline size_t get_size() const;
	//inline base_t get(size_t pos) const;
	//inline void   set(size_t pos, base_t n);
	inline base_t & operator [] (size_t pos);
	inline const base_t & operator [] (size_t pos) const;

public:

	void normalize();

public:

	__deleteMe("somewhere") base_t * get_data() const;
	void resize(size_t size);
};

class radix
{

private:

	const char *alphabet;
	size_t alphabet_size;

private:

	size_t size;
	base_t *radices;

public:

	radix(base_t rad, const char *alphabet = default_alphabet);
	virtual ~radix();

public:

	inline base_t get_radix() const
	{
		return radices[1];
	}

	inline base_t get_base() const
	{
		return radices[this->size];
	}

	inline base_t digits() const
	{
		return size;
	}

	inline base_t operator [] (size_t idx) const
	{
		return radices[idx];
	}

	virtual inline void print(std::ostream &out, const base_t &digit) const;

};

extern const radix radix2, radix8, radix16, radix10;

/************************************************************************/
/*                     Raw Type Operations                              */
/************************************************************************/

inline void base_mult(const base_t &a, const base_t &b, base_t &res, base_t &rem, const base_t &modulo)
{
	wraw_t cc = static_cast<wraw_t>(a) * static_cast<wraw_t>(b);
	wraw_t dd = res + cc % static_cast<wraw_t>(modulo);
	res = static_cast<base_t>(dd % static_cast<wraw_t>(modulo));
	rem += static_cast<base_t>(dd / static_cast<wraw_t>(modulo) + cc / static_cast<wraw_t>(modulo));
}
inline base_t base_div (base_t a, base_t b);
inline void base_add(const base_t &a, const base_t &b, base_t &res, base_t &r, const base_t &modulo)
{
	wraw_t result = static_cast<wraw_t>(a) + static_cast<wraw_t>(b) + static_cast<wraw_t>(res);
	r = static_cast<raw_t>(result / modulo);
	res = static_cast<base_t>(result % modulo);
}
inline void base_sub(const base_t &a, const base_t &b, base_t &res, base_t &r, const base_t &modulo)
{
	if (a < b)
	{
		res = modulo - a + b;
		r = 1;
	}
	else
	{
		res = a - b;
		r = 0;
	}
}
// TODO: base_sub through wraw_t
// TODO: subtraction -> add different comparison (mantissa only)

template <class T>
inline size_t digits_of(T n, base_t modulo = 10)
{
	size_t digits = 0;
	while (n != 0)
	{
		++digits;
		n /= modulo;
	}
	return digits;
}

/************************************************************************/
/*                     Number Structure Operations                      */
/************************************************************************/

int          compare(const num_struct &a, const num_struct &b);
num_struct * mult   (const num_struct &a, const num_struct &b,           const radix *ns);
num_struct * add_sub(const num_struct &a, const num_struct &b, bool sub, const radix *ns);

/************************************************************************/
/*                     User-Friendly API                                */
/************************************************************************/

//template <numeration_system *base>
class decimal
{

private:

	num_struct *raw;
	const radix *ns;

	// Constructors
	decimal(num_struct *raw, const radix *ns = &radix10) : raw(raw), ns(ns) {} // From raw

public:

	// Constructors & destructors
	decimal(const radix *ns = &radix10);
	decimal(const decimal &);
	decimal(wraw_t, const radix *ns = &radix10);
	decimal(double, const radix *ns = &radix10);
	decimal(const char *, const radix *ns = &radix10);
	virtual ~decimal();

	// Basic math operations
	decimal operator + (const decimal &) const;
	decimal operator - (const decimal &) const;
	decimal operator * (const decimal &) const;
	decimal operator / (const decimal &) const;
	decimal operator % (const decimal &) const;

	// Basic comparison
	bool operator > (const decimal &) const;
	bool operator < (const decimal &) const;
	bool operator >= (const decimal &) const;
	bool operator <= (const decimal &) const;
	bool operator == (const decimal &) const;
	int compare_to(const decimal &) const;

	// Assignment
	decimal& operator = (const decimal &);

	// To double conversion

	friend std::ostream & operator << (std::ostream &, const decimal &);
};

kScienceMathNSFooter()