# Skills de mejora del motor — integración del 29/10/2026

Tres skills locales adaptadas de `replica_motor.zip`, aportado por Oriol:

| Skill | Cuándo usarla | Resultado |
|---|---|---|
| [replica-motor-recon](../../../.agents/skills/replica-motor-recon/SKILL.md) | Reconocer una capacidad del Pla U | Contrato, fuentes y brechas verificadas |
| [replica-motor-construir](../../../.agents/skills/replica-motor-construir/SKILL.md) | Implementar un paquete autorizado | Arquitectura, integración en producto y evidencia |
| [replica-motor-verificar](../../../.agents/skills/replica-motor-verificar/SKILL.md) | Comprobar un hito o medir lo que falta | Casos, bugs y paridad con alcance explícito |

Son skills de proyecto en `MotorGraphico-main/.agents/skills/`, con selección
automática normal. `MotorGraphico-main/AGENTS.md` también las señala para agentes
que no descubran automáticamente las skills locales. No se instalan globalmente.
Ejemplos: «usa $replica-motor-recon para estudiar los triggers», «construye el
paquete autorizado con $replica-motor-construir» y «verifica el hito con
$replica-motor-verificar». Lee solo la que corresponda al trabajo.

## Adaptaciones respecto al ZIP

- Techo vigente de 500 líneas y 15 miembros, separación GL/no-GL, CMake y alcance
  desde los tres ejecutables reales. La versión recibida todavía decía 600.
- Rutas de comandos desde `MotorGraphico/`, coordinación/Git desde `Software/`.
  Sustituido `dades_unity.py`, ausente, por la propuesta del
  Pla U descompuesto (`planificacion/pla_unity/descomposicio/README.md`). Ese plan
  está pendiente de revisión, es WIP local sin versionar y no se incluye ni se
  modifica al instalar estas skills. En otro clon puede faltar: las skills lo
  explican y remiten entonces al contrato vigente, sin inventar el plan.
- Conexión con la skill `fabrica` real, sin dependencia inexistente
  `fabrica-onegai`. Se registra el problema conocido de rutas del hook en el
  monorepo; no se cambia el hook compartido en esta tarea.
  El ejecutor de Fábrica también es WIP local ajeno: se utiliza para registrar
  esta sesión, pero no se incorpora al commit. Si falta, las skills piden registrar
  la limitación y usar comprobaciones nativas sin atribuirse una entrega de Fábrica.
- La autorización procede de la petición y las normas aplicables. No se adopta
  como orden la exigencia adicional de aprobar todo C++ incluida en el ZIP.
- Se reutiliza código existente antes de proponer clases, pools o sistemas nuevos.
  Se conserva canon → cartas → motor y el trabajo local de otras sesiones.
- La lista de paridad ordena por prioridad/crédito, sin resolver dependencias.
  El orden real procede del Pla U. Optimizar rendimiento requiere medidas propias.
- `yes` exige evidencia de integración y aceptación aplicable. Una plataforma no
  probada queda pendiente; una cifra alta no acredita preparación para publicar.

Esta integración instala el procedimiento y la herramienta; no implementa las
capacidades C++ de la matriz ni acredita una mejora de FPS o de tiempos de carga.

## Matriz y puntuación

`features.csv` conserva las 35 filas recibidas: 25 puntuables, 5 exclusiones y
5 capacidades propias. Los estados y notas son **afirmaciones del ZIP pendientes
de auditoría del estado actual**, incluidas las afirmaciones de que algo falta.
`paridad_inicial.md` reproduce su estimación de **10/100**, con aviso de procedencia.
No se crea un recon, build-log o informe actual ficticio: se escribirán al estudiar,
construir o verificar un paquete real.

Desde `MotorGraphico-main/MotorGraphico/`:

```bash
python3 planificacion/replica/parity.py planificacion/replica/features.csv
python3 planificacion/replica/parity.py planificacion/replica/features.csv --json
python3 planificacion/replica/parity.py planificacion/replica/features.csv --markdown
python3 -m unittest discover -s planificacion/replica -p 'test_*.py' -v
```

Must=3, should=2, could=1; partial=0,5. Se excluyen skip y original=no. El
denominador solo corresponde al subconjunto seleccionado, no a todo Unity.
El comando devuelve 0 con una matriz válida, 1 si no alcanza `--fail-under N`,
y 2 con datos inválidos. El umbral utiliza la puntuación sin redondear.
El argumento visual heredado se conserva por compatibilidad, pero estas skills
no lo usan para puntuar Unity; las capturas propias sirven para revisión visual.

`parity.py` se ha endurecido para rechazar CSV mal formado, columnas duplicadas,
estados inválidos, exclusiones sin motivo y umbrales no finitos/fuera de rango.
Mantiene los pesos del original y su licencia. Las pruebas cubren cálculos,
exclusiones, errores, Unicode y códigos de salida.

## Procedencia y comprobación

[procedencia.json](procedencia.json) guarda el SHA-256 del ZIP y de cada archivo
recibido. La atribución recibida apunta a `replica-skill`, de Jake Schincariol;
se conserva [su licencia MIT](LICENSE_replica.txt). No se ha verificado por red
el origen declarado ni se ha utilizado código o documentación de Unity en esta
instalación. Los scripts recibidos se inspeccionaron antes de ejecutarlos.

La línea base general de Fábrica de esta sesión registra cinco checks con fallo
heredado. Para este cambio de skills/herramienta se usa además
[fabrica_replica.json](fabrica_replica.json), con comprobaciones específicas sin
compilar C++ que no se ha modificado. La guardia general se compara por separado
con la base; no se afirma que el motor esté globalmente verde.

Los registros generales viven en `Software/out/fabrica/SKILLS-REPLICA-01/`.
La prueba del commit en copia limpia y los resultados concretos quedan en la
sesión `Software/sesiones/2026-10-29-codex-skills-replica.md`.
