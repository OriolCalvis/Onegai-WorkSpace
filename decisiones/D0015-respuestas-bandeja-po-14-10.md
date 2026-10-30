# D0015 — Las respuestas de Oriol a la bandeja del product owner (14/10/2026)

- **Estado:** `accepted` — Oriol contestó en `BANDEJA_PRODUCT_OWNER.md` el 14/10/2026 a las 09:01.
- **Lo apunta:** Claude (Cowork), con sus palabras, sin inventar lo que no dijo.
- **Lo que no contestó todavía** (lo verá a las 16:00): PO-01, PO-02, PO-25.4, PO-31, PO-33, PO-34, PO-37.

## El combate

1. **Los acompañantes tienen su propio turno en la iniciativa y los juega el jugador**, salvo que se
   diga lo contrario (PO-10). Como en BG3: tiran 1d6 + DES, tienen su retrato y sus cartas, y el
   jugador elige qué hacen. Un acompañante «de IA» lo dice su ficha.
2. **La ayuda de dados dura hasta que la característica de ataque principal llega a 4 dados**
   (PO-11). Se calcula siempre en el mismo punto (scripteado) y, si después bajas de 4, **no vuelve**.
3. **El daño de un golpe** (PO-12.1, aclaración): la diferencia de éxitos, redondeada hacia arriba,
   son los d6 que se tiran y **se suman**: 1,5 contra 0 son 2d6 (un 4 y un 3: 7 de daño). *Es lo que
   ya hacía el motor (`RPG::dano::dadosFisicos`); la pregunta salía de una revisión vieja del 17/09.*
4. **El crítico atraviesa el bloqueo, salvo que el bloqueo sea crítico** (PO-12.2).
5. **La vida por tier contra el tope de 125** (PO-12.3): aún no está balanceado; no se recorta nada.
6. **La pifia del atacante da un ataque de oportunidad** al que se defendía, **solo golpes físicos,
   nunca hechizos** (PO-12.4).
7. **Morir** (PO-13): escena de noqueo o rescate, cicatriz y moral; después **la pantalla de Game
   Over y vuelves al último punto de guardado con una cicatriz más**.
8. **La esquiva, también fuera del combate, es siempre una tirada enfrentada** (PO-14).
9. **El sonido espera al banco de qwen** (AV14-04) (PO-15): nada de sonidos provisionales.
10. **El reglamento** (PO-16):
    - **La CON** da el peso que puedes mover: los objetos llevan peso (ligeros y pesados), un personaje
      fuerte mueve los más pesados, y se escala por tier.
    - **La pifia en una salvación**: te entra el daño y **el efecto, duplicado**.
    - **El kit**: un kit de curación cura; un kit de suministros cura 1 o 2 PV como mucho.
    - **La muerte y la agonía**: lo de §7 (no hay otra regla).
    - **Los sets de Tier 2 a 5**: después de Boundington; ahora no afectan al juego.
    - **Las deidades** de clérigos y paladines son **los dioses de Oriol** (Vida, Egos, Envidia,
      Chronos…). No se inventan otros.
11. **El combate y las pruebas antiguas** (sin mesa de cartas, como el de la prueba de nivel del editor)
    **se borran**, midiendo antes que nada los usa (PO-17).

## La historia

12. **Prólogo** (PO-20):
    - el cofre en el prado vale;
    - los recuerdos de 757 están decentes (los revisará con calma);
    - lo de las Santas Montañas: **dos amigos se pelearon por su distinta fe**; se deja entender cuando
      aparecen rezando a un dios nuevo, y no se cuenta más;
    - lo que se ve andando como espectro: aún sin revisar;
    - **los tres amigos llevan túnica, sin capucha; después se ponen la capucha y salen andando de la
      habitación**.
13. **La compañía de Rosita** (PO-21): llega **cuando Sandes, estando a 1/4 de vida o menos, te da un
    golpe mortal**. Si no pasa solo, se scriptea para que pase siempre así.
14. **La tienda y la ciudad** (PO-22):
    - le molestan los menús con la estética vieja (comprar casas, las termas…);
    - los sitios tienen que llamarse por su nombre, por encima de su función: donde está Luisarda,
      **«El lingote de oro»**;
    - **lo que más le molesta es lo difícil que es moverse por la ciudad**: muchos objetos tapan la vista
      e impiden ir recto.
15. **D-MOR-09 sí** (la reputación por facción, en el comercio y el HUD) y **D-MIS-03 sí** (el rumor es
    un beat que se dispara al hablar) (PO-23).
16. **El diario de misiones** (PO-24): se juntan las dos mitades en **un sistema de seguimiento completo
    y detallado**: los apartados de cada misión, cada decisión tomada, sus consecuencias, y lo que
    depara o lo que se te pide.
17. **El mundo** (PO-25):
    - la capital de Bosmurg, **lo que diga el canon** (estamos en el 2000 b.f., el 757 del despertar
      elemental);
    - la leyenda del atlas: bien;
    - **las 950 cartas de ejemplo y las clases escondidas se transforman en habilidades y clases de
      monstruos y enemigos**.

## El motor y la forma de trabajar

18. **Integrar lo terminado que nadie usa: planificarlo en el Gantt** (PO-30).
19. **Aceptadas:**
    - D0006, criterios de cierre (PO-32);
    - D0003, campos obligatorios (PO-35);
    - D0004 opción B, edificios como instancia (PO-36);
    - D0009, partir `BitmapFont` (PO-38);
    - D0012 §2–3, el flujo de las clases (PO-39).
20. **Los restos** (PO-40):
    - el contador de FPS, solo de depuración;
    - las tildes en los JSON, aún no;
    - el UML es histórico, y habría que rehacer el del proyecto entero;
    - las fases: decide Claude (manda el plan de alcance, AV);
    - RUN-05: se replantea;
    - `MotorGraphico-HUD-Redesign/`: revisar qué es, sin prioridad (Oriol quiere un solo proyecto,
      modernizado y terminado);
    - el commit `11cec01`: TRAE intentó diseñar el sur del mapa del mundo, probablemente mal.
21. **Usabilidad** (PO-05): aplicar al proyecto los apuntes de usabilidad de la UOC (`/Users/admin/Documents/UOC`).
    Hecho en `planificacion_alcance_y_vision/USABILIDAD_UOC.md`.

## Consecuencias

- Las tareas nuevas van al plan: la fase **AV15** (usabilidad y ciudad) y tareas nuevas en AV13 y AV14.
- `BANDEJA_PRODUCT_OWNER.md`: todo lo contestado pasa a §0 «Resuelto»; lo de las 16:00 se queda, con
  más detalle.

## Corrección (14/10, 13:01 — PO-39 a PO-41)

- **§14:** el ejemplo «donde está Luisarda, El lingote de oro» era una confusión. **«Lingote de Oro» es la
  herrería de Aigren** (canon D1-02); **la tienda de Luisarda es «La Boutique de Madame»** (canon D1-03).
- **La Taberna Rica es la posada** (`interior_posada`): cartel «Taberna Rica», «cama y comida», emblema
  cama.
- **La tabla de nombres de Boundington** (`planificacion_alcance_y_vision/NOMBRES_BOUNDINGTON.md`) queda
  aprobada con esos dos cambios.

