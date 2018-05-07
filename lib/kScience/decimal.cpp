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

#include "decimal.h"

using namespace kScienceMathNS;

kScienceMathNSHeader()

//typedef unsigned int base_t;
//typedef long long wide_t;
//
//const base_t base = 1000000000; // 0 - 999 999 999
//const size_t base_digits = 9;   // 999 999 999
//
///**
// * n = sign * <data> * (base ^ exponent)
// */
//struct decimal_raw
//{
//
//	base_t *data;
//	size_t size;
//	size_t offset;
//
//	int sign;
//	int exponent;
//
//	decimal_raw();
//	decimal_raw(long long);
//	decimal_raw(const char *);
//	decimal_raw(const decimal_raw &);
//	~decimal_raw();
//};
//
//template <class T> size_t digits_of(T n)
//{
//	size_t digits = 0;
//	while(n != 0)
//	{
//		++digits;
//		n /= 10;
//	}
//	return digits;
//}
//
//inline base_t digit_at(const decimal_raw *n, unsigned int pos, int shift)
//{
//	if (static_cast<int>(pos)-shift < 0
//		|| static_cast<int>(pos)-shift >= static_cast<int>(n->size)) return 0;
//	return n->data[n->offset + pos - shift];
//}
//
//void normalize(decimal_raw *n)
//{
//	while(n->size > 0 && n->data[n->size - 1 + n->offset] == 0)
//	{
//		--(n->size);
//	}
//	while (n->size > 0 && n->data[n->offset] == 0)
//	{
//		--(n->size);
//		++(n->offset);
//		++(n->exponent);
//	}
//	if (n->size == 0) n->sign = 0;
//}
//
//std::ostream & operator << (std::ostream &str, const decimal_raw &n)
//{
//	if (n.sign == 0) return str << '0';
//	if (n.sign < 0) str << '-';
//	base_t digit;
//	int exponent = 0;
//	for (unsigned int i = n.size; i-- > 0;)
//	{
//		digit = digit_at(&n, i, 0);
//		if (i != n.size - 1)
//		{
//			size_t diff = base_digits - digits_of(digit);
//			for (unsigned int j = 0; j < diff; j++)
//			{
//				str << '0';
//			}
//		}
//		if (i == 0)
//		{
//			while (digit % 10 == 0)
//			{
//				++exponent;
//				digit /= 10;
//			}
//		}
//		str << digit;
//	}
//	if (n.exponent != 0 || exponent != 0)
//	{
//		str << 'e' << n.exponent * static_cast<int>(base_digits) + exponent;
//	}
//	return str;
//}
//
//double to_double(const decimal_raw *n)
//{
//	double result = 0;
//	for (unsigned int i = 0; i < n->size; i++)
//	{
//		result += digit_at(n, i, 0);
//		result *= base;
//	}
//	for (int i = 0; i < n->exponent; i++)
//	{
//		result *= base;
//	}
//	for (int i = -n->exponent; i > 0; i++)
//	{
//		result /= base;
//	}
//	result *= n->sign;
//	return result;
//}
//
//decimal_raw * mult(const decimal_raw *first, const decimal_raw *second)
//{
//	decimal_raw __deleteMe(decimal) *result = new decimal_raw;
//
//	result->size = first->size + second->size + 1;
//	result->data = new base_t[result->size](); // filling with zeros
//	
//	result->exponent = first->exponent + second->exponent;
//	result->sign = first->sign * second->sign;
//
//	long long element, element2;
//	for (unsigned int i = 0; i < second->size; i++)
//	{
//		for (unsigned int j = 0; j < first->size; j++)
//		{
//			element = static_cast<long long>(digit_at(second, i, 0)) * digit_at(first, j, 0);
//			element2 = static_cast<long long>(result->data[i + j]) + element % base;
//			result->data[i + j] = element2 % base;
//			result->data[i + j + 1] +=
//				static_cast<unsigned int>(element2 / base + element / base);
//		}
//	}
//
//	normalize(result);
//
//	return result;
//}
//
//decimal_raw * add(const decimal_raw *first, const decimal_raw *second)
//{
//	const decimal_raw *lower = (first->exponent < second->exponent) ? first : second;
//	const decimal_raw *higher = (lower == first) ? second : first;
//
//	int exponent_diff = higher->exponent - lower->exponent;
//
//	decimal_raw __deleteMe(decimal) *result = new decimal_raw;
//
//	result->size = max_of(lower->size, higher->size + exponent_diff) + 1;
//	result->data = new unsigned int[result->size](); // filling with zeros
//
//	result->exponent = lower->exponent;
//
//	int element = 0, element2 = 0;
//	for (unsigned int i = 0; i < result->size - 1; i++)
//	{
//		base_t el1 = digit_at(higher, i, exponent_diff);
//		base_t el2 = digit_at(lower, i, 0);
//		element = (higher->sign * static_cast<int>(digit_at(higher, i, exponent_diff))) +
//			lower->sign * static_cast<int>(digit_at(lower, i, 0));
//		result->data[i] += static_cast<unsigned int>(abs(element)) % base;
//		element2 = (element < 0) ? -1 : (element >= base) ? 1 : 0;
//	}
//
//	result->sign = (element2 < 0) ? -1 : 1;
//
//	normalize(result);
//
//	return result;
//}
//
//decimal_raw * negate(const decimal_raw *n)
//{
//	decimal_raw __deleteMe(decimal) *result = new decimal_raw(*n);
//	result->sign *= -1;
//	return result;
//}
//
//kScienceMathNSFooter()
//
//decimal_raw::decimal_raw() : size(0), exponent(0), data(nullptr), offset(0), sign(0)
//{
//}
//
//decimal_raw::decimal_raw(long long n) : decimal_raw()
//{
//	this->sign = (n > 0 ? 1 : n < 0 ? -1 : 0);
//	n = abs(n);
//	this->size = digits_of(n) / base_digits + 1;
//	this->data = new base_t[this->size];
//	for (unsigned int i = 0; i < this->size;  i++)
//	{
//		this->data[i] = n % base;
//		n /= base;
//	}
//	normalize(this);
//}
//
//decimal_raw * from_double(double n)
//{
//	int sign = (n > 0 ? 1 : n < 0 ? -1 : 0);
//	n = abs(n);
//	int order = static_cast<int>(floor(log10(n))) - 15;
//	int exponent = order / static_cast<int>(base_digits);
//	int diff = order % static_cast<int>(base_digits);
//	if (diff < 0)
//	{
//		diff += static_cast<int>(base_digits);
//		exponent -= 1;
//	}
//	double mantissa = n * pow(10, -order);
//	unsigned long long long_n = static_cast<unsigned long long>(mantissa);
//	decimal_raw *tmp = new decimal_raw(long_n);
//	tmp->exponent += exponent;
//	tmp->sign = sign;
//	long long diff_base_pow = 1;
//	for (int i = 0; i < diff; i++) diff_base_pow *= 10;
//	decimal_raw *tmp2 = new decimal_raw(diff_base_pow);
//	decimal_raw __deleteMe(decimal) *result = mult(tmp, tmp2);
//	delete tmp;
//	delete tmp2;
//	normalize(result);
//	return result;
//}
//
//decimal_raw::decimal_raw(const char *n) : decimal_raw()
//{
//	size_t str_size = strlen(n);
//	if (str_size == 0) return;
//	this->sign = 1;
//	if (n[0] == '-')
//	{
//		this->sign = -1;
//	}
//	this->size = str_size / base_digits + 1;
//	this->data = new base_t[this->size]();
//	base_t digit = 0;
//	int base_power = 1;
//	for (unsigned int i = (n[0] == '-') ? 1 : 0; i < str_size; i++) // TODO.
//	{
//		if (i % base_digits == 0) base_power = 1;
//		digit = n[str_size - i - 1] - '0';
//		this->data[i / base_digits] += digit * base_power;
//		base_power *= 10;
//	}
//	normalize(this);
//}
//
//decimal_raw::decimal_raw(const decimal_raw &other) : decimal_raw()
//{
//	this->data = new unsigned int[other.size];
//	for (unsigned int i = 0; i < other.size; i++)
//	{
//		this->data[i] = digit_at(&other, i, 0);
//	}
//	this->size = other.size;
//	this->sign = other.sign;
//	this->exponent = other.exponent;
//	normalize(this);
//}
//
//decimal_raw::~decimal_raw()
//{
//	delete[] this->data;
//}
//
//
//
//// ================= API ========================
//
//decimal::decimal(const decimal &other)
//{
//	this->raw = new decimal_raw(*other.raw);
//}
//
//decimal::decimal(long long n)
//{
//	this->raw = new decimal_raw(n);
//}
//
//decimal::decimal(double n)
//{
//	this->raw = from_double(n);
//}
//
//decimal::decimal(const char *n)
//{
//	this->raw = new decimal_raw(n);
//}
//
//decimal decimal::operator * (const decimal &other) const
//{
//	return decimal(mult(this->raw, other.raw));
//}
//
//decimal decimal::operator + (const decimal &other) const
//{
//	return decimal(add(this->raw, other.raw));
//}
//
//decimal decimal::operator - (const decimal &other) const
//{
//	decimal_raw *neg_other = negate(other.raw);
//	decimal_raw *result = add(this->raw, neg_other);
//	delete neg_other;
//	return decimal(result);
//}
//
//bool decimal::operator > (const decimal &other) const
//{
//	return (*this - other).raw->sign > 0;
//}
//
//bool decimal::operator >= (const decimal &other) const
//{
//	return (*this - other).raw->sign >= 0;
//}
//
//bool decimal::operator < (const decimal &other) const
//{
//	return (*this - other).raw->sign < 0;
//}
//
//bool decimal::operator <= (const decimal &other) const
//{
//	return (*this - other).raw->sign <= 0;
//}
//
//bool decimal::operator == (const decimal &other) const
//{
//	return (*this - other).raw->sign == 0;
//}
//
//decimal & decimal::operator = (const decimal &other)
//{
//	delete this->raw;
//	this->raw = new decimal_raw(*other.raw);
//	return *this;
//}
//
//decimal::~decimal()
//{
//	delete raw;
//}
//
//kScienceMathNSHeader()
//
//std::ostream & operator << (std::ostream &str, const decimal &n)
//{
//	return str << *n.raw;
//}



/************************************************************************/
/*                     Constants                                        */
/************************************************************************/



const base_t max_base_t = UINT_LEAST32_MAX;



/************************************************************************/
/*                     Radix                                            */
/************************************************************************/



const char *default_alphabet = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const radix radix2(2), radix8(8), radix16(16), radix10(10);



radix::radix(base_t rad, const char *alphabet /* = default_alphabet */) :
alphabet(alphabet), alphabet_size(strlen(alphabet)), size(digits_of(max_base_t, rad) - 1), radices(new base_t[this->size + 1])
{
	radices[0] = 1;
	for (size_t i = 1; i <= this->size; i++)
	{
		radices[i] = radices[i - 1] * rad;
	}
}

inline void radix::print(std::ostream &out, const base_t &digit) const
{
	if (digit < alphabet_size)
	{
		out << this->alphabet[digit];
	}
	else
	{
		out << '[' << digit << ']';
	}
}

radix::~radix()
{
	delete this->radices;
}



/************************************************************************/
/*                     Number Structure                                 */
/************************************************************************/



num_struct::num_struct() : size(0), exponent(0), data(nullptr), offset(0), sign(0)
{
}

num_struct::num_struct(const num_struct &other) : num_struct()
{
	this->data = new unsigned int[other.size];
	for (unsigned int i = 0; i < other.size; i++)
	{
		this->data[i] = other.data[i + other.offset];
	}
	this->size = other.size;
	this->sign = other.sign;
	this->exponent = other.exponent;
	this->normalize();
}

num_struct::~num_struct()
{
	delete this->data;
}

void num_struct::normalize()
{
	while (this->size > 0 && this->data[this->size - 1 + this->offset] == 0)
	{
		--(this->size);
	}
	while (this->size > 0 && this->data[this->offset] == 0)
	{
		--(this->size);
		++(this->offset);
		++(this->exponent);
	}
	if (this->size == 0) this->sign = 0;
}

inline size_t num_struct::get_size() const
{
	return this->size;
}

inline base_t & num_struct::operator [] (size_t pos)
{
	return this->data[pos + this->offset];
}

inline const base_t & num_struct::operator [] (size_t pos) const
{
	return this->data[pos + this->offset];
}

void num_struct::resize(size_t size)
{
	if (this->size >= size) return;
	base_t *new_data = new base_t[size]();
	if (this->data != nullptr)
	{
		for (size_t i = 0; i < this->size; i++)
		{
			new_data[i] = this->data[i];
		}
	}
	this->size = size;
	delete this->data;
	this->data = new_data;
}



/************************************************************************/
/*                     Number Structure Operations                      */
/************************************************************************/




int compare(const num_struct &a, const num_struct &b)
{
	if (a.sign < b.sign)
	{
		return -1;
	}
	if (a.sign > b.sign)
	{
		return 1;
	}
	if (a.exponent + static_cast<int>(a.get_size()) < b.exponent + static_cast<int>(b.get_size()))
	{
		return -a.sign;
	}
	if (a.exponent + static_cast<int>(a.get_size()) > b.exponent + static_cast<int>(b.get_size()))
	{
		return a.sign;
	}

	const num_struct &c = a.exponent >= b.exponent ? a : b, &d = (&c == &a) ? b : a;

	size_t exponent_diff = c.exponent - d.exponent;

	size_t max_size = max_of(a.get_size(), b.get_size() + exponent_diff);

	base_t digit_a, digit_b;
	for (size_t i = max_size; i-- > 0;)
	{
		digit_a = (i < exponent_diff || i >= exponent_diff + a.get_size()) ? 0 : a[i - exponent_diff];
		digit_b = (i >= b.get_size()) ? 0 : b[i];
		if (digit_a < digit_b) return -a.sign;
		if (digit_a > digit_b) return a.sign;
	}

	return 0;
}

num_struct * mult(const num_struct &a, const num_struct &b, const radix *ns)
{
	num_struct __deleteMe(*) *c = new num_struct;

	c->resize(a.get_size() + b.get_size() + 1);

	c->exponent = a.exponent + b.exponent;
	c->sign = a.sign * b.sign;

	for (size_t i = 0; i < b.get_size(); i++)
	{
		for (size_t j = 0; j < a.get_size(); j++)
		{
			base_mult(a[j], b[i], (*c)[i + j], (*c)[i + j + 1], ns->get_base());
		}
	}

	c->normalize();

	return c;
}

num_struct * add_sub(const num_struct &a, const num_struct &b, bool sub, const radix *ns)
{
	sub ^= (a.sign != b.sign);

	num_struct __deleteMe(decimal) *c = new num_struct;

	if (a.sign == b.sign) return c;

	const num_struct &d = (a.exponent >= b.exponent) ? a : b, e = (&d == &a) ? b : a;

	size_t exponent_diff = d.exponent - e.exponent;

	if (!sub)
	{
		c->resize(max_of(d.get_size(), e.get_size() + exponent_diff) + 1);

		base_t digit_d, digit_e;
		for (size_t i = 0; i < c->get_size() - 1; i++)
		{
			digit_d = (i < exponent_diff || i >= exponent_diff + d.get_size()) ? 0 : d[i - exponent_diff];
			digit_e = (i >= e.get_size()) ? 0 : e[i];
			base_add(digit_d, digit_e, (*c)[i], (*c)[i + 1], ns->get_base());
		}

		c->sign = a.sign;
	}
	else
	{
		int cmp = compare(a, b);

		if (cmp == 0)
		{
			return c;
		}

		c->resize(max_of(d.get_size(), e.get_size() + exponent_diff));

		base_t digit_d, digit_e, r;
		for (size_t i = 0; i < c->get_size() - 1; i++)
		{
			digit_d = (i < exponent_diff || i >= exponent_diff + d.get_size()) ? 0 : d[i - exponent_diff];
			digit_e = (i >= e.get_size()) ? 0 : e[i];
			base_sub(digit_d, digit_e, (*c)[i], (*c)[i + 1], ns->get_base());
		}

		c->sign = (cmp > 0) ? a.sign : b.sign;
	}

	return c;
}



/************************************************************************/
/*                     Decimal                                          */
/************************************************************************/



decimal::decimal(const radix *ns /* = &radix10 */) : raw(new num_struct), ns(ns)
{
}

decimal::decimal(wraw_t n, const radix *ns /* = &radix10 */) : decimal(ns)
{
	wraw_t m = abs(n);
	this->raw->sign = (n > 0 ? 1 : n < 0 ? -1 : 0);
	this->raw->resize(digits_of(m, ns->get_radix()) / ns->digits() + 1);
	for (size_t i = 0; i < this->raw->get_size(); i++)
	{
		(*this->raw)[i] = m % ns->get_base();
		m /= ns->get_base();
	}
	this->raw->normalize();
}

decimal::~decimal()
{
	delete this->raw;
}

bool decimal::operator > (const decimal &other) const
{
	return compare(*this->raw, *other.raw) > 0;
}
bool decimal::operator < (const decimal &other) const
{
	return compare(*this->raw, *other.raw) < 0;
}
bool decimal::operator == (const decimal &other) const
{
	return compare(*this->raw, *other.raw) == 0;
}
bool decimal::operator >= (const decimal &other) const
{
	return compare(*this->raw, *other.raw) >= 0;
}
bool decimal::operator <= (const decimal &other) const
{
	return compare(*this->raw, *other.raw) <= 0;
}
int decimal::compare_to(const decimal &other) const
{
	return compare(*this->raw, *other.raw);
}

decimal decimal::operator * (const decimal &other) const
{
	return decimal(mult(*this->raw, *other.raw, this->ns), this->ns);
}

decimal decimal::operator + (const decimal &other) const
{
	return decimal(add_sub(*this->raw, *other.raw, false, this->ns), this->ns);
}

decimal decimal::operator - (const decimal &other) const
{
	return decimal(add_sub(*this->raw, *other.raw, true, this->ns), this->ns);
}

// TODO: flags, decimal point

std::ostream & operator << (std::ostream &str, const decimal &m)
{
	num_struct &n = *m.raw;

	if (n.sign == 0) return str << '0';

	if (n.sign < 0) str << '-';

	base_t digit;
	base_t raw_digit;
	size_t exponent = 0;
	bool nil = true;
	for (size_t i = n.get_size(); i-- > 0;)
	{
		digit = n[i];
		if (i == 0)
		{
			while (digit % m.ns->get_radix() == 0)
			{
				++exponent;
				digit /= m.ns->get_radix();
			}
		}
		for (size_t j = m.ns->digits() - exponent; j-- > 0;)
		{
			raw_digit = (digit / (*m.ns)[j]) % m.ns->get_radix();
			if (nil && digit < (*m.ns)[j]) continue;
			m.ns->print(str, raw_digit);
		}
		nil &= (digit == 0);
	}

	if (n.exponent != 0 || exponent != 0)
	{
		str << 'e' << n.exponent * static_cast<int>(m.ns->digits()) + exponent;
	}

	return str;
}

kScienceMathNSFooter()