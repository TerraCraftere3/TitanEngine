#include "RenderCommand.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Titan
{

    RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

}