///*
//Copyright(C) 2015 Alexander Vasilevsky aka kalaider
//
//This program is free software : you can redistribute it and / or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.If not, see <http://www.gnu.org/licenses/>.
//*/
//
//#pragma once
//
//#define kScienceApNS kScienceNS::ap
//#define kScienceApNSHeader() kScienceNSHeader() namespace ap {
//#define kScienceApNSFooter() kScienceNSFooter() }
//
//#include "kScience.h"
//#include "apfloat/global.h"
//#include "apfloat/apfloat.h"
//#include "apfloat/apint.h"
//#include "apfloat/modint.h"
//#include "apfloat/raw.h"
//#include <sstream>
//
//kScienceApNSHeader()
//
//enum class RoundingMode
//{
//	UP, DOWN, CEILING, FLOOR
//};
//
//// Returns pos'th block of data of passed apfloat's apstruct
//inline rawtype get(const apfloat &num, int pos = -1)
//{
//	rawtype mantissa;
//
//	if (!num.ap->size) return 0;
//
//	if (pos < 0) pos = num.ap->size - 1;
//
//	modint *data = num.ap->getdata(pos, 1);
//	mantissa = (rawtype)data[0];
//	num.ap->cleardata();
//
//	return mantissa;
//}
//
//// Normalized last block of data of num's apstruct (without trailing zeros)
//inline rawtype getnl(const apfloat &num)
//{
//	rawtype mantissa = get(num);
//	if (mantissa == 0) return 0;
//	while (mantissa % 10 == 0) mantissa /= 10;
//	return mantissa;
//}
//
//// Additional number of trailing zeros in mantissa
//inline int exp10a(const apfloat &num)
//{
//	if (!num.ap->size) return 0;
//
//	rawtype mantissa = get(num);
//
//	if (mantissa == 0) return 0;
//
//	int exp = 0;
//	while (mantissa % 10 == 0)
//	{
//		++exp;
//		mantissa /= 10;
//	}
//	return exp;
//}
//
//// 10^exp10a
//inline int pow10a(const apfloat &num)
//{
//	if (!num.ap->size) return 1;
//
//	rawtype mantissa = get(num);
//
//	if (mantissa == 0) return 1;
//
//	int exp = 1;
//	while (mantissa % 10 == 0)
//	{
//		exp *= 10;
//		mantissa /= 10;
//	}
//	return exp;
//}
//
//// Returns a real exponent of mantissa
//inline long long exp10(const apfloat &num)
//{
//	return (num.ap->exp - 1) * Basedigits + exp10a(num);
//}
//
//inline rawtype toRawtype(const apfloat &num)
//{
//	// An integral
//	assert(num.ap->size <= 1);
//	return get(num);
//}
//
//inline double toDouble(const apfloat &num)
//{
//	return ap2double(num.ap);
//}
//
//inline apfloat round(const apfloat &num, RoundingMode roundingMode, const apfloat &interval = 1)
//{
//	apfloat quantity = num / interval;
//
//	switch (roundingMode)
//	{
//	case RoundingMode::CEILING:
//		quantity = ceil(quantity);
//		break;
//	case RoundingMode::FLOOR:
//		quantity = floor(quantity);
//		break;
//	case RoundingMode::UP:
//		if (quantity > 0) quantity = ceil(quantity);
//		else quantity = floor(quantity);
//		break;
//	case RoundingMode::DOWN:
//		if (quantity < 0) quantity = ceil(quantity);
//		else quantity = floor(quantity);
//		break;
//	}
//
//	return interval * quantity;
//}
//
//kScienceApNSFooter()