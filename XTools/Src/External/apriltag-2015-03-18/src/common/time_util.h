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

#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H

#include <stdint.h>
/// MICROSOFT PROJECT B CHANGES BEGIN
#ifdef _MSC_VER
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <time.h>
/// MICROSOFT PROJECT B CHANGES END

#ifdef __cplusplus
extern "C" {
#endif

int64_t utime_now(); // blacklist-ignore
int64_t utime_get_seconds(int64_t v);
int64_t utime_get_useconds(int64_t v);
void    utime_to_timeval(int64_t v, struct timeval *tv);
void    utime_to_timespec(int64_t v, struct timespec *ts);

int32_t  timeutil_usleep(int64_t useconds);
uint32_t timeutil_sleep(unsigned int seconds);


#ifdef __cplusplus
}
#endif

#endif
