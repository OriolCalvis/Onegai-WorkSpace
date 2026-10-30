#version 330 core

// Fullscreen triangle: NO usa atributos ni VBO. Se dibuja con
// glDrawArrays(GL_TRIANGLES, 0, 3) y el vertex shader genera las 3
// posiciones NDC directamente a partir de gl_VertexID. Cubre todo el
// framebuffer con un unico triangulo (mas eficiente que un quad de 2
// triangulos y sin generar la costura diagonal). UV de pantalla en [0,1]
// para muestrear la textura de escena y el lightmap alineadamente.
out vec2 vScreenUV;

void main() {
    // gl_VertexID: 0 -> (-1,-1), 1 -> (3,-1), 2 -> (-1,3). El triangulo
    // gigante cubre el clip space [-1,1]^2 con exceso a la derecha/arriba
    // (que el rasterizer descarta fuera del viewport).
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
    // FBO y máscaras GL comparten origen abajo-izquierda. Sin invertir Y:
    // cada pase debe conservar la orientación de la escena anterior.
    vScreenUV = pos;
}
