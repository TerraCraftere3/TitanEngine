#include "OpenGLCubemap.h"
#include "Titan/Core/Log.h"
#include "Titan/PCH.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#undef min
#undef max

namespace Titan
{

    static const char* s_EquirectToCubemapVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 LocalPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    LocalPos = aPos;
    gl_Position = projection * view * vec4(LocalPos, 1.0);
}
)";

    static const char* s_EquirectToCubemapFragmentShader = R"(
#version 330 core
out vec4 FragColor;
in vec3 LocalPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(LocalPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}
)";

    // Irradiance convolution shader
    static const char* s_IrradianceFragmentShader = R"(
#version 330 core
out vec4 FragColor;
in vec3 LocalPos;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main()
{
    // The world vector acts as the normal of a tangent surface
    // from the origin, aligned to LocalPos. Given this normal, calculate all
    // incoming radiance of the environment. The result of this radiance
    // is the radiance of light coming from -Normal direction, which is what
    // we use in the PBR shader to sample irradiance.
    vec3 N = normalize(LocalPos);

    vec3 irradiance = vec3(0.0);

    // Tangent space calculation from origin point
    vec3 up    = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up         = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;
    
    for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // Tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; 

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}
)";

    // Cube vertices for rendering
    static float s_CubeVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                                     1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                                     -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                                     -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                                     1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                                     -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                                     -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                                     1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                                     -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                                     1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    static GLuint CompileShader(GLenum type, const char* source)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetShaderInfoLog(shader, 512, nullptr, infoLog);
            TI_CORE_ERROR("Shader compilation failed: {0}", infoLog);
        }

        return shader;
    }

    static GLuint CreateShaderProgram(const char* vertexSource, const char* fragmentSource)
    {
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[512];
            glGetProgramInfoLog(program, 512, nullptr, infoLog);
            TI_CORE_ERROR("Shader program linking failed: {0}", infoLog);
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    OpenGLCubemap::OpenGLCubemap(const std::string& path) : m_Path(path)
    {
        // Load HDR image
        stbi_set_flip_vertically_on_load(true);
        int width, height, channels;
        float* data = stbi_loadf(path.c_str(), &width, &height, &channels, 0);

        if (!data)
        {
            TI_CORE_ERROR("Failed to load HDR image: {0}", path);
            return;
        }

        TI_CORE_INFO("Loaded HDR image: {0} ({1}x{2}, {3} channels)", path, width, height, channels);

        // Create temporary equirectangular texture
        GLuint hdrTexture;
        glGenTextures(1, &hdrTexture);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        // Create cubemap texture
        m_Width = m_Height = 512; // Default cubemap resolution
        glGenTextures(1, &m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT,
                         nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Setup framebuffer for rendering to cubemap
        GLuint captureFBO, captureRBO;
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_Width, m_Height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        // Setup shader program
        GLuint shaderProgram = CreateShaderProgram(s_EquirectToCubemapVertexShader, s_EquirectToCubemapFragmentShader);

        // Setup cube geometry
        GLuint cubeVAO, cubeVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_CubeVertices), s_CubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Setup projection and view matrices for each face
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        // Convert HDR equirectangular environment map to cubemap
        glUseProgram(shaderProgram);
        glUniform1i(glGetUniformLocation(shaderProgram, "equirectangularMap"), 0);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &captureProjection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);

        glViewport(0, 0, m_Width, m_Height);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &captureViews[i][0][0]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   m_RendererID, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Generate mipmaps
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // Calculate mip levels
        m_MipLevels = 1 + static_cast<uint32_t>(floor(log2(glm::max(m_Width, m_Height))));

        // Cleanup
        glDeleteTextures(1, &hdrTexture);
        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteProgram(shaderProgram);

        TI_CORE_INFO("Created cubemap from HDR: {0} (Resolution: {1}x{2}, Mips: {3})", path, m_Width, m_Height,
                        m_MipLevels);
    }

    // Constructor for creating empty cubemap (used by CreateIrradianceMap)
    OpenGLCubemap::OpenGLCubemap() : m_Path(""), m_RendererID(0), m_Width(0), m_Height(0), m_MipLevels(0) {}

    OpenGLCubemap::~OpenGLCubemap()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLCubemap::SetData(void* data, uint32_t size)
    {
        // Not typically used for cubemaps loaded from HDR
        TI_CORE_WARN("SetData not implemented for OpenGLCubemap");
    }

    void OpenGLCubemap::SetFaceData(uint32_t face, void* data, uint32_t size)
    {
        if (face >= 6)
        {
            TI_CORE_ERROR("Invalid cubemap face index: {0}", face);
            return;
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        // Assuming RGB16F format
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_FLOAT, data);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    }

    void OpenGLCubemap::Bind(uint32_t slot) const
    {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    }

    Ref<Cubemap> OpenGLCubemap::CreateIrradianceMap(uint32_t resolution)
    {
        auto irradianceMap = CreateRef<OpenGLCubemap>();
        irradianceMap->m_Width = resolution;
        irradianceMap->m_Height = resolution;
        irradianceMap->m_Path = m_Path + "_irradiance";

        // Create irradiance cubemap texture
        glGenTextures(1, &irradianceMap->m_RendererID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap->m_RendererID);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT,
                         nullptr);
        }

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Setup framebuffer
        GLuint captureFBO, captureRBO;
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        // Create irradiance shader program
        GLuint irradianceShader = CreateShaderProgram(s_EquirectToCubemapVertexShader, s_IrradianceFragmentShader);

        // Setup cube geometry
        GLuint cubeVAO, cubeVBO;
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(s_CubeVertices), s_CubeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        // Setup projection and view matrices
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[] = {
            glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
            glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

        // Convolve environment map to create irradiance map
        glUseProgram(irradianceShader);
        glUniform1i(glGetUniformLocation(irradianceShader, "environmentMap"), 0);
        glUniformMatrix4fv(glGetUniformLocation(irradianceShader, "projection"), 1, GL_FALSE, &captureProjection[0][0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

        glViewport(0, 0, resolution, resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

        for (unsigned int i = 0; i < 6; ++i)
        {
            glUniformMatrix4fv(glGetUniformLocation(irradianceShader, "view"), 1, GL_FALSE, &captureViews[i][0][0]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                                   irradianceMap->m_RendererID, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glBindVertexArray(cubeVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Calculate mip levels (irradiance maps typically don't need many mips)
        irradianceMap->m_MipLevels = 1;

        // Cleanup
        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteProgram(irradianceShader);

        TI_CORE_INFO("Created irradiance map from cubemap: {0} (Resolution: {1}x{2})", m_Path, resolution, resolution);

        return irradianceMap;
    }

} // namespace Titan