#include "SkyboxRenderer.h"
#include "../core/GLCheck.h"
#include <array>
#include <glm/gtc/type_ptr.hpp>

namespace gfx
{

    ////////////////////////////////////////////////////////////////////////////////
    //  Geometría precomputada
    ////////////////////////////////////////////////////////////////////////////////

    /// Geometría del cubo unitario usada para dibujar el skybox (36 vértices).
    static const float CUBE_VERTICES[108] = {
        // Cara trasera (Z-)
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        // Cara frontal (Z+)
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,

        // Cara izquierda (X-)
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        // Cara derecha (X+)
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,

        // Cara inferior (Y-)
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,

        // Cara superior (Y+)
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f};

    ////////////////////////////////////////////////////////////////////////////
    //  Ciclo de vida
    ////////////////////////////////////////////////////////////////////////////

    SkyboxRenderer::~SkyboxRenderer()
    {
        if (vbo_)
            glDeleteBuffers(1, &vbo_);
        if (vao_)
            glDeleteVertexArrays(1, &vao_);
    }

    void SkyboxRenderer::init()
    {
        // La inicialización ocurre en dos pasos: geometría y shader.

        // 1) Buffers y atributos para el cubo unitario.
        createCubeGeometry();

        // 2) Compilación / link del shader específico del skybox.
        try
        {
            shader_.load("shaders/skybox.vert", "shaders/skybox.frag");
            std::cout << "Skybox shaders loaded successfully" << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to load skybox shaders: " << e.what() << std::endl;
            throw;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Configuración de buffers
    ////////////////////////////////////////////////////////////////////////////

    void SkyboxRenderer::createCubeGeometry()
    {
        // Generar objetos de geometría.
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);

        // Subir la lista de posiciones (solo XYZ) al VBO.
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);

        // El layout del shader usa location 0 para posiciones.
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        checkGLError("Creating skybox geometry");
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Render loop
    ////////////////////////////////////////////////////////////////////////////

    void SkyboxRenderer::draw(const glm::mat4 &view, const glm::mat4 &proj)
    {
        if (!cube_)
        {
            std::cerr << "No cubemap texture set for skybox" << std::endl;
            return;
        }

        // El skybox debe dibujarse incluso cuando comparte z=1 con la profundidad, por lo
        // que relajamos la comparación para permitir valores iguales (LEQUAL).
        glDepthFunc(GL_LEQUAL);

        shader_.use();

        // Para evitar que el cubo parezca trasladarse con la cámara, eliminamos la
        // traslación y conservamos únicamente la rotación de la vista.
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));

        // Uniformes básicos: vista, proyección y unidad de textura.
        shader_.setMat4("uView", viewNoTranslation);
        shader_.setMat4("uProj", proj);
        shader_.setInt("uCube", 0);

        // Activar y vincular la textura cubemap en la unidad 0.
        cube_->bindUnit(0);

        // Renderizar el cubo unitario (36 vértices).
        glBindVertexArray(vao_);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Restablecer la función por defecto para posteriores draw calls.
        glDepthFunc(GL_LESS);
    }

} // namespace gfx
