#ifndef DALI_WIDGET_CONTROLLER_H
#define DALI_WIDGET_CONTROLLER_H

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

// EXTERNAL INCLUDES
#include <dali/public-api/signals/connection-tracker.h>

// INTERNAL INCLUDES
#ifdef WIDGET_SUPPOERTED
#include <widget_base.h>
#endif
#include <widget-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{

/**
 * @brief Holds the Implementation for the internal WidgetImpl class
 */
class Widget::Impl : public ConnectionTracker
{
public:

#ifndef WIDGET_SUPPOERTED
  typedef void* widget_base_instance_h;
#endif

  /**
   * Constructor
   */
  Impl( widget_base_instance_h instanceHandle );

  /**
   * Destructor
   */
  ~Impl();

public:

  /**
   * Set content information to widget framework
   */
  void SetContentInfo( const std::string& contentInfo );

private:

  widget_base_instance_h mInstanceHandle;
};

} // namespace Adaptor

} // namespace Internal

} // namespace Dali

#endif // DALI_WIDGET_CONTROLLER_H
