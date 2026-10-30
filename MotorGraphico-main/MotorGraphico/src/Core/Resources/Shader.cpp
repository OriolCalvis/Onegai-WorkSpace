#include "Core/Resources/Shader.h"

#include "Core/Errors/EngineException.h"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>

unsigned int Shader::compileStage(unsigned int glShaderType, const std::string& source) {
    unsigned int id = glCreateShader(glShaderType);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int compiled = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        int logLen = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<std::size_t>(logLen));
        glGetShaderInfoLog(id, logLen, nullptr, log.data());
        glDeleteShader(id);
        throw ShaderCompileException(std::string(log.data(), log.size()));
    }
    return id;
}

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    unsigned int vs = compileStage(GL_VERTEX_SHADER, vertexSource);
    unsigned int fs = compileStage(GL_FRAGMENT_SHADER, fragmentSource);

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vs);
    glAttachShader(m_programID, fs);
    glLinkProgram(m_programID);

    int linked = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &linked);

    // Los stages individuales ya se pueden marcar para borrado en cuanto
    // se adjuntan al programa; GL los libera de verdad al hacer
    // glDeleteProgram si el link falla.
    glDeleteShader(vs);
    glDeleteShader(fs);

    if (!linked) {
        int logLen = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> log(static_cast<std::size_t>(logLen));
        glGetProgramInfoLog(m_programID, logLen, nullptr, log.data());
        glDeleteProgram(m_programID);
        m_programID = 0;
        throw ShaderCompileException(std::string(log.data(), log.size()));
    }
}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
    }
}

void Shader::use() const { glUseProgram(m_programID); }

void Shader::setUniformInt(const std::string& name, int value) {
    int loc = glGetUniformLocation(m_programID, name.c_str());
    glUniform1i(loc, value);
}

void Shader::setUniformFloat(const std::string& name, float value) {
    int loc = glGetUniformLocation(m_programID, name.c_str());
    glUniform1f(loc, value);
}

void Shader::setUniformVec2(const std::string& name, const glm::vec2& vec) {
    int loc = glGetUniformLocation(m_programID, name.c_str());
    glUniform2fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setUniformVec3(const std::string& name, const glm::vec3& vec) {
    int loc = glGetUniformLocation(m_programID, name.c_str());
    glUniform3fv(loc, 1, glm::value_ptr(vec));
}

void Shader::setUniformMat4(const std::string& name, const glm::mat4& mat) {
    int loc = glGetUniformLocation(m_programID, name.c_str());
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}
