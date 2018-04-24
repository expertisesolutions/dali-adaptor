#ifndef DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_FACTORY_H
#define DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_FACTORY_H

/*
 * Copyright (c) 2017 Samsung Electronics Co., Ltd.
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

#include <memory>
#include <dali/public-api/object/any.h>

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

class DisplayConnection;
class DisplayConnectionFactory
{
public:

  DisplayConnectionFactory() = default;
  virtual ~DisplayConnectionFactory() = default;

  virtual std::unique_ptr<Dali::Internal::Adaptor::DisplayConnection> CreateDisplayConnection() = 0;

};

extern std::unique_ptr<DisplayConnectionFactory> GetDisplayConnectionFactory();

extern void DisplayConnectionFactoryGetDpi(unsigned int& dpiHorizontal, unsigned int& dpiVertical);

extern void DisplayConnectionFactoryGetDpi(Any nativeWindow, unsigned int& dpiHorizontal, unsigned int& dpiVertical);

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_INTERNAL_WINDOWSYSTEM_COMMON_DISPLAY_CONNECTION_FACTORY_H
