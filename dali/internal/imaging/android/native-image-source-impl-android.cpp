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

#define EGL_EGLEXT_PROTOTYPES
#if __ANDROID_API__ < 26
#error "Unsupported Android API version, must be >= 26"
#endif

// CLASS HEADER
#include <dali/internal/imaging/android/native-image-source-impl-android.h>

// EXTERNAL INCLUDES
#include <EGL/egl.h>
#include <include/EGL/eglext.h>
#include <dali/integration-api/debug.h>

// INTERNAL INCLUDES
#include <dali/devel-api/adaptor-framework/bitmap-saver.h>
#include <dali/integration-api/render-surface.h>
#include <dali/internal/graphics/common/egl-image-extensions.h>
#include <dali/internal/graphics/gles/egl-graphics.h>
#include <dali/internal/adaptor/common/adaptor-impl.h>

namespace Dali
{

namespace Internal
{

namespace Adaptor
{
using Dali::Integration::PixelBuffer;

NativeImageSourceAndroid* NativeImageSourceAndroid::New( uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
{
  NativeImageSourceAndroid* image = new NativeImageSourceAndroid( width, height, depth, nativeImageSource );
  DALI_ASSERT_DEBUG( image && "NativeImageSource allocation failed." );

  // 2nd phase construction
  if( image ) //< Defensive in case we ever compile without exceptions.
  {
    image->Initialize();
  }

  return image;
}

NativeImageSourceAndroid::NativeImageSourceAndroid( uint32_t width, uint32_t height, Dali::NativeImageSource::ColorDepth depth, Any nativeImageSource )
: mWidth( width ),
  mHeight( height ),
  mOwnPixmap( true ),
  mPixmap( NULL ),
  mBlendingRequired( false ),
  mColorDepth( depth ),
  mEglImageKHR( NULL ),
  mEglImageExtensions( NULL )
{
  DALI_ASSERT_ALWAYS( Adaptor::IsAvailable() );

  GraphicsInterface* graphics = &( Adaptor::GetImplementation( Adaptor::Get() ).GetGraphicsInterface() );
  auto eglGraphics = static_cast<EglGraphics*>( graphics );

  mEglImageExtensions = eglGraphics->GetImageExtensions();

  DALI_ASSERT_DEBUG( mEglImageExtensions );

  // assign the pixmap
  mPixmap = static_cast<AHardwareBuffer*>( GetPixmapFromAny( nativeImageSource ) );
  if( !mPixmap )
  {
    AHardwareBuffer_Desc bufferDescription;
    memset( &bufferDescription, 0, sizeof( AHardwareBuffer_Desc ) );
    bufferDescription.width = width;
    bufferDescription.height = height;
    bufferDescription.layers = 1;
    bufferDescription.usage = AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN | AHARDWAREBUFFER_USAGE_CPU_READ_RARELY | AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE;
    switch( mColorDepth )
    {
      case Dali::NativeImageSource::COLOR_DEPTH_32:
      case Dali::NativeImageSource::COLOR_DEPTH_DEFAULT:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_24:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_16:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM;
        break;
      case Dali::NativeImageSource::COLOR_DEPTH_8:
        bufferDescription.format = AHARDWAREBUFFER_FORMAT_BLOB;
        break;
    }

    int ret = AHardwareBuffer_allocate( &bufferDescription, &mPixmap );
    if( ret )
    {
      DALI_LOG_ERROR("Failed to allocate AHardwareBuffer %d", ret);
    }

    mOwnPixmap = true;
  }
}

void NativeImageSourceAndroid::Initialize()
{
  if( mPixmap && !mOwnPixmap )
  {
    AHardwareBuffer_acquire( mPixmap ) ;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

NativeImageSourceAndroid::~NativeImageSourceAndroid()
{
  AHardwareBuffer_release( mPixmap );
  mPixmap = NULL;
}

Any NativeImageSourceAndroid::GetNativeImageSource() const
{
  return Any( mPixmap );
}

bool NativeImageSourceAndroid::GetPixels(std::vector<unsigned char>& pixbuf, unsigned& width, unsigned& height, Pixel::Format& pixelFormat) const
{
  DALI_ASSERT_DEBUG( sizeof(unsigned) == 4 );
  bool success = false;

  width  = mWidth;
  height = mHeight;

  AHardwareBuffer_Desc bufferDescription;
  memset( &bufferDescription, 0, sizeof( AHardwareBuffer_Desc ) );
  AHardwareBuffer_describe( mPixmap, &bufferDescription );
  switch( bufferDescription.format )
  {
    case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
      pixelFormat = Pixel::Format::RGBA8888;
      break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM:
      pixelFormat = Pixel::Format::RGB8888;
      break;
    case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
      pixelFormat = Pixel::Format::RGB888;
      break;
    case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
      pixelFormat = Pixel::Format::RGB565;
      break;
    case AHARDWAREBUFFER_FORMAT_BLOB:
    default:
      pixelFormat = Pixel::Format::A8;
      break;
  }

  void* buffer = NULL;
  int ret = AHardwareBuffer_lock( mPixmap, AHARDWAREBUFFER_USAGE_CPU_READ_RARELY, -1, NULL, &buffer );
  if( ret != 0 )
  {
    DALI_LOG_ERROR("Failed to AHardwareBuffer_lock %d", ret);
    return success;
  }

  uint32_t size = bufferDescription.stride * bufferDescription.height;
  pixbuf.resize( size );
  memcpy( pixbuf.data(), buffer, size );

  ret = AHardwareBuffer_unlock( mPixmap, NULL );
  if( ret != 0 )
  {
    DALI_LOG_ERROR("failed to AHardwareBuffer_unlock %d", ret);
    return success;
  }
  success = true;

  return success;
}

bool NativeImageSourceAndroid::EncodeToFile(const std::string& filename) const
{
  std::vector< unsigned char > pixbuf;
  unsigned int width(0), height(0);
  Pixel::Format pixelFormat;

  if( GetPixels( pixbuf, width, height, pixelFormat ) )
  {
    return Dali::EncodeToFile( &pixbuf[0], filename, pixelFormat, width, height );
  }
  return false;
}

void NativeImageSourceAndroid::SetSource( Any source )
{
  if( mPixmap )
  {
    mOwnPixmap = false;

    AHardwareBuffer_release( mPixmap );
    mPixmap = NULL;
  }

  mPixmap = static_cast<AHardwareBuffer*>( GetPixmapFromAny( source ) );

  if( mPixmap )
  {
    // we don't own the pixmap
    mOwnPixmap = false;

    // find out the pixmap width / height and color depth
    GetPixmapDetails();
  }
}

bool NativeImageSourceAndroid::IsColorDepthSupported( Dali::NativeImageSource::ColorDepth colorDepth )
{
  return true;
}

bool NativeImageSourceAndroid::GlExtensionCreate()
{
  // if the image existed previously delete it.
  if( mEglImageKHR != NULL )
  {
    GlExtensionDestroy();
  }

  DALI_ASSERT_ALWAYS( mPixmap );
  EGLClientBuffer eglBuffer = eglGetNativeClientBufferANDROID( mPixmap );
  switch( eglGetError() )
  {
    case EGL_SUCCESS :
    {
      break;
    }
    case EGL_BAD_PARAMETER:
    {
      DALI_LOG_ERROR( "EGL_BAD_PARAMETER: bad pixmap parameter\n" );
      break;
    }
    case EGL_BAD_ACCESS:
    {
      DALI_LOG_ERROR( "EGL_BAD_ACCESS: bad access to pixmap\n" );
      break;
    }
    case EGL_BAD_ALLOC:
    {
      DALI_LOG_ERROR( "EGL_BAD_ALLOC: Insufficient memory is available\n" );
      break;
    }
    default:
    {
      DALI_LOG_ERROR( "eglGetNativeClientBufferANDROID error\n" );
      break;
    }
  }

  DALI_ASSERT_ALWAYS( eglBuffer );
  mEglImageKHR = mEglImageExtensions->CreateImageKHR( eglBuffer );

  return mEglImageKHR != NULL;
}

void NativeImageSourceAndroid::GlExtensionDestroy()
{
  mEglImageExtensions->DestroyImageKHR( mEglImageKHR );

  mEglImageKHR = NULL;
}

uint32_t NativeImageSourceAndroid::TargetTexture()
{
  mEglImageExtensions->TargetTextureKHR( mEglImageKHR );

  return 0;
}

void NativeImageSourceAndroid::PrepareTexture()
{
}

void* NativeImageSourceAndroid::GetPixmapFromAny(Any pixmap) const
{
  if( pixmap.Empty() )
  {
    return 0;
  }

  return AnyCast<void*>( pixmap );
}

void NativeImageSourceAndroid::GetPixmapDetails()
{
  // get the width, height and depth
  mBlendingRequired = false;

  AHardwareBuffer_Desc* bufferDescription = nullptr;
  AHardwareBuffer_describe( mPixmap, bufferDescription );

  mWidth = bufferDescription->width;
  mHeight = bufferDescription->height;
  switch (bufferDescription->format)
  {
  case AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM:
    mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_32;
    break;
  case AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM:
    mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_24;
    break;
  case AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM:
    mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_16;
    break;
  case AHARDWAREBUFFER_FORMAT_BLOB:
  default:
    mColorDepth = Dali::NativeImageSource::COLOR_DEPTH_8;
  }
}

} // namespace Adaptor

} // namespace internal

} // namespace Dali
