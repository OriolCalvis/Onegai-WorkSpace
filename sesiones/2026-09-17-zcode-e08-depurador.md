# 17/09/2026 (2ª tanda) — E-08: el depurador paso a paso del intérprete, probado

- **Commits:** MotorGraphico `1662a64` (1 fichero, 132 líneas de test).
- **Deja esperando a Oriol:** nada.

## Qué se cerró

E-06 aterrizó esta misma mañana (`ec70d29`) y desbloqueó E-08 (dependía de E-03 + E-06). Al estudiarlo resultó que **la maquinaria ya existía**: `RunMode/stepOnce/breakpoints/trace/lastStepNote` llegaron con el rescate del intérprete (f23dba9)… **sin un solo test** — validador que deja de validar, la trampa de la Parte 3. Esta tanda la ejercita y fija su contrato:

- **La prova de la ficha, literal**: dos intérpretes gemelos; el historial del host tras N `stepOnce` es idéntico al de ejecutar N comandos continuos, y reanudar completa el resto en el mismo orden.
- **Puntos de parada**: pausan ANTES de ejecutar el comando (el autor ve con qué estado va a correr), `lastHit` lleva su camino, `nextCommand` dice cuál viene, y continuar pasa por encima UNA vez y limpia el golpe.
- **Paso sobre espera**: contrato fijado a la implementación — el paso SOLO atraviesa la espera (un paso = una acción visible; el comando siguiente va en el paso siguiente) y lo **anota** en `lastStepNote` para distinguir un recorrido del juego de uno del depurador.
- **Traza**: cada comando con su camino (el «log con orden temporal» de §15.3).
- **Aviso** al pisar sin ejecuciones vivas.

`test_event_interpreter` **19/19** (14 previos + 5 nuevos). Falsabilidad: la mutación que desactiva el atraviesamiento de esperas la cazan 2 casos. Verificado en clon limpio de HEAD+parche (build 0 errores).

## Coordinación

Zona virgen (nadie reclama EventInterpreter.* hoy; E-05/E-06 ya aterrizaron). Claude (Cowork) ya pone filas en el TABLERO — la advertencia del expediente cuajó. El paquete E-08 queda ABIERTO por su mitad de panel (event_editor, zona editor): esta es la mitad GL-free consumible por quien haga la UI.

## Registro

Fila liberada con nota «núcleo probado, panel pendiente»; TABLERO Hecho actualizado; INDICE regenerado.
