/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd.
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
#include <dali/internal/graphics/gles/egl-graphics.h>

// INTERNAL INCLUDES
#include <dali/internal/window-system/common/display-utils.h> // For Utils::MakeUnique

namespace Dali
{
namespace Internal
{
namespace Adaptor
{

EglGraphics::EglGraphics( )
: mMultiSamplingLevel( 0 )
{
}

EglGraphics::~EglGraphics()
{
}

void EglGraphics::SetGlesVersion( const int32_t glesVersion )
{
  mEglImplementation->SetGlesVersion( glesVersion );
  mGLES->SetGlesVersion( glesVersion );
}

void EglGraphics::SetIsSurfacelessContextSupported( const bool isSupported )
{
  mGLES->SetIsSurfacelessContextSupported( isSupported );
}

void EglGraphics::Initialize( EnvironmentOptions* environmentOptions )
{
  if( environmentOptions->GetGlesCallTime() > 0 )
  {
    mGLES = Utils::MakeUnique< GlProxyImplementation >( *environmentOptions );
  }
  else
  {
    mGLES.reset ( new GlImplementation() );
  }

  mDepthBufferRequired = static_cast< Integration::DepthBufferAvailable >( environmentOptions->DepthBufferRequired() );
  mStencilBufferRequired = static_cast< Integration::StencilBufferAvailable >( environmentOptions->StencilBufferRequired() );
  mPartialUpdateRequired = static_cast< Integration::PartialUpdateAvailable >( environmentOptions->PartialUpdateRequired() );

  mMultiSamplingLevel = environmentOptions->GetMultiSamplingLevel();

  mEglSync = Utils::MakeUnique< EglSyncImplementation >();

  mEglContextHelper = Utils::MakeUnique< EglContextHelperImplementation >();
}

void EglGraphics::Initialize( bool depth, bool stencil, int msaa )
{
  mDepthBufferRequired = static_cast< Integration::DepthBufferAvailable >( depth );
  mStencilBufferRequired = static_cast< Integration::StencilBufferAvailable >( stencil );

  mMultiSamplingLevel = msaa;

  mEglSync = std::unique_ptr<EglSyncImplementation>( new EglSyncImplementation() );

  mEglContextHelper = std::unique_ptr<EglContextHelperImplementation>( new EglContextHelperImplementation() );
}

EglInterface* EglGraphics::Create()
{
  mEglImplementation = Utils::MakeUnique< EglImplementation >( mMultiSamplingLevel, mDepthBufferRequired, mStencilBufferRequired, mPartialUpdateRequired );
  mEglImageExtensions = Utils::MakeUnique< EglImageExtensions >( mEglImplementation.get() );

  mEglSync->Initialize( mEglImplementation.get() ); // The sync impl needs the EglDisplay

  mEglContextHelper->Initialize( mEglImplementation.get() ); // The context helper impl needs the EglContext

  return mEglImplementation.get();
}

void EglGraphics::Destroy()
{
}

GlImplementation& EglGraphics::GetGlesInterface()
{
  return *mGLES;
}

Integration::GlAbstraction& EglGraphics::GetGlAbstraction() const
{
  DALI_ASSERT_DEBUG( mGLES && "GLImplementation not created" );
  return *mGLES;
}

EglImplementation& EglGraphics::GetEglImplementation() const
{
  DALI_ASSERT_DEBUG( mEglImplementation && "EGLImplementation not created" );
  return *mEglImplementation;
}

EglInterface& EglGraphics::GetEglInterface() const
{
  DALI_ASSERT_DEBUG( mEglImplementation && "EGLImplementation not created" );
  EglInterface* eglInterface = mEglImplementation.get();
  return *eglInterface;
}

EglSyncImplementation& EglGraphics::GetSyncImplementation()
{
  DALI_ASSERT_DEBUG( mEglSync && "EglSyncImplementation not created" );
  return *mEglSync;
}

EglContextHelperImplementation& EglGraphics::GetContextHelperImplementation()
{
  DALI_ASSERT_DEBUG( mEglContextHelper && "EglContextHelperImplementation not created" );
  return *mEglContextHelper;
}

EglImageExtensions* EglGraphics::GetImageExtensions()
{
  DALI_ASSERT_DEBUG( mEglImageExtensions && "EglImageExtensions not created" );
  return mEglImageExtensions.get();
}

} // Adaptor
} // Internal
} // Dali
