#version 330 core

// Pase de post-procesado de paletizado/LUT de color (Fase 4, tercer
// efecto). Remapea el color de la escena a traves de una LUT 3D
// (color-grading estandar) codificada como textura 2D: "uLutLevels"
// slices de uLutLevels x uLutLevels en una unica fila (ver
// ColorLUT::ColorLUT()). El canal azul de la escena selecciona el slice
// (que tile de la fila), rojo y verde son las coordenadas dentro del
// slice.
//
// A diferencia de lightmap/fog (que interpolan suave, GL_LINEAR), la LUT
// se samplea con GL_NEAREST y sin interpolar entre slices adyacentes: la
// cuantizacion ya viene hecha en la textura (ColorLUT::quantizeColor), y
// el punto del efecto es un posterizado NITIDO (paleta limitada, estilo
// pixel art -- ver motor_grafico_dafo.md), no un degradado de color
// grading. Por eso el indice de cada canal se redondea aqui con la MISMA
// formula que ColorLUT::quantizeChannel (round(v*(levels-1))): sin este
// redondeo explicito, el texel que caeria justo en el borde entre dos
// escalones podria samplear el vecino equivocado.
in vec2 vScreenUV;
out vec4 fragColor;

uniform sampler2D uScene;  // textura del FBO (slot 0)
uniform sampler2D uLut;    // textura de la LUT (slot 1)
uniform float uLutLevels;  // ColorLUT::levels(): resolucion del cubo

void main() {
    vec3 scene = clamp(texture(uScene, vScreenUV).rgb, 0.0, 1.0);

    float steps = uLutLevels - 1.0;
    float r = floor(scene.r * steps + 0.5);
    float g = floor(scene.g * steps + 0.5);
    float b = floor(scene.b * steps + 0.5);

    // UV del texel exacto: tile "b" ocupa las columnas
    // [b*uLutLevels, b*uLutLevels + uLutLevels) de la textura
    // (uLutLevels*uLutLevels de ancho x uLutLevels de alto); x=r dentro
    // del tile, y=g. +0.5 centra el muestreo en el texel (GL_NEAREST hace
    // el resto: sin interpolacion, texel exacto).
    float texelW = 1.0 / (uLutLevels * uLutLevels);
    float texelH = 1.0 / uLutLevels;
    vec2 uv = vec2((b * uLutLevels + r + 0.5) * texelW, (g + 0.5) * texelH);

    fragColor = vec4(texture(uLut, uv).rgb, 1.0);
}
