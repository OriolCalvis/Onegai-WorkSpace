#version 330 core

in vec2 vUV;
in vec4 vColor;
out vec4 fragColor;

uniform sampler2D uTexture;

// Multiplica el texel por el tint del vertice: con vColor = blanco opaco
// (por defecto) el resultado es identico a "solo textura", asi que los
// demos existentes no cambian. El tint lo usa Entity para colorear/sombrear
// sprites individuales.
void main() { fragColor = texture(uTexture, vUV) * vColor; }
