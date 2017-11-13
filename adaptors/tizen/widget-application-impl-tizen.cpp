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

// CLASS HEADER
#include "widget-application-impl.h"

// INTERNAL INCLUDE
#include <widget.h>
#include <widget-impl.h>
#include <widget-controller.h>

namespace Dali
{

namespace Internal
{

namespace
{

int OnInstanceInit(widget_base_instance_h instanceHandle, bundle *content, int w, int h, void *classData)
{
  char *id;
  widget_base_context_get_id(instanceHandle, &id);

  widget_base_class_on_create(instanceHandle, content, w, h);

  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // After DALi can support multi window, this part should be changed.
  Dali::Window window = application->GetWindow();
  window.ShowIndicator(Dali::Window::INVISIBLE);
  Any nativeHandle = window.GetNativeHandle();
  Ecore_Wl_Window * wlWindow = AnyCast<Ecore_Wl_Window*>( nativeHandle );
  widget_base_context_window_bind( instanceHandle, id, wlWindow );
  window.SetSize( Dali::Window::WindowSize( w, h ) );

  Dali::Internal::Adaptor::WidgetApplication::CreateWidgetFunctionPair pair = application->GetWidgetCreatingFunctionPair(std::string(id));
  Dali::WidgetApplication::CreateWidgetFunction createFunction = pair.second;

  Dali::Widget widgetInstance = createFunction( pair.first );
  application->AddWidget( instanceHandle, widgetInstance );

  Dali::Internal::Adaptor::Widget::Impl *widgetImpl = new Dali::Internal::Adaptor::Widget::Impl(instanceHandle);
  Internal::Adaptor::GetImplementation(widgetInstance).SetImpl( widgetImpl );

  std::string encodedContentString = "";

  if( bundle_get_count( content ) )
  {
    bundle_raw *bundleRaw;
    int len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast< char* >( bundleRaw );
    encodedContentString = std::string( encodedContent );
    free(bundleRaw);
  }

  Internal::Adaptor::GetImplementation(widgetInstance).OnCreate( encodedContentString, window );

  return 0;
}

int OnInstanceDestroy(widget_base_instance_h instanceHandle, widget_base_destroy_type_e reason, bundle *content, void *classData)
{
  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget( instanceHandle );

  Dali::Widget::Termination destroyReason = Dali::Widget::Termination::TEMPORARY;

  if(reason == WIDGET_BASE_DESTROY_TYPE_PERMANENT)
  {
    destroyReason = Dali::Widget::Termination::PERMANENT;
  }

  std::string encodedContentString = "";

  if( bundle_get_count( content ) )
  {
    bundle_raw *bundleRaw;
    int len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast< char* >( bundleRaw );
    encodedContentString = std::string(encodedContent);
    free(bundleRaw);
  }

  Internal::Adaptor::GetImplementation(widgetInstance).OnTerminate( encodedContentString, destroyReason );

  widget_base_class_on_destroy(instanceHandle, reason, content);

  application->DeleteWidget( instanceHandle );

  return 0;
}

int OnInstancePause(widget_base_instance_h instanceHandle, void *classData)
{
  widget_base_class_on_pause(instanceHandle);

  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget( instanceHandle );

  Internal::Adaptor::GetImplementation(widgetInstance).OnPause();

  return 0;
}

int OnInstanceResume(widget_base_instance_h instanceHandle, void *classData)
{
  widget_base_class_on_resume(instanceHandle);

  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget( instanceHandle );

  Internal::Adaptor::GetImplementation(widgetInstance).OnResume();

  return 0;
}

int OnInstanceResize(widget_base_instance_h instanceHandle, int w, int h, void *classData)
{
  widget_base_class_on_resize(instanceHandle, w, h);

  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget( instanceHandle );

  Dali::Window window = application->GetWindow();
  window.SetSize( Dali::Window::WindowSize(w, h) );
  Internal::Adaptor::GetImplementation(widgetInstance).OnResize(window);

  return 0;
}

int OnInstanceUpdate(widget_base_instance_h instanceHandle, bundle *content, int force, void *classData)
{
  widget_base_class_on_update(instanceHandle, content, force);

  Dali::Internal::Adaptor::WidgetApplication* application = static_cast<Dali::Internal::Adaptor::WidgetApplication*>(classData);

  // Get Dali::Widget instance.
  Dali::Widget widgetInstance = application->GetWidget( instanceHandle );

  std::string encodedContentString = "";

  if( bundle_get_count( content ) )
  {
    bundle_raw *bundleRaw;
    int len;
    bundle_encode(content, &bundleRaw, &len);
    char* encodedContent = reinterpret_cast< char* >( bundleRaw );
    encodedContentString = std::string(encodedContent);
    free(bundleRaw);
  }

  Internal::Adaptor::GetImplementation(widgetInstance).OnUpdate( encodedContentString, force );

  return 0;
}

} // anonymous namespace

namespace Adaptor
{

WidgetApplicationPtr WidgetApplication::New(
  int* argc,
  char **argv[],
  const std::string& stylesheet)
{
  WidgetApplicationPtr widgetApplication( new WidgetApplication (argc, argv, stylesheet ) );
  return widgetApplication;
}

WidgetApplication::WidgetApplication( int* argc, char** argv[], const std::string& stylesheet )
:Application(argc, argv, stylesheet, Dali::WidgetApplication::OPAQUE, PositionSize(), Framework::WIDGET)
{
}

WidgetApplication::~WidgetApplication()
{
}


void WidgetApplication::RegisterWidgetCreatingFunction( const std::string& widgetName, Dali::WidgetApplication::CreateWidgetFunction createFunction )
{
  AddWidgetCreatingFunctionPair( CreateWidgetFunctionPair(widgetName, createFunction) );

  // Register widget class to widget framework
  widget_base_class cls = widget_base_class_get_default();
  cls.ops.create = OnInstanceInit;
  cls.ops.destroy = OnInstanceDestroy;
  cls.ops.pause = OnInstancePause;
  cls.ops.resume = OnInstanceResume;
  cls.ops.resize = OnInstanceResize;
  cls.ops.update = OnInstanceUpdate;

  widget_base_class_add(cls, widgetName.c_str(), this);
}

void WidgetApplication::AddWidgetCreatingFunctionPair( CreateWidgetFunctionPair pair )
{
  mCreateWidgetFunctionContainer.push_back( pair );
}

WidgetApplication::CreateWidgetFunctionPair WidgetApplication::GetWidgetCreatingFunctionPair( const std::string& widgetName )
{
  for( CreateWidgetFunctionContainer::const_iterator iter = mCreateWidgetFunctionContainer.begin(); iter != mCreateWidgetFunctionContainer.end(); ++iter )
  {
    if( widgetName.find((*iter).first) != std::string::npos  )
    {
      return *iter;
    }
  }

  return CreateWidgetFunctionPair( "", NULL );
}

void WidgetApplication::AddWidget( widget_base_instance_h widgetBaseInstance, Dali::Widget widget )
{
  mWidgetInstanceContainer.push_back( WidgetInstancePair(widgetBaseInstance, widget) );
}

Dali::Widget WidgetApplication::GetWidget( widget_base_instance_h widgetBaseInstance )
{
  for( WidgetInstanceContainer::const_iterator iter = mWidgetInstanceContainer.begin(); iter != mWidgetInstanceContainer.end(); ++iter )
  {
    if( (*iter).first == widgetBaseInstance  )
    {
      return (*iter).second;
    }
  }
  return Dali::Widget();
}

void WidgetApplication::DeleteWidget( widget_base_instance_h widgetBaseInstance )
{
  for( WidgetInstanceContainer::const_iterator iter = mWidgetInstanceContainer.begin(); iter != mWidgetInstanceContainer.end(); ++iter )
  {
    if( (*iter).first == widgetBaseInstance  )
    {
      mWidgetInstanceContainer.erase(iter);
      break;
    }
  }
}

} // namespace Adaptor

} // namespace Internal

} // namespace Dali
