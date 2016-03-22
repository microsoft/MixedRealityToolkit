/* (C) 2013-2015, The Regents of The University of Michigan
All rights reserved.

This software may be available under alternative licensing
terms. Contact Edwin Olson, ebolson@umich.edu, for more information.

   Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
 */

#include "time_util.h"

int64_t utime_now() // blacklist-ignore
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#ifdef _MSC_VER
	LARGE_INTEGER perfFreq;
	LARGE_INTEGER perfCount;
	QueryPerformanceFrequency(&perfFreq);
	QueryPerformanceCounter(&perfCount);

	LARGE_INTEGER frequency;
	LARGE_INTEGER time;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&time);

	return (int64_t)((double)time.QuadPart / (double)frequency.QuadPart * 1000000);
#else
    struct timeval tv;
    gettimeofday (&tv, NULL); // blacklist-ignore
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
#endif
	/// MICROSOFT PROJECT B CHANGES END
}

int64_t utime_get_seconds(int64_t v)
{
    return v/1000000;
}

int64_t utime_get_useconds(int64_t v)
{
    return v%1000000;
}

/// MICROSOFT PROJECT B CHANGES BEGIN
//void utime_to_timeval(int64_t v, struct timeval *tv)
//{
//    tv->tv_sec  = (time_t) utime_get_seconds(v);
//    tv->tv_usec = (suseconds_t) utime_get_useconds(v);
//}
//
//void utime_to_timespec(int64_t v, struct timespec *ts)
//{
//    ts->tv_sec  = (time_t) utime_get_seconds(v);
//    ts->tv_nsec = (suseconds_t) utime_get_useconds(v)*1000;
//}
//
//int32_t timeutil_usleep(int64_t useconds)
//{
//    // unistd.h function
//    return usleep(useconds);
//}
//
//uint32_t timeutil_sleep(unsigned int seconds)
//{
//    // unistd.h function
//    return sleep(seconds);
//}
/// MICROSOFT PROJECT B CHANGES END