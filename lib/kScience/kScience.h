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

#include <cmath>
#include <limits>

/*

Define namespaces

*/

#define kDesktopNS org::kalaider::desktop
#define kDesktopNSHeader() namespace org { namespace kalaider { namespace desktop {
#define kDesktopNSFooter() }}}

#define kScienceNS kDesktopNS::science
#define kScienceNSHeader() kDesktopNSHeader() namespace science {
#define kScienceNSFooter() kDesktopNSFooter() }





// Meta tags describing a pointer that is is not a locally created one (i.e. external).
// __extptrd allows to add some description.
#define __extptr
#define __extptrd(a)

// Meta tag describing a variable that is to be deleted but it is
// much simpler not to do that (e.g. global pointer initialized only once).
#define __ignoreDeleteMe(a)

// Meta tag describing a variable that is to be deleted later in the code.
// The parameter specifies that place in human-readable form.
// 
// Usage:
// 
// __deleteMe(in the 'release' method) int *i = new int(23);
// /* <...> */
// release(i);
// 
// or
// 
// int *i = __deleteMe(this) new int(23);
// /* <...> */
// delete i;
// 
// or
// 
// void someFunction()
// {
//     int *i = __deleteMe(doSomeWork) new int(23);
//     /* <...> */
//     doSomeWork(i);
// }
// 
// /* <...> */
// void doSomeWork(void /* either 1. __deleteMe() */ *p)
// {
//     int *i = /* or 2. __deleteMe(this) / __extptrd(someFunction) __deleteMe(this) */ reinterpret_cast<int *>(p);
//     /* <...> */
//     delete /* or/and, maybe, 3. __deleteme() */ i;
// }
//
#define __deleteMe(a)




// Meta tag marks the method parameter used as an output parameter.
#define __outp



/*

Let other modules register this block of functions first

*/

#ifndef kDesktopMinOfMaxOf
#define kDesktopMinOfMaxOf

kDesktopNSHeader()

/*

Replacement for min and max macro as they sometimes make conflicts

*/

template<class T> inline T min_of(const T first, const T second)
{
	return first < second ? first : second;
}

template<class T> inline T max_of(const T first, const T second)
{
	return first > second ? first : second;
}

kDesktopNSFooter()

#endif /* #ifndef kDesktopMinOfMaxOf */





/*

Define some constants

*/

kScienceNSHeader()

#ifdef max
#define ___max_was_defined
#undef max
#endif // max
#ifdef min
#define ___min_was_defined
#undef min
#endif // min

const double double_error = 1e-15;

template<class T> inline const T max_val()
{
	return std::numeric_limits<T>::max();
}

template<class T> inline const T min_val()
{
	return std::numeric_limits<T>::max();
}

#ifdef ___max_was_defined
#define max(a,b) ((a)>(b)?(a):(b))
#endif // ___max_was_defined
#ifdef ___min_was_defined
#define min(a,b) ((a)<(b)?(a):(b))
#endif // ___min_was_defined


kScienceNSFooter()