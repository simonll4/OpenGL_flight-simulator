#include "Shader.h"
#include "GLCheck.h"

namespace gfx {

void Shader::load(const char* vsPath, const char* fsPath) {
    // Leer archivos de shader
    std::string vertexCode = readFile(vsPath);
    std::string fragmentCode = readFile(fsPath);

    // Compilar shaders
    GLuint vertex = compileShader(vertexCode, GL_VERTEX_SHADER);
    GLuint fragment = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    // Crear programa
    prog_ = glCreateProgram();
    glAttachShader(prog_, vertex);
    glAttachShader(prog_, fragment);
    glLinkProgram(prog_);
    checkCompileErrors(prog_, "PROGRAM");

    // Limpiar shaders (ya están linkeados al programa)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::setMat4(const char* name, const glm::mat4& m) const {
    GLint location = glGetUniformLocation(prog_, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(m));
    }
}

void Shader::setInt(const char* name, int v) const {
    GLint location = glGetUniformLocation(prog_, name);
    if (location != -1) {
        glUniform1i(location, v);
    }
}

void Shader::setBool(const char* name, bool v) const {
    GLint location = glGetUniformLocation(prog_, name);
    if (location != -1) {
        glUniform1i(location, v ? 1 : 0);
    }
}

void Shader::setFloat(const char* name, float v) const {
    GLint location = glGetUniformLocation(prog_, name);
    if (location != -1) {
        glUniform1f(location, v);
    }
}

void Shader::setVec3(const char* name, const glm::vec3& v) const {
    GLint location = glGetUniformLocation(prog_, name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(v));
    }
}

std::string Shader::readFile(const char* path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    try {
        file.open(path);
        std::stringstream stream;
        stream << file.rdbuf();
        file.close();
        return stream.str();
    }
    catch (std::ifstream::failure& e) {
        throw std::runtime_error("ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " + std::string(path));
    }
}

GLuint Shader::compileShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, typeStr);
    
    return shader;
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            throw std::runtime_error("ERROR::SHADER_COMPILATION_ERROR of type: " + type + "\n" + infoLog);
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            throw std::runtime_error("ERROR::PROGRAM_LINKING_ERROR of type: " + type + "\n" + infoLog);
        }
    }
}

} // namespace gfx
