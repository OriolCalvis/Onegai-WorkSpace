#pragma once

#include <string>

// glm::mat4 es un alias de plantilla (glm::mat<4,4,float,...>), no una
// clase: no se puede forward-declarar con "struct mat4;" como si fuera
// un tipo normal. <glm/fwd.hpp> es el header oficial de glm pensado
// justo para esto: declara los tipos sin arrastrar toda la libreria.
#include <glm/fwd.hpp>

// Compila y enlaza un programa GLSL (vertex + fragment). El constructor
// lanza ShaderCompileException si falla; ResourceManager<Shader>::load()
// captura esa excepcion y la convierte en Result<Shader*>::Error, asi que
// el codigo de aplicacion nunca necesita un try/catch por si mismo.
class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;
    void setUniformInt(const std::string& name, int value);
    void setUniformFloat(const std::string& name, float value);
    void setUniformVec2(const std::string& name, const glm::vec2& vec);
    void setUniformVec3(const std::string& name, const glm::vec3& vec);
    void setUniformMat4(const std::string& name, const glm::mat4& mat);

private:
    unsigned int compileStage(unsigned int glShaderType, const std::string& source);
    unsigned int m_programID = 0;
};
