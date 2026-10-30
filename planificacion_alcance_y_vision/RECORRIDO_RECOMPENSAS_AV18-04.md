# AV18-04 — recorrido de recompensas, 15/10/2026

La calibración sigue abierta. Las aventuras y `quests.json` se han leído sin modificarlas.
`recorrer_recompensas` enlaza con `motor_level` y usa `NarrativeEngine`: requisitos, opciones,
flags y exclusiones los decide el mismo motor que juega la campaña. No hay un intérprete narrativo Python.

`tools/rutas_economia.json` declara los escenarios historia, explora todo y optimiza. Los tres llegan
al epílogo; el nombre del perfil no acredita exploración exhaustiva ni un óptimo global.
Tiradas de éxito, victorias, primera cara de tablas, moral neutral y nivel 1 son hipótesis,
no resultados de una partida real. La política de respuestas es explícita: preferencias,
pasar página y última salida disponible. Se registra cada beat y opción que paga o entrega objetos.
El prólogo no tiene efectos económicos narrativos; kit/cofres de cinemáticas quedan fuera.
No se simulan movimiento, disponibilidad de tenderos, tiempo ni asentamiento de moral.

## Qué se ha leído

El manifiesto contiene seis aventuras. Hay 29 efectos económicos: 19 `grantGold` (también cobros)
y diez `darObjeto`, contando las opciones. Son alternativas, nunca una suma de ingresos.
Las seis fichas de `quests.json` tienen `rewardGold=0` y `rewardItemIds=[]`; no se añade
un premio por completar una flag. «Silbido de Medianoche» ofrece 30 de oro, pero está fuera
del manifiesto Boundington: se informa y no se cobra. La auditoría JSON conserva SHA-256 de las fuentes.

| Archivo | Beat | Opción (índice; — = efecto del beat) | Efecto |
|---|---|---|---|
| boundington_primer_dia.json | `b0_calle_sin_cama` | — | grantGold -25 |
| boundington_primer_dia.json | `b0_rosita` | 0 | grantGold -25 |
| boundington_primer_dia.json | `b0_rosita_s0` | 0 | grantGold -25 |
| boundington_primer_dia.json | `b0_rosita_p0` | 0 | grantGold -25 |
| boundington_primer_dia.json | `b1_rosita_luisarda` | — | darObjeto 2 × pocion |
| boundington_primer_dia.json | `b1_skilla_hora` | 0 | grantGold -5 |
| boundington_primer_dia.json | `b1_luisarda_pociones_miedo` | — | darObjeto 2 × pocion |
| boundington_primer_dia.json | `b1_luisarda_pociones` | — | darObjeto 2 × pocion |
| boundington_primer_dia.json | `b1_xila_resentido` | 0 | grantGold -1 |
| boundington_primer_dia.json | `b1_xila_s4_a` | 0 | grantGold -1 |
| boundington_primer_dia.json | `b1_xila_s4` | 0 | grantGold -1 |
| boundington_primer_dia.json | `b1_xila_tarde` | 0 | grantGold -1 |
| boundington_primer_dia.json | `tut_derrota` | — | grantGold -15 |
| boundington_primer_dia.json | `b1_pulga_s1_a` | 1 | grantGold 10 |
| boundington_primer_dia.json | `b1_carterista_robo` | — | grantGold -10 |
| boundington_primer_dia.json | `b1_fuego_s3_a` | 0 | grantGold 10 |
| boundington_segundo_dia.json | `b2_skilla_espada_devuelta` | 2 | grantGold -1 |
| boundington_segundo_dia.json | `b2_skilla_hora` | 0 | grantGold -5 |
| boundington_segundo_dia.json | `b2_aigren_daga` | — | darObjeto 1 × daga_de_aigren |
| boundington_segundo_dia.json | `b2_luisarda_pociones` | — | darObjeto 2 × pocion |
| boundington_segundo_dia.json | `b2_cueva_llegada` | 0 | grantGold -5 |
| boundington_segundo_dia.json | `b2_gorran_robo_limpio` | — | darObjeto 1 × furia_de_alaro |
| boundington_segundo_dia.json | `b2_gorran_victoria` | 1 | darObjeto 1 × furia_de_alaro |
| boundington_segundo_dia.json | `b2_gorran_victoria` | 2 | darObjeto 1 × furia_de_alaro |
| boundington_segundo_dia.json | `b2_ritual_bien` | — | darObjeto 1 × amuleto_gitano |
| boundington_segundo_dia.json | `b2_ritual_medio` | — | darObjeto 1 × amuleto_gitano |
| boundington_segundo_dia.json | `b2_cobrador` | 0 | grantGold -20 |
| boundington_segundo_dia.json | `b2_robo_bien` | — | grantGold 40 |
| boundington_segundo_dia.json | `b2_sonsaca_exito` | 1 | grantGold 30 |

## Resultado provisional

| Perfil | Día 1 | Día 2 | Día 3 |
|---|---:|---:|---:|
| historia | 5 | 5 | 5 |
| explora_todo | 5 | 2 | 2 |
| optimiza | 37 | 14 | 6 |

Semilla 17, precios neutrales, cero contenedores nativos colocados en esta rama. Los nueve márgenes
siguen fuera. En las tres rutas se cobra la cama una vez (25); se descuenta de la reserva narrativa
del día para no duplicarla. Un cobro mayor que la bolsa se limita a cero, como `GameSession::addGold`.
Los premios narrativos se conservan por unidad; no se venden para fingir liquidez de objetos de misión.
Optimiza cobra 10 al carterista y 40 en el guardarropa, sin cobrar además los 30 del chantaje.
Las demás alternativas siguen inventariadas, sin fingir que estas tres rutas las visitan todas.

## Contraste con AV18-05

Qwen ha publicado `da120b1` en `agente/qwen/AV18-05`. Su reparto inicial es del día 1:
17 instancias con `loot` fijo, cero con el bloque nativo de AV18-02/03, y `eventId` repetido
por triplicado en ciudad_centro, interior_armeria y afueras_boundington.
El reparto se puede leer con `--reparto` sin copiar niveles ni fusionar ramas. La herramienta
lo señala como incompatible; no convierte esos 17 lotes en cero ingresos certificados.
Faltan el contrato `contenedor`/tabla, `botin{dia,region,canal,orden}` e identidad estable por instancia,
y contrastar los días 2/3. El mínimo de escondite vigente en ECONOMIA.md ya es 20 (AV18-03);
el informe de Qwen aún cita la versión de 40. No se han cambiado sus datos ni su generador.

## Repetir cuando llegue el reparto compatible

Desde `MotorGraphico-main/MotorGraphico` de esta rama:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target recorrer_recompensas simular_contenedores -j8
ctest --test-dir build-release -R '^(test_simular_economia|test_recompensas_economia)$' --output-on-failure
python3 tools/simular_economia.py --json
python3 tools/simular_economia.py --json --reparto /ruta/al/motor/de/qwen
```

Repetir con `--ajuste -25` y `--ajuste 50`; `--rutas` permite revisar decisiones sin tocar aventuras.
Salida 2 = modelo ejecutado y calibración pendiente; salida 1 = error de datos/recorrido.
Esta continuación no emite certificación automática (calibrado=false), incluso si algún saldo entra
por casualidad. Antes del cierre hay que verificar cobertura de rutas, colocación/alcance, horarios,
límites PO-49, compras y márgenes reales; no se han cambiado presupuestos ni precios para dar verde.

Validación: Release completa; gl-free 234/236 (solo dos guardias heredadas, 49 instantáneos/44 deuda).
13 casos anteriores del simulador y 13 nuevos de recompensas en verde. Escenarios neutral, -25, +50
y lectura del reparto Qwen salen 2 sin errores. Sin cambios a assets/adventures, quests ni niveles.
