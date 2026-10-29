#pragma once

// MORAL-01 — Lo que una aventura apunta en la moral del héroe y la noche que
// lo asienta. GameSession::applyNarrative lo llama con lo que devuelve el
// motor narrativo (efecto "acto" y descanso largo), como hace con los objetos
// o las tiradas: el motor narrativo es puro y la moral es de la sesión.

#include <string>
#include <vector>

struct SessionData;

namespace moral {

// Los actos de un beat: pesan en la moral al dormir (con su motivo en el
// registro), la pureza baja al momento y la reputación se mueve con quien lo
// vio. Devuelve las líneas del registro.
std::vector<std::string> apuntaActos(SessionData& d, const std::vector<std::string>& ids);

// El descanso largo: asienta lo del día y cuenta cómo te deja.
std::vector<std::string> asientaNoche(SessionData& d);

// Los beats que piden un tramo («requires»: {"moral": ...}) leen la moral
// asentada del estado de la aventura.
void reflejaEnLaAventura(SessionData& d);

}  // namespace moral
