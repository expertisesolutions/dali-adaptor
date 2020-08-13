/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// HEADER
#include <dali/internal/system/common/time-service.h>

// EXTERNAL INCLUDES
#include <ctime>

namespace Dali::Internal::Adaptor::TimeService
{

namespace
{
const uint64_t NANOSECONDS_PER_SECOND = 1e+9;
}

void GetNanoseconds( uint64_t& timeInNanoseconds )
{
  timespec timeSpec;
  if( clock_gettime( CLOCK_MONOTONIC, &timeSpec ) == 0 )
  {
    // Convert all values to uint64_t to match our return type
    timeInNanoseconds = ( static_cast< uint64_t >( timeSpec.tv_sec ) * NANOSECONDS_PER_SECOND ) + static_cast< uint64_t >( timeSpec.tv_nsec );
  }
  else
  {
    timeInNanoseconds = 0;
  }
}

void SleepUntil( uint64_t timeInNanoseconds )
{
  uint64_t curTimeInNanoSeconds;
  GetNanoseconds(curTimeInNanoSeconds);
  if (timeInNanoseconds <= curTimeInNanoSeconds)
  {
    return;
  }

  const auto timeToSleep = timeInNanoseconds - curTimeInNanoSeconds;

  timespec timeSpec;
  timeSpec.tv_sec  = timeToSleep / NANOSECONDS_PER_SECOND;
  timeSpec.tv_nsec = timeToSleep % NANOSECONDS_PER_SECOND;

  // nanosleep returns 0 if it sleeps for the period specified, otherwise it returns an error value
  // If an error value is returned, just sleep again till the absolute time specified
  while( nanosleep( &timeSpec, NULL ) )
  {
  }
}

} // namespace Dali::Internal::Adaptor::TimeService
