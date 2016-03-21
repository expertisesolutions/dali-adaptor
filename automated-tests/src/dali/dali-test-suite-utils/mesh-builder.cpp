/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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
#include "mesh-builder.h"

namespace Dali
{

Material CreateMaterial()
{
  Shader shader = Shader::New( "vertexSrc", "fragmentSrc" );
  Material material = Material::New(shader);

  return material;
}

Material CreateMaterial( Image image )
{
  Shader shader = Shader::New( "vertexSrc", "fragmentSrc" );
  Material material = Material::New(shader);

  material.AddTexture( image, "sTexture" );

  return material;
}

PropertyBuffer CreatePropertyBuffer()
{
  Property::Map texturedQuadVertexFormat;
  texturedQuadVertexFormat["aPosition"] = Property::VECTOR2;
  texturedQuadVertexFormat["aVertexCoord"] = Property::VECTOR2;

  PropertyBuffer vertexData = PropertyBuffer::New( texturedQuadVertexFormat );
  return vertexData;
}

Geometry CreateQuadGeometry(void)
{
  PropertyBuffer vertexData = CreatePropertyBuffer();
  return CreateQuadGeometryFromBuffer( vertexData );
}

Geometry CreateQuadGeometryFromBuffer( PropertyBuffer vertexData )
{
  const float halfQuadSize = .5f;
  struct TexturedQuadVertex { Vector2 position; Vector2 textureCoordinates; };
  TexturedQuadVertex texturedQuadVertexData[4] = {
    { Vector2(-halfQuadSize, -halfQuadSize), Vector2(0.f, 0.f) },
    { Vector2( halfQuadSize, -halfQuadSize), Vector2(1.f, 0.f) },
    { Vector2(-halfQuadSize,  halfQuadSize), Vector2(0.f, 1.f) },
    { Vector2( halfQuadSize,  halfQuadSize), Vector2(1.f, 1.f) } };
  vertexData.SetData(texturedQuadVertexData, 4);

  unsigned int indexData[6] = { 0, 3, 1, 0, 2, 3 };
  Property::Map indexFormat;
  indexFormat["indices"] = Property::INTEGER;
  PropertyBuffer indices = PropertyBuffer::New( indexFormat );
  indices.SetData( indexData, sizeof(indexData)/sizeof(indexData[0]) );

  Geometry geometry = Geometry::New();
  geometry.AddVertexBuffer( vertexData );
  geometry.SetIndexBuffer( indices );

  return geometry;
}



} // namespace Dali
