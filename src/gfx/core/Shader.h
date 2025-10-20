#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

extern "C" {
#include <glad/glad.h>
}

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace gfx {

class Shader {
public:
    Shader() = default;
    Shader(const char* vsPath, const char* fsPath) { load(vsPath, fsPath); }
    ~Shader() { 
        if (prog_) glDeleteProgram(prog_); 
    }

    // No permitir copia, solo movimiento
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept : prog_(other.prog_) { other.prog_ = 0; }
    Shader& operator=(Shader&& other) noexcept {
        if (this != &other) {
            if (prog_) glDeleteProgram(prog_);
            prog_ = other.prog_;
            other.prog_ = 0;
        }
        return *this;
    }

    void load(const char* vsPath, const char* fsPath);
    void use() const { glUseProgram(prog_); }
    GLuint id() const { return prog_; }

    // Setters para uniformes
    void setMat4(const char* name, const glm::mat4& m) const;
    void setInt(const char* name, int v) const;
    void setBool(const char* name, bool v) const;
    void setFloat(const char* name, float v) const;
    void setVec3(const char* name, const glm::vec3& v) const;

private:
    GLuint prog_ = 0;
    
    std::string readFile(const char* path);
    GLuint compileShader(const std::string& source, GLenum type);
    void checkCompileErrors(GLuint shader, const std::string& type);
};

} // namespace gfx
