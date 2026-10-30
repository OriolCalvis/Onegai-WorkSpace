# Revisión de errores antes de subir a GitHub — 14/09/2026

Base reproducida: `47111d1`, rama `feature/editor-s1-contratos`. La revisión se ejecutó en una copia de los archivos versionados y después con las correcciones de esta entrega. Una segunda copia independiente permitió revisar también el trabajo sin commit, sin incorporarlo al candidato.

Destino de la subida solicitada: `https://github.com/OriolCalvis/Onegai-WorkSpace.git`, misma rama. No se integra en `main`.

## Corregido y comprobado

| Hallazgo reproducido | Causa y corrección |
|---|---|
| `test_object_catalog` fallaba en `tendes_buides == 0` | Confundía PNJ con comerciante. Se comprueba que existen PNJ sin tienda y se conserva la prueba independiente de inventarios/referencias de las tiendas reales. El round-trip de `ObjectCatalog::exportJson()` sigue probado en el candidato. |
| `test_skill_effect_real_assets` exigía magnitud positiva | Elegía el primer CON del catálogo, que era `atletismo`, una habilidad narrativa con magnitud cero. El caso nuevo selecciona por ID `abrazo_de_brasas` y `atletismo`, controla los dados y exige respectivamente 4/12 y 0/0 en éxito/crítico. No cambia las reglas del motor ni los datos de las cartas. |
| `rpg_declaration_check` decía que faltaba `editorActionDefinitions` | La definición existe; el analizador no reconocía la coma del retorno `std::array<ActionDefinition, 14>`. Ahora admite comas y saltos de línea en el tipo. Regresiones positivas y negativa: una declaración sin definición sigue fallando. |
| `validar_todo.py` señalaba `ejemplos_objetos.json` fuera del mundo | La galería pertenece deliberadamente al proyecto Ejemplos, separado de Boundington por DEC-05. Se añade a las exclusiones explícitas del validador del mundo. Un nivel desconocido con el mismo prefijo y una puerta rota siguen produciendo error. |
| `test_application` bloqueaba la batería sin gráficos | Usa `Application::init()`, que crea una ventana OpenGL. Se etiqueta `gl`, se limita a 60 segundos y se ejecuta en serie. El workflow de Linux la incluye bajo xvfb, también en su paso ASan/UBSan. No se elimina la prueba. |

## Verificación del candidato

- Configuración y compilación completa Debug en macOS, con GLM/GLFW locales y sin descargar dependencias.
- **106/106 pruebas CTest sin gráficos**, incluidas las seis nuevas regresiones Python de validadores.
- **10/10 pruebas CTest gráficas** con sesión de macOS: Application, cierre/escenas, composición y demos de postprocesado, editor a escalas 1 y 2.
- **19/19 regresiones Python de controles** y verificador del shell: 14 acciones / 15 atajos.
- `validar_todo.py`: **0 errores, 14 avisos**. Validador del mundo: 93 niveles alcanzables y 191 puertas.
- C-22: 382 declaraciones comprobadas en 50 cabeceras; queda la deuda conocida de `ObjectInspector::confirmar`.

La prueba gráfica de Application pasó tras darle acceso a una sesión gráfica; su bloqueo inicial dentro del sandbox no era un fallo de la lógica de aplicación. Las pruebas de esta revisión son Debug/macOS: no equivalen a haber ejecutado la matriz remota Linux/Windows/Release ni sanitizadores. La configuración de CI queda preparada para ello.

## Abierto en el trabajo sin commit

Los siguientes resultados corresponden a una fotografía del árbol de trabajo concurrente del 14/09. No son errores del candidato validado ni una afirmación sobre futuras ediciones de los otros agentes.

| Zona | Resultado de la copia del trabajo en curso | Siguiente paso |
|---|---|---|
| `src/Engine/Application.cpp`, pantallas GDB-12 | Diez errores de compilación por uso de `kMenuPadding`, `kPanelColor` y `kBorderColor` sin declaración visible, alrededor de las líneas 2148–2210. Los veinte errores antiguos por métodos ausentes ya no se reproducen en esta fotografía. | El agente que integra las pantallas debe resolver el alcance de esas constantes y volver a compilar el lote completo. |
| `test_event_editor`, E-15 | El fuente ya compila, pero el enlazador no encuentra `main`. | Completar el ejecutable de pruebas y su registro antes de versionar el paquete. |
| `AssetBrowserState`, E-13 | Quince declaraciones sin definición en el instante de la copia; paquete reservado y en desarrollo por qwen. | Terminar implementación y pruebas dentro de esa reserva. |
| Respuestas y planificación | Hay cambios de otros agentes y respuestas de Oriol sin commit. | Cada propietario debe validar/versionar su lote; no se añaden en bloque a esta subida. |

## Avisos y decisiones que permanecen

- Los 14 avisos del validador son 12 referencias de cartas a deidades/monstruo no presentes y dos proyectos editables sin entrada jugable (`mundo`, `este_sur`). La corrección de referencias de cartas corresponde aguas arriba; no se inventa contenido en el motor.
- Los 13 desembarcos del mapamundi no alcanzables andando siguen aplazados a viajar entre naciones. El grafo de puertas y el recorrido a pie comprueban cosas distintas.
- `ObjectInspector::confirmar` sigue pendiente de E-12/E-14. La antigua entrada conocida de `exportJson` ya está resuelta; su limpieza existe en el trabajo concurrente y se conserva sin incorporarla aquí.
- E-10/E-12 siguen abiertos para las herramientas y formularios restantes, según sus seguimientos. No se cambian las decisiones de producto del cuaderno de Oriol.

## Coordinación de Git

El parche publicable contiene únicamente estas correcciones, sus pruebas y este informe. Se contrasta por SHA-256 con el snapshot probado y se registra mediante `herramientas/git-safe.sh` con un índice privado. Las seis rutas que otros agentes tenían preparadas en el índice se conservan con los mismos blobs.

En los archivos de pruebas compartidos se conserva además la variante de trabajo de los otros agentes: el candidato mantiene el round-trip existente y sustituye solo el caso defectuoso sobre cartas; la nueva regresión por ID también se añade al árbol de trabajo. No se publican los cambios concurrentes de catálogos, pantallas, navegador ni del editor de eventos.
