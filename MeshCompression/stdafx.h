#include <QtWidgets>
#include <QAction>
#include <QKeyEvent>
#include <QtOpenGl/QGLWidget>
#include <QEvent>

// this Macro has been put to Preprocessor Setting 
// (in Project Property - C/C++ - Preprocessor - Definitions)
// to avoid warnings for Macro redefine in math.h.
//#define  _USE_MATH_DEFINES 
#include <stdlib.h>
#include <stdio.h>
#include <gl/glut.h>

// Before these files can be included, 
// the files should be in the list of 'VC++ Include Directory',
// meanwhile the relating .lib file should in 'VC++ Library Directory'.
// Before you can compile them, add OpenMeshCored.lib and OpenMeshToolsd.lib
// to Project Property - Linker - Input - Additional Dependencies.
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
