// © 2019 Miguel Fernández Arce - All rights reserved

#include "CoreEngine.h"

namespace OpenGL {
	GLenum glCheckError_(const char* file, i32 line);
};

#define glCheckError() OpenGL::glCheckError_(__FILE__, __LINE__)
