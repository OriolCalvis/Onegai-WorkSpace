#include "Core/Math/EstadoTween.h"

#include "TestHarness.h"

#include <cmath>
#include <limits>

using namespace Interpolacion;

namespace {

const double kNaN = std::numeric_limits<double>::quiet_NaN();
const double kInf = std::numeric_limits<double>::infinity();

EstadoTween nuevo(double duracion, double transcurrido = 0.0, TipoCurva c = TipoCurva::Lineal) {
    EstadoTween e;
    e.duracion = duracion;
    e.transcurrido = transcurrido;
    e.curva = c;
    return e;
}

bool igualEstado(const EstadoTween& a, const EstadoTween& b) {
    return a.duracion == b.duracion && a.transcurrido == b.transcurrido && a.curva == b.curva &&
           a.pausado == b.pausado && a.terminado == b.terminado;
}

MOTOR_TEST_CASE(avanza_a_mitad_no_termina) {
    const auto r = avanzaTween(nuevo(2.0), 1.0);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().estado.transcurrido == 1.0);
    MOTOR_CHECK(!r.value().estado.terminado && !r.value().acabaDeTerminar);
    MOTOR_CHECK(r.value().progreso == 0.5 && r.value().valorCurvado == 0.5);
}

MOTOR_TEST_CASE(usa_progreso_y_curva_de_interpolacion) {
    const auto r = avanzaTween(nuevo(1.0, 0.0, TipoCurva::EntradaCuadratica), 0.5);
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().progreso == progreso(0.5, 1.0).value());
    MOTOR_CHECK(r.value().valorCurvado == curva(TipoCurva::EntradaCuadratica, 0.5));
    MOTOR_CHECK(r.value().valorCurvado == 0.25);
}

MOTOR_TEST_CASE(cruza_el_final_emite_una_vez_y_fija_transcurrido) {
    const auto r = avanzaTween(nuevo(1.0, 0.75), 0.5);  // sobra 0.25
    MOTOR_CHECK(r.isOk());
    MOTOR_CHECK(r.value().acabaDeTerminar && r.value().estado.terminado);
    MOTOR_CHECK(r.value().estado.transcurrido == 1.0);
    MOTOR_CHECK(r.value().progreso == 1.0 && r.value().valorCurvado == 1.0);
}

MOTOR_TEST_CASE(llegar_exacto_al_final_termina) {
    const auto r = avanzaTween(nuevo(1.0, 0.5), 0.5);
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().estado.terminado);
}

MOTOR_TEST_CASE(ya_terminado_no_reemite_nunca) {
    EstadoTween e = nuevo(1.0, 1.0);
    e.terminado = true;
    for (double dt : {0.0, 0.5, 100.0}) {
        const auto r = avanzaTween(e, dt);
        MOTOR_CHECK(r.isOk());
        MOTOR_CHECK(!r.value().acabaDeTerminar);
        MOTOR_CHECK(igualEstado(r.value().estado, e));
        MOTOR_CHECK(r.value().progreso == 1.0);
    }
    EstadoTween z = nuevo(0.0);
    z.terminado = true;
    MOTOR_CHECK(!avanzaTween(z, 0.0).value().acabaDeTerminar);
}

MOTOR_TEST_CASE(encadenado_emite_exactamente_una_vez) {
    EstadoTween e = nuevo(1.0);
    int emisiones = 0;
    for (int i = 0; i < 20; ++i) {
        const auto r = avanzaTween(e, 0.25);
        MOTOR_CHECK(r.isOk());
        if (r.value().acabaDeTerminar) ++emisiones;
        e = r.value().estado;
    }
    MOTOR_CHECK(emisiones == 1 && e.terminado);
}

MOTOR_TEST_CASE(dt_cero_no_avanza_con_duracion_positiva) {
    EstadoTween e = nuevo(2.0, 0.5);
    const auto r = avanzaTween(e, 0.0);
    MOTOR_CHECK(r.isOk() && igualEstado(r.value().estado, e) && !r.value().acabaDeTerminar);
    MOTOR_CHECK(r.value().progreso == 0.25);
}

MOTOR_TEST_CASE(duracion_cero_termina_en_el_primer_avance_incluso_con_dt_cero) {
    // Como Camera::update con duracion 0 (CameraTransitionTests: update(0.0f) termina).
    for (double dt : {0.0, 0.016}) {
        const auto r = avanzaTween(nuevo(0.0), dt);
        MOTOR_CHECK(r.isOk());
        MOTOR_CHECK(r.value().acabaDeTerminar && r.value().estado.terminado);
        MOTOR_CHECK(r.value().progreso == 1.0 && r.value().valorCurvado == 1.0);
        const auto r2 = avanzaTween(r.value().estado, dt);
        MOTOR_CHECK(r2.isOk() && !r2.value().acabaDeTerminar);
    }
}

MOTOR_TEST_CASE(pausado_manda_sobre_todo) {
    EstadoTween e = nuevo(2.0, 0.5);
    e.pausado = true;
    const auto r = avanzaTween(e, 10.0);
    MOTOR_CHECK(r.isOk() && igualEstado(r.value().estado, e) && !r.value().acabaDeTerminar);
    EstadoTween z = nuevo(0.0);
    z.pausado = true;
    const auto rz = avanzaTween(z, 0.0);
    MOTOR_CHECK(rz.isOk() && !rz.value().estado.terminado && !rz.value().acabaDeTerminar);
    MOTOR_CHECK(rz.value().progreso == 1.0);  // duracion 0 -> progreso 1 (Interpolacion::progreso)
}

MOTOR_TEST_CASE(dt_invalido_es_error) {
    for (double dt : {-0.001, kNaN, kInf, -kInf}) {
        MOTOR_CHECK(!avanzaTween(nuevo(1.0), dt).isOk());
    }
    EstadoTween p = nuevo(1.0);
    p.pausado = true;  // se valida igualmente
    MOTOR_CHECK(!avanzaTween(p, -1.0).isOk());
}

MOTOR_TEST_CASE(estado_invalido_es_error_siempre) {
    // Decision: duracion/transcurrido negativos o no finitos -> Error, tambien pausado/terminado.
    const double malos[] = {-1.0, kNaN, kInf, -kInf};
    for (double m : malos) {
        MOTOR_CHECK(!avanzaTween(nuevo(m), 0.1).isOk());
        MOTOR_CHECK(!avanzaTween(nuevo(1.0, m), 0.1).isOk());
        EstadoTween p = nuevo(m);
        p.pausado = true;
        MOTOR_CHECK(!avanzaTween(p, 0.1).isOk());
        EstadoTween t = nuevo(1.0, m);
        t.terminado = true;
        MOTOR_CHECK(!avanzaTween(t, 0.0).isOk());
    }
    MOTOR_CHECK(!avanzaTween(nuevo(0.0, -0.5), 0.0).isOk());
}

MOTOR_TEST_CASE(particion_equivale_a_avanzar_de_una_vez) {
    const double duracion = 1.7;
    const int partes[] = {1, 2, 3, 7, 10, 100};
    for (int n : partes) {
        const double total = 1.0;  // no llega al final
        const auto una = avanzaTween(nuevo(duracion, 0.0, TipoCurva::EntradaSalidaCubica), total);
        EstadoTween e = nuevo(duracion, 0.0, TipoCurva::EntradaSalidaCubica);
        int emisiones = 0;
        PasoTween ult;
        for (int i = 0; i < n; ++i) {
            const auto r = avanzaTween(e, total / n);
            MOTOR_CHECK(r.isOk());
            if (r.value().acabaDeTerminar) ++emisiones;
            e = r.value().estado;
            ult = r.value();
        }
        MOTOR_CHECK(emisiones == 0 && !e.terminado && !una.value().acabaDeTerminar);
        MOTOR_CHECK(std::abs(ult.estado.transcurrido - una.value().estado.transcurrido) < 1e-12);
        MOTOR_CHECK(std::abs(ult.progreso - una.value().progreso) < 1e-12);
        MOTOR_CHECK(std::abs(ult.valorCurvado - una.value().valorCurvado) < 1e-12);
    }
}

MOTOR_TEST_CASE(particion_que_cruza_el_final_emite_una_vez_en_ambos_casos) {
    const double total = 2.5;  // sobrepasa duracion 1.7
    const int partes[] = {1, 2, 3, 7, 10, 100};
    for (int n : partes) {
        const auto una = avanzaTween(nuevo(1.7), total);
        MOTOR_CHECK(una.isOk() && una.value().acabaDeTerminar);
        EstadoTween e = nuevo(1.7);
        int emisiones = 0;
        for (int i = 0; i < n; ++i) {
            const auto r = avanzaTween(e, total / n);
            MOTOR_CHECK(r.isOk());
            if (r.value().acabaDeTerminar) ++emisiones;
            e = r.value().estado;
        }
        MOTOR_CHECK(emisiones == 1 && e.terminado);
        MOTOR_CHECK(std::abs(e.transcurrido - una.value().estado.transcurrido) < 1e-12);
        MOTOR_CHECK(e.transcurrido == 1.7);
    }
}

// Parte `total` en n trozos iguales; devuelve emisiones y deja el estado final en e.
int avanzaEnTrozos(EstadoTween& e, double total, int n) {
    int emisiones = 0;
    for (int i = 0; i < n; ++i) {
        const auto r = avanzaTween(e, total / n);
        MOTOR_CHECK(r.isOk());
        if (r.value().acabaDeTerminar) ++emisiones;
        e = r.value().estado;
    }
    return emisiones;
}

MOTOR_TEST_CASE(tolerancia_de_final_en_particiones_que_acumulan_por_debajo) {
    EstadoTween e = nuevo(1.0);
    // 10 x 0.1 acumula 0.9999999999999999: debe terminar en el ultimo trozo.
    EstadoTween diez = nuevo(1.0);
    int emisiones = 0;
    for (int i = 0; i < 10; ++i) {
        const auto r = avanzaTween(diez, 0.1);
        MOTOR_CHECK(r.isOk());
        MOTOR_CHECK(r.value().acabaDeTerminar == (i == 9));
        emisiones += r.value().acabaDeTerminar ? 1 : 0;
        diez = r.value().estado;
    }
    const auto una = avanzaTween(e, 1.0);
    MOTOR_CHECK(emisiones == 1 && diez.terminado && diez.transcurrido == 1.0);
    MOTOR_CHECK(igualEstado(diez, una.value().estado));
    // 7 x (1/7)
    EstadoTween siete = nuevo(1.0);
    MOTOR_CHECK(avanzaEnTrozos(siete, 1.0, 7) == 1);
    MOTOR_CHECK(igualEstado(siete, una.value().estado));
}

MOTOR_TEST_CASE(la_tolerancia_no_se_come_tiempo_real) {
    const auto r = avanzaTween(nuevo(1.0), 1.0 - 1e-6);  // margen 1e-6 > tolerancia
    MOTOR_CHECK(r.isOk() && !r.value().acabaDeTerminar && !r.value().estado.terminado);
}

MOTOR_TEST_CASE(dt_cero_en_el_borde_no_termina_ni_avanza) {
    for (double transcurrido : {1.0, 1.5}) {
        const EstadoTween e = nuevo(1.0, transcurrido);  // no terminado
        const auto r = avanzaTween(e, 0.0);
        MOTOR_CHECK(r.isOk() && igualEstado(r.value().estado, e) && !r.value().acabaDeTerminar);
        const auto s = avanzaTween(e, 0.001);  // termina en el siguiente avance con dt > 0
        MOTOR_CHECK(s.isOk() && s.value().acabaDeTerminar && s.value().estado.transcurrido == 1.0);
    }
}

MOTOR_TEST_CASE(dt_enorme_no_rompe) {
    const auto r = avanzaTween(nuevo(1.0), std::numeric_limits<double>::max());
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().estado.transcurrido == 1.0);
}

MOTOR_TEST_CASE(tolerancia_relativa_1e12_margen_absoluto) {
    // duracion 1e12 -> margen absoluto 1: 0.5 antes termina, 10 antes no.
    const auto cerca = avanzaTween(nuevo(1e12), 1e12 - 0.5);
    MOTOR_CHECK(cerca.isOk() && cerca.value().acabaDeTerminar);
    const auto lejos = avanzaTween(nuevo(1e12), 1e12 - 10.0);
    MOTOR_CHECK(lejos.isOk() && !lejos.value().acabaDeTerminar && !lejos.value().estado.terminado);
}

MOTOR_TEST_CASE(frontera_de_tolerancia_es_inclusiva) {
    const double umbral = 1.0 - 1e-12 * 1.0;  // duracion 1: producto exacto
    const auto en = avanzaTween(nuevo(1.0), umbral);  // 0 + umbral == umbral exacto
    MOTOR_CHECK(en.isOk() && en.value().acabaDeTerminar && en.value().estado.terminado);
    const auto debajo = avanzaTween(nuevo(1.0), std::nextafter(umbral, 0.0));
    MOTOR_CHECK(debajo.isOk() && !debajo.value().acabaDeTerminar && !debajo.value().estado.terminado);
}

MOTOR_TEST_CASE(cancela_mantener_actual_congela_valor) {
    const EstadoTween e = nuevo(2.0, 0.5, TipoCurva::EntradaCuadratica);
    const auto r = cancelaTween(e, PoliticaCancelacion::MantenerActual);
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().estado.terminado);
    MOTOR_CHECK(r.value().estado.transcurrido == 0.5);
    MOTOR_CHECK(r.value().progreso == 0.25 && r.value().valorCurvado == curva(TipoCurva::EntradaCuadratica, 0.25));
}

MOTOR_TEST_CASE(cancela_terminar_salta_al_final) {
    const auto r = cancelaTween(nuevo(2.0, 0.5, TipoCurva::EntradaCuadratica), PoliticaCancelacion::Terminar);
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().estado.terminado);
    MOTOR_CHECK(r.value().estado.transcurrido == 2.0);
    MOTOR_CHECK(r.value().progreso == 1.0 && r.value().valorCurvado == 1.0);
}

MOTOR_TEST_CASE(cancela_es_idempotente_y_emite_una_vez) {
    for (PoliticaCancelacion p : {PoliticaCancelacion::MantenerActual, PoliticaCancelacion::Terminar}) {
        const auto a = cancelaTween(nuevo(2.0, 0.5), p);
        MOTOR_CHECK(a.isOk() && a.value().acabaDeTerminar);
        const auto b = cancelaTween(a.value().estado, p);
        MOTOR_CHECK(b.isOk() && !b.value().acabaDeTerminar && igualEstado(b.value().estado, a.value().estado));
        // otra politica sobre un cancelado tampoco cambia nada
        const auto c = cancelaTween(a.value().estado, p == PoliticaCancelacion::Terminar ? PoliticaCancelacion::MantenerActual : PoliticaCancelacion::Terminar);
        MOTOR_CHECK(c.isOk() && !c.value().acabaDeTerminar && igualEstado(c.value().estado, a.value().estado));
        // y avanzar un cancelado no emite
        const auto d = avanzaTween(a.value().estado, 1.0);
        MOTOR_CHECK(d.isOk() && !d.value().acabaDeTerminar && igualEstado(d.value().estado, a.value().estado));
    }
}

MOTOR_TEST_CASE(cancela_terminado_por_avance_sin_efecto) {
    const auto fin = avanzaTween(nuevo(1.0), 2.0);
    for (PoliticaCancelacion p : {PoliticaCancelacion::MantenerActual, PoliticaCancelacion::Terminar}) {
        const auto r = cancelaTween(fin.value().estado, p);
        MOTOR_CHECK(r.isOk() && !r.value().acabaDeTerminar && igualEstado(r.value().estado, fin.value().estado));
        MOTOR_CHECK(r.value().progreso == 1.0);
    }
}

MOTOR_TEST_CASE(cancela_pausado_se_puede_y_conserva_pausa) {
    EstadoTween e = nuevo(2.0, 0.5);
    e.pausado = true;
    const auto r = cancelaTween(e, PoliticaCancelacion::Terminar);
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().estado.terminado);
    MOTOR_CHECK(r.value().estado.pausado && r.value().estado.transcurrido == 2.0);
}

MOTOR_TEST_CASE(cancela_duracion_cero) {
    const auto r = cancelaTween(nuevo(0.0), PoliticaCancelacion::MantenerActual);
    MOTOR_CHECK(r.isOk() && r.value().acabaDeTerminar && r.value().progreso == 1.0);
}

MOTOR_TEST_CASE(cancela_estado_invalido_es_error) {
    for (PoliticaCancelacion p : {PoliticaCancelacion::MantenerActual, PoliticaCancelacion::Terminar}) {
        MOTOR_CHECK(!cancelaTween(nuevo(kNaN), p).isOk());
        MOTOR_CHECK(!cancelaTween(nuevo(kInf), p).isOk());
        MOTOR_CHECK(!cancelaTween(nuevo(-1.0), p).isOk());
        MOTOR_CHECK(!cancelaTween(nuevo(1.0, kNaN), p).isOk());
        MOTOR_CHECK(!cancelaTween(nuevo(1.0, -0.5), p).isOk());
        EstadoTween t = nuevo(kNaN);
        t.terminado = true;  // se valida tambien terminado, como avanzaTween
        MOTOR_CHECK(!cancelaTween(t, p).isOk());
    }
}

}  // namespace

int main() { return motor_test::runAllTests(); }
