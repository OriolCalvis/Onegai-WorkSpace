#pragma once

#include "Core/Errors/Result.h"

#include <string>

// U3-07 — Interpolacion pura (sin GL). Sala limpia: matematica estandar.
namespace Interpolacion {

// Progreso temporal normalizado en [0,1].
//  - transcurrido o duracion NaN/inf, o duracion < 0 -> Error.
//  - duracion == 0 -> Ok(1.0) sea cual sea el signo de transcurrido (fin inmediato).
//  - duracion > 0 -> clamp(transcurrido / duracion, 0, 1).
Result<double> progreso(double transcurrido, double duracion);

// Catalogo de curvas (decision D2). Nombres en datos: ver curvaDesdeTexto.
enum class TipoCurva {
    Lineal,
    EntradaCuadratica,
    SalidaCuadratica,
    EntradaSalidaCuadratica,
    EntradaCubica,
    SalidaCubica,
    EntradaSalidaCubica
};

// Easing de t. t se acota a [0,1]; NaN -> 0.0. f(0)=0 y f(1)=1 exactos; monotona.
//  Lineal t | EntradaCuad t^2 | SalidaCuad 1-(1-t)^2
//  EntradaSalidaCuad t<0.5 ? 2t^2 : 1-(-2t+2)^2/2   (= Camera.cpp:22)
//  EntradaCubica t^3 | SalidaCubica 1-(1-t)^3        (= Camera.cpp:24)
//  EntradaSalidaCubica t<0.5 ? 4t^3 : 1-(-2t+2)^3/2
double curva(TipoCurva tipo, double t);

// Parser para datos. Textos exactos en minusculas sin tildes: lineal,
// entrada_cuadratica, salida_cuadratica, entrada_salida_cuadratica,
// entrada_cubica, salida_cubica, entrada_salida_cubica. Otro texto -> Error.
Result<TipoCurva> curvaDesdeTexto(const std::string& texto);

// Mezcla escalar entre a y b con progreso p (decision D4: total, sin Result).
//  - p se acota a [0,1]. p<=0 -> a exacto; p>=1 -> b exacto; a==b -> a exacto.
//  - Interior: a + p*(b-a) acotado a [min(a,b), max(a,b)], monotono en p (si b-a desborda
//    se reparte en mitades). Las guardes de p van antes de a==b (p>=1 -> b con su signo).
//    Error <= ~4 ulp de max(|a|,|b|); los tests usan tolerancia 1e-12 relativa a esa magnitud.
//  - Descendente (b<a) permitido. Entradas no finitas (a, b o p NaN/inf): resultado
//    indefinido por contrato; el llamador valida antes.
double tweenEscalar(double a, double b, double p);

}  // namespace Interpolacion
