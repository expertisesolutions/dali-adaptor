//
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Flora License, Version 1.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://floralicense.org/license/
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an AS IS BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// CLASS HEADER
#include <dali/internal/render/common/texture-cache-dispatcher.h>

// INTERNAL INCLUDES
#include <dali/internal/render/queue/render-queue.h>

namespace Dali
{

namespace Internal
{

namespace SceneGraph
{

TextureCacheDispatcher::TextureCacheDispatcher( RenderQueue& renderQueue )
: mRenderQueue(renderQueue),
  mSceneGraphBuffers(NULL)
{
}

TextureCacheDispatcher::~TextureCacheDispatcher()
{
}

void TextureCacheDispatcher::SetBufferIndices( const SceneGraphBuffers* bufferIndices )
{
  mSceneGraphBuffers = bufferIndices;
}

} // SceneGraph

} // Internal

} // Dali
