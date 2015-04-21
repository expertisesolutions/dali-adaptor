/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
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
#include <dali/internal/update/node-attachments/scene-graph-renderable-attachment.h>

// INTERNAL INCLUDES
#include <dali/integration-api/resource-declarations.h>
#include <dali/public-api/actors/renderable-actor.h>
#include <dali/internal/update/nodes/node.h>
#include <dali/internal/update/resources/resource-manager.h>
#include <dali/internal/update/resources/complete-status-manager.h>
#include <dali/internal/update/resources/resource-tracker.h>
#include <dali/internal/render/queue/render-queue.h>
#include <dali/internal/render/renderers/scene-graph-renderer.h>
#include <dali/internal/render/shaders/scene-graph-shader.h>
#include <dali/internal/common/image-sampler.h>

namespace Dali
{

namespace Internal
{

namespace SceneGraph
{

RenderableAttachment::RenderableAttachment( bool usesGeometryScaling )
: mSceneController(NULL), //@todo MESH_REWORK Pass in where required rather than store
  mShader( NULL ),
  mTrackedResources(),
  mSortModifier( 0.0f ),
  mBlendingMode( Dali::RenderableActor::DEFAULT_BLENDING_MODE ),
  mUsesGeometryScaling( usesGeometryScaling ),
  mScaleForSizeDirty( true ),
  mUseBlend( false ),
  mHasSizeAndColorFlag( false ),
  mResourcesReady( false ),
  mFinishedResourceAcquisition( false ),
  mHasUntrackedResources( false )
{
}

RenderableAttachment::~RenderableAttachment()
{
}

void RenderableAttachment::Initialize( SceneController& sceneController, BufferIndex updateBufferIndex )
{
  mSceneController = &sceneController;

  // Chain to derived attachments
  Initialize2( updateBufferIndex );
}

void RenderableAttachment::OnDestroy()
{
  // Chain to derived attachments
  OnDestroy2();

  // SceneController is no longer valid
  mSceneController = NULL;
}

void RenderableAttachment::SetRecalculateScaleForSize()
{
  mScaleForSizeDirty = true;
}

void RenderableAttachment::GetScaleForSize( const Vector3& nodeSize, Vector3& scaling )
{
  DoGetScaleForSize( nodeSize, scaling );
  mScaleForSizeDirty = false;
}

bool RenderableAttachment::ResolveVisibility( BufferIndex updateBufferIndex )
{
  mHasSizeAndColorFlag = false;
  const Vector4& color = mParent->GetWorldColor( updateBufferIndex );
  if( color.a > FULLY_TRANSPARENT )               // not fully transparent
  {
    const float MAX_NODE_SIZE = float(1u<<30);
    const Vector3& size = mParent->GetSize( updateBufferIndex );
    if( ( size.width > Math::MACHINE_EPSILON_1000 ) &&  // width is greater than a very small number
        ( size.height > Math::MACHINE_EPSILON_1000 ) )  // height is greater than a very small number
    {
      if( ( size.width < MAX_NODE_SIZE ) &&             // width is smaller than the maximum allowed size
          ( size.height < MAX_NODE_SIZE ) )             // height is smaller than the maximum allowed size
      {
        mHasSizeAndColorFlag = true;
      }
      else
      {
        DALI_LOG_ERROR("Actor size should not be bigger than %f.\n", MAX_NODE_SIZE );
        DALI_LOG_ACTOR_TREE( mParent );
      }
    }
  }
  return mHasSizeAndColorFlag;
}

void RenderableAttachment::DoGetScaleForSize( const Vector3& nodeSize, Vector3& scaling )
{
  scaling = Vector3::ONE;
}

void RenderableAttachment::PrepareResources( BufferIndex updateBufferIndex, ResourceManager& resourceManager )
{
  mHasUntrackedResources = false; // Only need to know this if the resources are not yet complete
  mTrackedResources.Clear(); // Resource trackers are only needed if not yet completea

  if( mShader )
  {
    Integration::ResourceId id = mShader->GetEffectTextureResourceId();

    if( id != 0 )
    {
      CompleteStatusManager& completeStatusManager = mSceneController->GetCompleteStatusManager();

      if(CompleteStatusManager::COMPLETE != completeStatusManager.GetStatus( id ))
      {
        FollowTracker(id);
        mFinishedResourceAcquisition = false;
        mResourcesReady = false;

        // If shader has effect texture and it's not complete and tracked, ensure
        // we call DoPrepareResources, as the effect texture may become ready in
        // the ProcessRenderTasks step.  Otherwise, may early out.
        if( mHasUntrackedResources )
        {
          return;
        }
      }
    }
  }

  mResourcesReady = DoPrepareResources( updateBufferIndex, resourceManager );
}

void RenderableAttachment::FollowTracker( Integration::ResourceId id )
{
  CompleteStatusManager& completeStatusManager = mSceneController->GetCompleteStatusManager();

  if( completeStatusManager.FindResourceTracker(id) != NULL )
  {
    bool found = false;
    std::size_t numTrackedResources = mTrackedResources.Count();
    for( size_t i=0; i < numTrackedResources; ++i )
    {
      if(mTrackedResources[i] == id)
      {
        found = true;
        break;
      }
    }
    if( ! found )
    {
      mTrackedResources.PushBack( id );
    }
  }
  else
  {
    mHasUntrackedResources = true;
  }
}


void RenderableAttachment::GetReadyAndComplete(bool& ready, bool& complete) const
{
  ready = mResourcesReady;
  complete = false;

  CompleteStatusManager& completeStatusManager = mSceneController->GetCompleteStatusManager();

  std::size_t numTrackedResources = mTrackedResources.Count();
  if( mHasUntrackedResources || numTrackedResources == 0 )
  {
    complete = mFinishedResourceAcquisition;
  }
  else
  {
    // If there are tracked resources and no untracked resources, test the trackers
    bool trackersComplete = true;
    for( size_t i=0; i < numTrackedResources; ++i )
    {
      ResourceId id = mTrackedResources[i];
      ResourceTracker* tracker = completeStatusManager.FindResourceTracker(id);
      if( tracker  && ! tracker->IsComplete() )
      {
        trackersComplete = false;
        break;
      }
    }

    complete = mFinishedResourceAcquisition || trackersComplete;
  }
}

void RenderableAttachment::PrepareRender( BufferIndex updateBufferIndex )
{
  // call the derived class first as it might change its state regarding blending
  DoPrepareRender( updateBufferIndex );

  // @todo MESH_REWORK Remove remainder of method after removing ImageAttachment

  bool blend = !IsFullyOpaque( updateBufferIndex );

  if ( mUseBlend != blend )
  {
    mUseBlend = blend;

    // Enable/disable blending in the next render
    typedef MessageValue1< Renderer, bool > DerivedType;

    // Reserve some memory inside the render queue
    unsigned int* slot = mSceneController->GetRenderQueue().ReserveMessageSlot( updateBufferIndex, sizeof( DerivedType ) );

    // Construct message in the render queue memory; note that delete should not be called on the return value
    new (slot) DerivedType( &GetRenderer(), &Renderer::SetUseBlend, blend );
  }
}

RenderableAttachment* RenderableAttachment::GetRenderable()
{
  return this;
}

void RenderableAttachment::SetSortModifier(float modifier)
{
  // Setting sort modifier makes the node dirty, i.e. we cannot reuse previous frames render items
  if( mParent )
  {
    // only do this if we are on-stage
    mParent->SetDirtyFlag( SortModifierFlag );
  }
  mSortModifier = modifier;
}


} // namespace SceneGraph

} // namespace Internal

} // namespace Dali
