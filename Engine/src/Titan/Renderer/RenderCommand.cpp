#include "RenderCommand.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLRendererAPI.h"

namespace Titan
{

    Scope<RendererAPI> RenderCommand::s_RendererAPI = CreateScope<OpenGLRendererAPI>();
}