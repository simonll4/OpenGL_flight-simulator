#pragma once
#include <stdexcept>
#include <string>
#include <iostream>

extern "C"
{
#include <glad/glad.h>
#include <GLFW/glfw3.h>
}

namespace gfx
{

    /**
     * @brief Valida una condición relacionada a OpenGL y lanza excepción si falla.
     * @param ok Resultado booleano del chequeo manual.
     * @param msg Mensaje contextual que se añadirá al texto de error.
     */
    inline void glCheck(bool ok, const char *msg)
    {
        if (!ok)
        {
            throw std::runtime_error(std::string{"GL error: "} + msg);
        }
    }

    /**
     * @brief Consulta glGetError e informa qué operación provocó el fallo.
     *
     * Se usa después de bloques críticos (creación de buffers, uploads de texturas,
     * etc.) para detectar errores de estado y abortar con un mensaje descriptivo.
     */
    inline void checkGLError(const char *operation)
    {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::string errorMsg = "OpenGL error in " + std::string(operation) + ": ";
            switch (error)
            {
            case GL_INVALID_ENUM:
                errorMsg += "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                errorMsg += "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                errorMsg += "GL_INVALID_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                errorMsg += "GL_OUT_OF_MEMORY";
                break;
            default:
                errorMsg += "Unknown error " + std::to_string(error);
                break;
            }
            throw std::runtime_error(errorMsg);
        }
    }

} // namespace gfx
