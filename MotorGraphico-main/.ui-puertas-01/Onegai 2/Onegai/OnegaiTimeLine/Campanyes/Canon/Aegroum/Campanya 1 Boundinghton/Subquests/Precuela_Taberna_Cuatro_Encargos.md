---
title: "Precuela de Boundington: La Taberna y los Cuatro Encargos"
tags: [boundington, precuela, videojuego, perdidos, diseno-narrativo]
estado: borrador-implementado
cronologia: "Antes de la llegada de Venides a Boundington"
---

# Precuela de Boundington: La Taberna y los Cuatro Encargos

Esta aventura es una **precuela jugable en solitario** de [[Campanya 1 Boundington - Estructura]]. Cuatro aventureros de la partida de rol llegan a la Taberna Humilde antes de que Venides entre en la ciudad. En el videojuego se conserva su llegada como trasfondo, pero el jugador controla a un único protagonista.

No sustituye la campaña canónica de Venides ni adelanta sus acontecimientos: funciona como una capa de historia local que explica por qué Boundington ya está herida y llena de rumores cuando comienza la campaña principal.

## Bucle de juego

La Taberna Humilde es el hub. Desde allí se abren tres hilos, en cualquier orden: hablar con [[Luisarda]] sobre los Perdidos; seguir a Ben Kafka hacia el bosque por los niños desaparecidos; y ayudar a Griffin, limpiador de desagües, ante la invasión naga.

Cada hilo deja una pista persistente. Al reunir las tres, el jugador comprende que las amenazas no son sucesos aislados: algo intenta devolver a Boundington a un poder antiguo.

## Encargo I — Luisarda y los Perdidos

Luisarda conoce criaturas pequeñas que se burlan, insultan y roban cuando la víctima se distrae. Imitan a un duende, pero no lo son: visten como payasos y usan máscaras de porcelana. Sirven a Los Perdidos y preparan el retorno de su antiguo creador.

**Resultado:** `pre_bnd_pista_perdidos` habilita el cierre de la precuela y deja una pista enlazable con la campaña principal.

## Encargo II — Ben Kafka, el bosque y los niños

Ben Kafka pide ayuda para buscar a varios niños, pero oculta parte de lo que sabe y plantea preguntas morales durante el trayecto. El antagonista es una Sàga surgida de un antiguo draug: cuerpo deformado por musgo, setas y materia vegetal; su brazo derecho es una columna que termina en la cabeza descompuesta de un lobo. Puede sacrificar a los niños si el jugador fracasa o actúa tarde.

**Resultado:** éxito: `pre_bnd_ninos_rescatados`; consecuencia trágica: `pre_bnd_ninos_sacrificados`. Ambos estados son persistentes para que el mundo pueda reaccionar después.

> Pendiente de fijación canónica: la identidad exacta del niño relacionado con los Kafka y la taxonomía definitiva Sàga/draug. El videojuego lo mantiene como misterio hasta cerrar esa decisión.

## Encargo III — Griffin y los desagües

Griffin detecta nagas en los conductos. El hilo abre el subsuelo de la ciudad, ofrece exploración alternativa y revela que la corrupción alcanza su infraestructura.

**Resultado:** `pre_bnd_desagues_limpios` deja un acceso seguro y contribuye a la investigación común.

## Adaptación al motor

La versión implementada está en `MotorGraphico/assets/adventures/boundington_precuela_taberna.json`. Usa `talk` para los encargos, `enter` para Taberna/Bosque/Desagües, flags persistentes como única fuente de verdad, objetivos derivados de esas flags y un cierre automático cuando los tres hilos se resuelven.

`demo_precuela_boundington` recorre una ruta de rescate y otra trágica, para mantener este documento sincronizado con el contenido de juego.

## Enlaces

- [[Campanya 1 Boundington - Estructura]]
- [[Cap03_Boundington_y_Los_Perdidos]]
- [[Luisarda]]
- [[00_Narrativa_Emergent]]
