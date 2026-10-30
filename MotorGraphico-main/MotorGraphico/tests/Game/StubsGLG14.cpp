// G-14 — stubs solo para los símbolos glad/gl de generateAtlas que BitmapFont.cpp
// referencia. El resto (~Texture, SpriteBatch::submit) los resuelve macOS con
// -Wl,-undefined,dynamic_lookup (nunca se ejecutan en un runner gl-free).

extern "C" {
void glad_glBindTexture(unsigned int target, unsigned int texture) { (void)target; (void)texture; }
void glad_glGenTextures(int n, unsigned int* textures) { if (textures) for (int i=0;i<n;++i) textures[i]=0; }
void glad_glTexImage2D(unsigned int,int,int,int,int,int,unsigned int,unsigned int,const void*) {}
void glad_glTexParameteri(unsigned int target, unsigned int pname, int param) { (void)target; (void)pname; (void)param; }
void glad_glTexParameterf(unsigned int target, unsigned int pname, float param) { (void)target; (void)pname; (void)param; }
void glad_glDeleteTextures(int n, const unsigned int* textures) { (void)n; (void)textures; }
}
