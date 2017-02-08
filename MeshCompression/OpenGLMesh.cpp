#include "stdafx.h"
#include "OpenGLMesh.h"

OpenGLMesh::OpenGLMesh(const QString &name)
{
    OpenMesh::IO::read_mesh(mesh_, name.toStdString());
}


OpenGLMesh::~OpenGLMesh()
{
}
