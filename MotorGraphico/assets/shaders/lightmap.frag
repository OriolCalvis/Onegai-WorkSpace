#version 330 core

// Pase de post-procesado de iluminacion (Fase 4). Combina la escena
// (renderizada a un FBO por IsometricRenderer) con un lightmap, modelo
// multiplicativo simple: oscura donde el lightmap es oscuro, no cambia
// donde es blanco. uAmbient es la luz minima global (vec3) para que las
// zonas sin lightmap no caigan a negro puro.
in vec2 vScreenUV;
out vec4 fragColor;

uniform sampler2D uScene;     // textura del FBO (slot 0)
uniform sampler2D uLightmap;  // textura de luz alineada al mundo (slot 1)
uniform vec3 uAmbient;        // luz ambiente RGB en [0,1], ej. (0.35,0.35,0.4)

void main() {
    vec3 scene = texture(uScene, vScreenUV).rgb;
    vec3 light = texture(uLightmap, vScreenUV).rgb;
    // max() contra ambient: la luz nunca baja del ambiente (no hay zonas
    // totalmente a oscuras salvo que ambient sea ~0). clamp final por
    // seguridad.
    vec3 lit = scene * max(light, uAmbient);
    fragColor = vec4(clamp(lit, 0.0, 1.0), 1.0);
}
