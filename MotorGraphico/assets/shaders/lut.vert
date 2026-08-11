#version 330 core

// Fullscreen triangle: identico a lightmap.vert/fog.vert (mismo patron de
// post-procesado). Se dibuja con glDrawArrays(GL_TRIANGLES, 0, 3) y genera
// las 3 posiciones NDC a partir de gl_VertexID, sin VBO ni atributos.
out vec2 vScreenUV;

void main() {
    vec2 pos = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    gl_Position = vec4(pos * 2.0 - 1.0, 0.0, 1.0);
    vScreenUV = vec2(pos.x, 1.0 - pos.y);
}
