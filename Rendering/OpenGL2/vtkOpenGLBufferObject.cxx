// SPDX-FileCopyrightText: Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
// SPDX-License-Identifier: BSD-3-Clause
#include "vtkOpenGLBufferObject.h"
#include "vtkObjectFactory.h"

#include "vtk_glew.h"

VTK_ABI_NAMESPACE_BEGIN
vtkStandardNewMacro(vtkOpenGLBufferObject);

namespace
{
inline GLenum convertType(vtkOpenGLBufferObject::ObjectType type)
{
  switch (type)
  {
    case vtkOpenGLBufferObject::ElementArrayBuffer:
      return GL_ELEMENT_ARRAY_BUFFER;
    case vtkOpenGLBufferObject::TextureBuffer:
#if defined(GL_TEXTURE_BUFFER)
      return GL_TEXTURE_BUFFER;
      // intentional fall through when not defined
#endif
    default:
    case vtkOpenGLBufferObject::ArrayBuffer:
      return GL_ARRAY_BUFFER;
  }
}
}

struct vtkOpenGLBufferObject::Private
{
  Private()
  {
    this->Handle = 0;
    this->Type = GL_ARRAY_BUFFER;
  }
  GLenum Type;
  GLuint Handle;
};

vtkOpenGLBufferObject::vtkOpenGLBufferObject()
{
  this->Dirty = true;
  this->Internal = new Private;
  this->Internal->Type = convertType(vtkOpenGLBufferObject::ArrayBuffer);
}

vtkOpenGLBufferObject::~vtkOpenGLBufferObject()
{
  if (this->Internal->Handle != 0)
  {
    glDeleteBuffers(1, &this->Internal->Handle);
  }
  delete this->Internal;
}

void vtkOpenGLBufferObject::ReleaseGraphicsResources()
{
  if (this->Internal->Handle != 0)
  {
    glBindBuffer(this->Internal->Type, 0);
    glDeleteBuffers(1, &this->Internal->Handle);
    this->Internal->Handle = 0;
  }
}

void vtkOpenGLBufferObject::SetType(vtkOpenGLBufferObject::ObjectType value)
{
  this->Internal->Type = convertType(value);
}

vtkOpenGLBufferObject::ObjectType vtkOpenGLBufferObject::GetType() const
{
  if (this->Internal->Type == GL_ARRAY_BUFFER)
  {
    return vtkOpenGLBufferObject::ArrayBuffer;
  }
  if (this->Internal->Type == GL_ELEMENT_ARRAY_BUFFER)
  {
    return vtkOpenGLBufferObject::ElementArrayBuffer;
  }
  else
  {
    return vtkOpenGLBufferObject::TextureBuffer;
  }
}

int vtkOpenGLBufferObject::GetHandle() const
{
  return static_cast<int>(this->Internal->Handle);
}

bool vtkOpenGLBufferObject::Bind()
{
  if (!this->Internal->Handle)
  {
    return false;
  }

  glBindBuffer(this->Internal->Type, this->Internal->Handle);
  return true;
}

bool vtkOpenGLBufferObject::Release()
{
  if (!this->Internal->Handle)
  {
    return false;
  }

  glBindBuffer(this->Internal->Type, 0);
  return true;
}

bool vtkOpenGLBufferObject::GenerateBuffer(vtkOpenGLBufferObject::ObjectType objectType)
{
  GLenum objectTypeGL = convertType(objectType);
  if (this->Internal->Handle == 0)
  {
    glGenBuffers(1, &this->Internal->Handle);
    this->Internal->Type = objectTypeGL;
  }
  return (this->Internal->Type == objectTypeGL);
}

bool vtkOpenGLBufferObject::UploadInternal(
  const void* buffer, size_t size, vtkOpenGLBufferObject::ObjectType objectType)
{
  const bool generated = this->GenerateBuffer(objectType);
  if (!generated)
  {
    this->Error = "Trying to upload array buffer to incompatible buffer.";
    return false;
  }

  // std::cout << "Uploading " << size << " bytes using glBufferData" << std::endl;
  glBindBuffer(this->Internal->Type, this->Internal->Handle);
  glBufferData(this->Internal->Type, size, static_cast<const GLvoid*>(buffer), GL_STATIC_DRAW);
  this->Dirty = false;
  return true;
}

//------------------------------------------------------------------------------
void vtkOpenGLBufferObject::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
VTK_ABI_NAMESPACE_END
