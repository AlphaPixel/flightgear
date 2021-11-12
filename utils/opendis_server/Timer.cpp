
#include "Timer.h"
#include "Logging.h"

#include <iostream>
#include <sstream>
#include <sys/time.h>

using namespace Example;

const double MILLI = 0.001;
const double MICRO = 0.000001;

Timer::Timer() {
}

void Timer::Update()
{
   struct timeval te; 
   gettimeofday(&te, NULL); // get current time
   long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // calculate milliseconds
   _ticks = milliseconds;
}


double Timer::GetSeconds() const
{
   return (_ticks * MILLI);
}
