// PERGAMINO-01 — ver include/Game/AuraMoral.h.
#include "Game/AuraMoral.h"

#include <algorithm>
#include <cmath>

#include "Core/Imagen/ImagenRGBA.h"
#include "Game/Personajes/MontajePersonajes.h"

namespace Game {

namespace {
constexpr float kPi = 3.14159265358979f;

// Numero pseudoaleatorio estable por particula (sin estado).
float semilla(int i, int canal) {
    const float s = std::sin(static_cast<float>(i * 127 + canal * 311) * 12.9898f) * 43758.5453f;
    return s - std::floor(s);
}

// 0..1 a lo largo de los grados [desde, hasta] (los dos incluidos).
float paso(int g, int desde, int hasta) {
    return hasta == desde ? 1.0f : static_cast<float>(g - desde) / static_cast<float>(hasta - desde);
}

float mezcla(float a, float b, float t) { return a + (b - a) * t; }
}  // namespace

TramoMoral tramoMoral(int valor) { return moral::tramoDe(valor); }

int gradoAura(int valorMoral) {
    const int v = std::clamp(valorMoral, -100, 100);
    const int a = v < 0 ? -v : v;
    int g = 0;
    for (int i = 0; i < kGradosPorLado; ++i)
        if (a >= kCorteGrado[i]) g = i + 1;
    return v < 0 ? -g : g;
}

int hojasPorAla(int nivel) {
    static const int kHojas[kNivelesAla] = {1, 1, 2, 2, 4, 6, 8, 8};
    return nivel >= 1 && nivel <= kNivelesAla ? kHojas[nivel - 1] : 0;
}

int filaAla(Alas lado, int nivel) {
    if (lado == Alas::Ninguna || nivel < 1 || nivel > kNivelesAla) return -1;
    return (lado == Alas::Celestiales ? 0 : kNivelesAla) + nivel - 1;
}

int frameAla(float segundos) {
    const int f = static_cast<int>(std::floor(segundos * 6.0f));
    return ((f % kFramesAla) + kFramesAla) % kFramesAla;
}

float bateAla(float segundos) { return 5.0f * std::sin(segundos * 2.0f * kPi / 2.6f); }

int alasEnSprite(const AspectoAura& aspecto, int frameSprite, float segundos, AlaEnSprite (&out)[2],
                 Reparto reparto) {
    const int fila = filaAla(aspecto.alas, aspecto.nivelAlas);
    if (!aspecto.visible || fila < 0) return 0;
    // Cada ala: el pivote (el hombro), cuanto se encoge de ancho, si va hacia
    // la izquierda (espejo) y su opacidad relativa.
    struct Pieza {
        float x, y, ancho;
        bool espejo;
        float alfa;
    };
    Pieza p[2];
    const Rumbo rumbo = rumboDeFrame(frameSprite, reparto);
    if (reparto == Reparto::Diagonal) {
        // PL-JUEGO-01: Arriba = noreste, Derecha = sureste, Abajo = suroeste, Izquierda = noroeste. La
        // cercana es la de la izquierda de la pantalla mirando al sureste y al noroeste, y la de la derecha
        // mirando al suroeste y al noreste.
        const float hx = kHombroDiagonalX, hy = kHombroDiagonalY;
        const bool cercaIzquierda = rumbo == Rumbo::Derecha || rumbo == Rumbo::Izquierda;
        if (cercaIzquierda) {
            p[0] = {hx + 2.0f, hy - 2.0f, 0.55f, false, 0.8f};  // la lejana, a la derecha
            p[1] = {hx - 1.0f, hy, 0.88f, true, 1.0f};          // la cercana, a la izquierda
        } else {
            p[0] = {hx - 2.0f, hy - 2.0f, 0.55f, true, 0.8f};
            p[1] = {hx + 1.0f, hy, 0.88f, false, 1.0f};
        }
    } else switch (rumbo) {
        case Rumbo::Derecha:  // mira a la derecha: las alas van hacia atras (la izquierda)
            p[0] = {kHombroX - 2.0f, kHombroY - 3.0f, 0.55f, true, 0.8f};
            p[1] = {kHombroX - 4.0f, kHombroY, 0.70f, true, 1.0f};
            break;
        case Rumbo::Izquierda:
            p[0] = {kHombroX + 2.0f, kHombroY - 3.0f, 0.55f, false, 0.8f};
            p[1] = {kHombroX + 4.0f, kHombroY, 0.70f, false, 1.0f};
            break;
        default:  // de frente o de espaldas: una a cada lado
            p[0] = {kHombroX - kSepAla, kHombroY, 1.0f, true, 1.0f};
            p[1] = {kHombroX + kSepAla, kHombroY, 1.0f, false, 1.0f};
            break;
    }
    const float celda = static_cast<float>(kCeldaAla);
    const float px = static_cast<float>(kPivoteAlaX), py = static_cast<float>(kPivoteAlaY);
    const float bate = bateAla(segundos) * kPi / 180.0f;
    for (int i = 0; i < 2; ++i) {
        AlaEnSprite& a = out[i];
        // Las esquinas respecto al hombro: la celda del ala derecha, o su espejo.
        float x0 = p[i].espejo ? -(celda - px) : -px;
        float x1 = p[i].espejo ? px : celda - px;
        x0 *= p[i].ancho;
        x1 *= p[i].ancho;
        const float y0 = -py, y1 = celda - py;
        const float ex[4] = {x0, x1, x1, x0}, ey[4] = {y0, y0, y1, y1};
        // Levantar el ala es subir la punta: hacia arriba (y negativa) por fuera.
        const float giro = p[i].espejo ? bate : -bate;
        const float c = std::cos(giro), s = std::sin(giro);
        for (int k = 0; k < 4; ++k) {
            a.x[k] = p[i].x + ex[k] * c - ey[k] * s;
            a.y[k] = p[i].y + ex[k] * s + ey[k] * c;
        }
        a.espejo = p[i].espejo;
        // de espaldas: en el clasico, mirando arriba; en diagonal, al noreste y al noroeste
        a.delante = reparto == Reparto::Diagonal ? (rumbo == Rumbo::Arriba || rumbo == Rumbo::Izquierda)
                                                 : rumbo == Rumbo::Arriba;
        a.alfa = std::clamp(aspecto.alfaAlas * p[i].alfa, 0.0f, 1.0f);
        a.fila = fila;
        a.columna = frameAla(segundos);
        a.hombroX = p[i].x;
        a.hombroY = p[i].y;
        a.ancho = p[i].ancho;
        a.giro = giro;
    }
    return 2;
}

void celdaASprite(const AlaEnSprite& ala, float cx, float cy, float& sx, float& sy) {
    // Las mismas cuentas que las esquinas en alasEnSprite: respecto al hombro, espejo, encoger y batir.
    float ex = cx - static_cast<float>(kPivoteAlaX);
    if (ala.espejo) ex = -ex;
    ex *= ala.ancho;
    const float ey = cy - static_cast<float>(kPivoteAlaY);
    const float c = std::cos(ala.giro), s = std::sin(ala.giro);
    sx = ala.hombroX + ex * c - ey * s;
    sy = ala.hombroY + ex * s + ey * c;
}

std::vector<PuntoAla> puntosDeAla(const imagen::ImagenRGBA& atlas, int fila) {
    std::vector<PuntoAla> out;
    const int c = kCeldaAla;
    if (fila < 0 || atlas.ancho < c || atlas.alto < (fila + 1) * c ||
        atlas.pixeles.size() < static_cast<std::size_t>(atlas.ancho) * static_cast<std::size_t>(atlas.alto) * 4)
        return out;
    // La mitad de fuera del ala: lo que queda mas lejos del hombro que el 45 % del pixel mas lejano.
    const float px = static_cast<float>(kPivoteAlaX), py = static_cast<float>(kPivoteAlaY);
    float lejos = 0.0f;
    for (int y = 0; y < c; ++y)
        for (int x = 0; x < c; ++x)
            if (atlas.pixel(x, fila * c + y)[3] > 0)
                lejos = std::max(lejos, std::hypot(static_cast<float>(x) - px, static_cast<float>(y) - py));
    for (int y = 0; y < c; ++y)
        for (int x = 0; x < c; ++x)
            if (atlas.pixel(x, fila * c + y)[3] > 0 &&
                std::hypot(static_cast<float>(x) - px, static_cast<float>(y) - py) > 0.45f * lejos)
                out.push_back(PuntoAla{static_cast<float>(x) + 0.5f, static_cast<float>(y) + 0.5f});
    return out;
}

int chispasPorAla(int nivel) { return nivel >= 1 && nivel <= kNivelesAla ? 4 + 3 * nivel : 0; }

ChispaAla chispaAla(const AspectoAura& aspecto, int indice, float segundos, const PuntoAla* puntos, int nPuntos) {
    ChispaAla c;
    if (!aspecto.visible || puntos == nullptr || nPuntos <= 0 || indice < 0 ||
        indice >= chispasPorAla(aspecto.nivelAlas))
        return c;
    // Cada chispa vive entre 1,2 y 2,1 s, con su fase; en cada vida, otra pluma.
    const float periodo = 1.2f + 0.9f * semilla(indice, 11);
    const float u = segundos / periodo + semilla(indice, 12);
    const float vuelta = std::floor(u);
    const float t = u - vuelta;
    const int vida = static_cast<int>(std::fmod(vuelta, 997.0f) + (vuelta < 0.0f ? 997.0f : 0.0f));
    const int k = std::min(nPuntos - 1, static_cast<int>(semilla(indice * 37 + vida, 13) * static_cast<float>(nPuntos)));
    const PuntoAla& p = puntos[k];
    // Sale del hombro hacia fuera por la pluma y sube un poco (la energia se va).
    const float dx = p.x - static_cast<float>(kPivoteAlaX), dy = p.y - static_cast<float>(kPivoteAlaY);
    const float d = std::max(1.0f, std::sqrt(dx * dx + dy * dy));
    const float sale = 5.0f + 9.0f * semilla(indice * 37 + vida, 14);
    c.x = p.x + dx / d * sale * t;
    c.y = p.y + dy / d * sale * t - 8.0f * t;
    c.alfa = std::clamp(std::sin(t * kPi), 0.0f, 1.0f);  // por la del ala (AlaEnSprite::alfa) al pintarla
    const float r = semilla(indice, 15);  // la mitad, de 2 px; algunas, de 3; el resto, de 1
    c.tam = r < 0.5f ? 2.0f : (r < 0.7f ? 3.0f : 1.0f);
    c.tono = indice % 3;
    return c;
}

AspectoAura aspectoAura(int valorMoral) {
    AspectoAura a;
    a.tramo = tramoMoral(valorMoral);
    a.grado = gradoAura(valorMoral);
    const int g = a.grado < 0 ? -a.grado : a.grado;
    if (g == 0) return a;
    a.visible = true;
    if (a.grado > 0) {
        a.luz = true;
        if (g <= 4) {  // Respetado: el halo tenue crece; desde el grado 3, motas de luz
            const float t = paso(g, 1, 4);
            a.intensidad = mezcla(0.30f, 0.54f, t);
            a.radio = mezcla(0.95f, 1.15f, t);
            a.color = Vector4{1.0f, 0.80f, 0.38f, 1.0f};
            a.particulas = g >= 3 ? g - 1 : 0;
        } else if (g <= 7) {  // Salvador: halo grande y, desde el grado 6, rayos de luz
            const float t = paso(g, 5, 7);
            a.intensidad = mezcla(0.60f, 0.72f, t);
            a.radio = mezcla(1.30f, 1.50f, t);
            a.color = Vector4{1.0f, 0.83f, 0.40f, 1.0f};
            a.particulas = g - 1;
            a.rayos = g == 5 ? 0 : (g == 6 ? 4 : 6);
        } else {  // Héroe: las alas celestiales, de 1 a 8 niveles, y motas de luz
            a.nivelAlas = g - 7;
            const float t = paso(a.nivelAlas, 1, kNivelesAla);
            a.intensidad = mezcla(0.74f, 1.0f, t);
            a.radio = mezcla(1.55f, 1.90f, t);
            a.color = Vector4{1.0f, 0.86f, 0.46f, 1.0f};
            a.particulas = 6 + a.nivelAlas;
            a.motas = true;
            a.alas = Alas::Celestiales;
            a.alfaAlas = mezcla(0.70f, 0.92f, t);
        }
        return a;
    }
    if (g <= 4) {  // Canalla: humo rojo oscuro a los pies, cada vez mas
        const float t = paso(g, 1, 4);
        a.intensidad = mezcla(0.36f, 0.60f, t);
        a.radio = mezcla(0.95f, 1.15f, t);
        a.color = Vector4{0.33f, 0.05f, 0.04f, 1.0f};
        a.particulas = 2 + g;
        const float s = mezcla(0.95f, 0.88f, t);
        a.tinteSprite = Vector4{mezcla(0.97f, 0.94f, t), s, s, 1.0f};
    } else if (g <= 7) {  // Desgraciado: humo espeso, el sprite en sombra y rayos de sombra
        const float t = paso(g, 5, 7);
        a.intensidad = mezcla(0.70f, 0.84f, t);
        a.radio = mezcla(1.25f, 1.40f, t);
        a.color = Vector4{0.24f, 0.02f, 0.02f, 1.0f};
        a.particulas = g + 2;
        a.rayos = g == 5 ? 0 : (g == 6 ? 4 : 6);
        const float s = mezcla(0.66f, 0.58f, t);
        a.tinteSprite = Vector4{mezcla(0.80f, 0.74f, t), s, s, 1.0f};
    } else {  // Villano: halo oscuro, las alas demoníacas de 1 a 8 niveles y motas de sombra
        a.nivelAlas = g - 7;
        const float t = paso(a.nivelAlas, 1, kNivelesAla);
        a.intensidad = mezcla(0.82f, 1.0f, t);
        a.radio = mezcla(1.50f, 1.85f, t);
        a.color = Vector4{0.14f, 0.01f, 0.04f, 1.0f};
        a.particulas = 6 + a.nivelAlas;
        a.motas = true;
        a.alas = Alas::Demoniacas;
        a.alfaAlas = mezcla(0.80f, 0.96f, t);
        const float s = mezcla(0.62f, 0.54f, t);
        a.tinteSprite = Vector4{mezcla(0.76f, 0.70f, t), s, mezcla(0.64f, 0.58f, t), 1.0f};
    }
    return a;
}

float pulsoAura(float segundos) { return 0.925f + 0.075f * std::sin(segundos * 2.0f * kPi / 2.4f); }

ParticulaAura particulaAura(const AspectoAura& aspecto, int indice, float segundos) {
    ParticulaAura p;
    if (!aspecto.visible || indice < 0 || indice >= aspecto.particulas) return p;
    if (aspecto.luz || aspecto.motas) {
        // Motas (de luz o de sombra): nacen en el halo, suben y se apagan;
        // cada una con su periodo y su fase para que no suban en fila. Con
        // alas, mas altas y mas abiertas (salen tambien de las alas).
        const float periodo = 1.6f + 0.9f * semilla(indice, 1);
        const float t = std::fmod(segundos / periodo + semilla(indice, 2), 1.0f);
        const float abre = aspecto.nivelAlas > 0 ? 0.7f + 0.12f * static_cast<float>(aspecto.nivelAlas) : 0.7f;
        p.dx = (semilla(indice, 3) - 0.5f) * abre * std::min(aspecto.radio, 1.3f);
        p.dy = -0.05f - t * (aspecto.nivelAlas > 0 ? 1.4f : 1.1f);
        p.radio = 0.05f + 0.03f * semilla(indice, 4);
        p.alfa = aspecto.intensidad * std::sin(t * kPi);
        return p;
    }
    // Volutas de humo: giran despacio alrededor de los pies y se levantan
    // un poco antes de deshacerse.
    const float periodo = 2.8f + 1.4f * semilla(indice, 1);
    const float t = std::fmod(segundos / periodo + semilla(indice, 2), 1.0f);
    const float giro = semilla(indice, 3) * 2.0f * kPi + segundos * 0.6f;
    p.dx = std::cos(giro) * 0.32f * aspecto.radio;
    p.dy = std::sin(giro) * 0.12f * aspecto.radio - t * 0.35f;
    p.radio = (0.22f + 0.18f * t) * aspecto.radio;
    p.alfa = aspecto.intensidad * 0.55f * std::sin(t * kPi);
    return p;
}

RayoAura rayoAura(const AspectoAura& aspecto, int indice, float segundos) {
    RayoAura r;
    if (!aspecto.visible || indice < 0 || indice >= aspecto.rayos) return r;
    // Repartidos en circulo desde el pecho, girando despacio (una vuelta cada
    // 40 s); cada uno late con su fase. Los de sombra, mas finos y mas largos.
    const bool sombra = !aspecto.luz;
    const float giro = segundos * 2.0f * kPi / 40.0f;
    const float a = giro + (static_cast<float>(indice) + 0.5f * semilla(indice, 5)) * 2.0f * kPi /
                               static_cast<float>(aspecto.rayos);
    const float late = 0.85f + 0.15f * std::sin(segundos * 2.0f * kPi / 1.9f + semilla(indice, 6) * 6.0f);
    const float largo = (sombra ? 0.78f : 0.66f) * late * (aspecto.rayos >= 6 ? 1.1f : 1.0f);
    const float anchoBase = sombra ? 0.045f : 0.07f, anchoPunta = sombra ? 0.004f : 0.012f;
    const float dx = std::cos(a), dy = std::sin(a);
    const float nx = -dy, ny = dx;
    const float tx = dx * largo, ty = dy * largo;
    // arriba-izquierda, arriba-derecha, abajo-derecha, abajo-izquierda
    r.x[0] = nx * anchoBase;  r.y[0] = ny * anchoBase;
    r.x[1] = tx + nx * anchoPunta; r.y[1] = ty + ny * anchoPunta;
    r.x[2] = tx - nx * anchoPunta; r.y[2] = ty - ny * anchoPunta;
    r.x[3] = -nx * anchoBase; r.y[3] = -ny * anchoBase;
    r.alfa = std::clamp(aspecto.intensidad * (sombra ? 0.75f : 0.55f) * late, 0.0f, 1.0f);
    return r;
}

}  // namespace Game
