# 2026-10-15 · zcode — AV17-02: pasar el tiempo y el día 3 entero

**Rama:** `agente/zcode/AV17-01`, commit `26f66fa` (siguiendo al `8aa86c6` de AV17-01; commits separados
por tarea). **Ficheros:** `boundington_primer_dia.json`, `boundington_segundo_dia.json`,
`boundington_ocaso.json`. Sin C++.

## Qué había

- Rosita (día 1 y 2): «Comer algo mientras pasa la mañana (5)» (+1 franja), «Esperar a que anochezca»,
  «Subir a dormir», «Nada». Sin condiciones: comer valía desde cualquier hora, dormir saltaba el día
  entero desde la mañana.
- Posada (Taberna Rica): nada — el `posadero` de `interior_posada.json` no tenía ni un beat.
- **Día 3 sin plan** (llegar sin documentos): `b3_plaza_vacia` encendía `bnd3_sin_plan` y el auto
  `b3_dia_pasa` se comía el día entero AL MOMENTO («El día pasa. Al ponerse el sol…»): el jugador se
  quedaba sin el día para terminar misiones — exactamente la queja de Oriol.

## Qué hay ahora

1. **Rosita días 1-2**: condiciones por franja en las opciones (comer solo de mañana; anochezca hasta la
   tarde; dormir desde la tarde). Máximo 4 visibles a la vez.
2. **`b1/b2_posadero_hora`** [talk:posadero]: «Esperar a la tarde» (gratis), «Esperar a que anochezca»,
   «Subir a dormir» (enciende la flag de dormir del día: cierra el día como la taberna), «Nada, gracias».
3. **Día 3**: `b3_dia_pasa` solo dispara con franja noche (y cede si esperas con Rosita:
   `notFlags bnd3_espera`); `b3_campanas_solas` también exige noche; nuevo **`b3_tarde_aviso`** (auto, día
   3 + tarde): la luz se tumba sobre los tejados y el registro dice «El Ocaso se acerca: al caer el sol
   empezará. Termina lo que tengas pendiente.»; Rosita del día 3 gana «Esperar a la tarde» y su «Esperar a
   que anochezca» ahora pasa el tiempo de verdad (antes no pasaba la franja). El Ocaso empieza, pues:
   al elegirlo (avisar a Venides / esperar con Rosita) o al caer la noche — y el jugador está avisado.

## Verificación

- `test_diario_completo` 5/5 · `medir_objetivos --exigir` 31/31 · `validar_todo` sección Aventuras OK
  (el FAIL global es el `numpy` preexistente de `validar_pnj`) · `./build-release/juego 60` exit 0 ·
  ctest gl-free con solo los rojos preexistentes.
- Simulación de flujos: mañana→tarde→noche por taberna y posada; día 3: mañana jugable (solo `b3_reloj`),
  aviso a la tarde, ocaso solo de noche; Rosita día 3 tarde→noche→campanas.

## Trampas nuevas (para la Parte 3 de la bitácora si procede)

- **`kMaxOpciones` = 4 es límite duro del cargador C++** (`BeatsAventura.cpp:329`): un beat con 5 opciones
  hace que TODA la aventura no cargue (todo-o-nada), y el validador Python no lo ve. Me cargó la primera
  versión de los menús; la bisección (restaurar por fichero y correr `test_diario_completo`) lo encontró.
  Cuidado también con `kMaxTextoOpcion` = 60 letras.
- Esta sesión perdí trabajo no commiteado por un `cp` a /tmp que falló en silencio antes de un
  `git checkout HEAD`: **commit temprano, commit menudo** en worktrees.

## Pendiente

- **PO-50** (bandeja): la franja «mediodía» pide una orden del motor (Claude). Textos listos en la PO.
