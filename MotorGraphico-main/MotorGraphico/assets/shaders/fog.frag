#version 330 core

// Pase de post-procesado de niebla de guerra (Fase 4). Multiplica la escena
// por la textura de niebla: donde la niebla es negra (Hidden) la escena queda
// oculta, gris (Explored) se ve tenue, blanca (Visible) a luz plena.
//
// uFogColor tinta la zona oculta/explorada en vez de dejarla negra pura: si
// vale (0,0,0) el resultado es identico a "scene * fogValue"; con un tono
// azulado (p.ej. 0.05,0.07,0.12) las zonas ocultas se ven de ese color en
// vez de negro absoluto, mas estilizado.
in vec2 vScreenUV;
out vec4 fragColor;

uniform sampler2D uScene;   // textura del FBO (slot 0)
uniform sampler2D uFog;     // textura de niebla de FogOfWar (slot 1)
uniform vec3 uFogColor;     // tinte de la zona oculta, ej. (0.05,0.07,0.12)

void main() {
    vec3 scene = texture(uScene, vScreenUV).rgb;
    float fogValue = texture(uFog, vScreenUV).r;  // niebla en escala de grises
    // Donde la niebla es negra, mezclar hacia uFogColor (no multiplicar a 0):
    // asi la zona oculta muestra el tinte en vez de negro puro, y la zona
    // visible (fogValue=1) muestra la escena tal cual.
    vec3 result = mix(uFogColor, scene, fogValue);
    fragColor = vec4(clamp(result, 0.0, 1.0), 1.0);
}
