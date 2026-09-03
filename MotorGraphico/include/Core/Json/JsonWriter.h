#pragma once

#include <cstdio>
#include <string>
#include <vector>

// Escritor JSON minimo, contrapartida de JsonValue (que solo LEE).
//
// Por que hacia falta: hasta RUN-10 el motor solo consumia JSON (niveles,
// catalogos, aventuras). Guardar una partida obliga por primera vez a
// PRODUCIRLO. Se mantiene el mismo criterio que en JsonValue.h -- parser
// propio, cero dependencias vendorizadas -- porque lo que hay que escribir
// (objetos, arrays, strings, numeros, bool) cabe en unas decenas de lineas
// y anadir una libreria por esto seria desproporcionado.
//
// Header-only y GL-free: se prueba sin ventana y no obliga a enlazar nada.
//
// Es un escritor de FLUJO, no un arbol: se abre un objeto, se escriben sus
// miembros y se cierra. No valida que el documento este balanceado (eso lo
// hace el test que reparsea lo escrito con JsonValue::parse, que es la
// comprobacion que de verdad importa: lo que sale de aqui tiene que poder
// volver a entrar).
class JsonWriter {
public:
    explicit JsonWriter(int indentWidth = 2) : m_indentWidth(indentWidth) {}

    void beginObject() {
        writePrefix();
        m_out += '{';
        m_stack.push_back('o');
        m_needComma = false;
    }

    void endObject() { closeContainer('}'); }

    void beginArray() {
        writePrefix();
        m_out += '[';
        m_stack.push_back('a');
        m_needComma = false;
    }

    void endArray() { closeContainer(']'); }

    // Clave de un miembro de objeto. El valor va en la llamada siguiente
    // (puede ser un escalar o un beginObject/beginArray).
    void key(const std::string& name) {
        writePrefix();
        m_out += '"';
        appendEscaped(name);
        m_out += "\": ";
        m_afterKey = true;
    }

    void value(const std::string& text) {
        writePrefix();
        m_out += '"';
        appendEscaped(text);
        m_out += '"';
        m_needComma = true;
    }

    void value(const char* text) { value(std::string(text ? text : "")); }

    void value(int number) {
        writePrefix();
        m_out += std::to_string(number);
        m_needComma = true;
    }

    void value(bool flag) {
        writePrefix();
        m_out += flag ? "true" : "false";
        m_needComma = true;
    }

    // Los float se escriben con 6 decimales y sin notacion cientifica: el
    // unico consumidor hoy son los temporizadores de mision (segundos
    // restantes), donde la precision de milisegundos sobra y un "1e-05"
    // suelto solo complicaria la lectura a mano del guardado.
    void value(float number) {
        writePrefix();
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.6f", static_cast<double>(number));
        m_out += buffer;
        m_needComma = true;
    }

    void valueNull() {
        writePrefix();
        m_out += "null";
        m_needComma = true;
    }

    // Atajos para el caso comun (una clave y su escalar).
    void member(const std::string& name, const std::string& text) { key(name); value(text); }
    void member(const std::string& name, const char* text) { key(name); value(text); }
    void member(const std::string& name, int number) { key(name); value(number); }
    void member(const std::string& name, bool flag) { key(name); value(flag); }
    void member(const std::string& name, float number) { key(name); value(number); }

    // Documento terminado, con salto de linea final (los ficheros de texto
    // del repositorio lo llevan, y git deja de quejarse de "\ No newline").
    std::string str() const { return m_out + "\n"; }

private:
    void appendEscaped(const std::string& text) {
        for (unsigned char c : text) {
            switch (c) {
                case '"': m_out += "\\\""; break;
                case '\\': m_out += "\\\\"; break;
                case '\b': m_out += "\\b"; break;
                case '\f': m_out += "\\f"; break;
                case '\n': m_out += "\\n"; break;
                case '\r': m_out += "\\r"; break;
                case '\t': m_out += "\\t"; break;
                default:
                    // Los caracteres de control por debajo de 0x20 son
                    // ilegales en crudo dentro de una cadena JSON; el resto
                    // (incluido UTF-8 multibyte) se copia tal cual, que es
                    // lo que espera JsonValue::parse al leerlo de vuelta.
                    if (c < 0x20) {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                        m_out += buffer;
                    } else {
                        m_out += static_cast<char>(c);
                    }
                    break;
            }
        }
    }

    void writePrefix() {
        if (m_afterKey) {
            m_afterKey = false;
            return;
        }
        if (m_needComma) {
            m_out += ',';
        }
        if (!m_stack.empty()) {
            m_out += '\n';
            m_out.append(static_cast<std::size_t>(m_indentWidth) * m_stack.size(), ' ');
        }
    }

    void closeContainer(char symbol) {
        const bool empty = !m_needComma;
        if (!m_stack.empty()) {
            m_stack.pop_back();
        }
        if (!empty) {
            m_out += '\n';
            m_out.append(static_cast<std::size_t>(m_indentWidth) * m_stack.size(), ' ');
        }
        m_out += symbol;
        m_needComma = true;
        m_afterKey = false;
    }

    std::string m_out;
    std::vector<char> m_stack;
    int m_indentWidth = 2;
    bool m_needComma = false;
    bool m_afterKey = false;
};
