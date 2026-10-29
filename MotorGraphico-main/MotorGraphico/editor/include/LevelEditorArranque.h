// EST-21 (relevo B) — LAS DOS SALIDAS RAPIDAS DEL EDITOR.
//
// `--proyectos` lista lo que hay y `--nuevo <id>` crea uno; las dos
// imprimen y salen, sin abrir ventana. Por eso viven en la libreria
// GL-free y no dentro del main.

#pragma once

#include "LevelEditorOpciones.h"

namespace EditorNiveles {

// Codigo de salida del proceso: 0 si hizo lo que se le pidio.
int pantallaDeArranque(const EditorOptions& opts);

}  // namespace EditorNiveles
