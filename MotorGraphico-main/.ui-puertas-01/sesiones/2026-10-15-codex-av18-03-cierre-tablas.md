# AV18-03 — cierre de tablas de botín tras el local

- **Autor:** Codex / ChatGPT
- **Fecha:** 2026-10-15
- **Commits:** `b6045e5`, sobre `5ff835d` y `ddab68b`, rama `agente/codex/AV18-03`.
- **Deja esperando a Oriol:** revisión e integración de su rama; reparto de instancias en AV18-05 por Qwen. PO-49 permanece en su carril.

Oriol pidió cerrar El Zurrón y seguir AV18-03. Se cierra primero AV16-08 en `43fa13d`/`3394990` y se retoma el catálogo ya hecho, sin crear otra colección ni tocar escenas o niveles de campaña. La implementación inicial está en 2026-10-15-codex-av18-03-botin.md.

La rama propia ahora incorpora la protección encontrada durante AV18-06: una única garantizada debe estar en las entradas de su misma tabla con unico=true. El generador consulta esa marca para reservarla globalmente; sin ella el garantizado podría repetirse. Se añade la regresión y una frase al contrato de contenido. Esta guardia ya está en `3f9ea7e`: al integrar la cadena AV18-06 no se aplica por segunda vez.

## Verificación

Desde MotorGraphico-main/MotorGraphico:

```sh
python3 tools/test_validar_botin.py -v
python3 tools/validar_botin.py
ctest --test-dir build-release -R '^(test_botin_equipo|test_validar_botin|test_contenedores|test_catalog_content|test_validar_todo)$' --output-on-failure
./build-release/test_botin_equipo
python3 tools/validar_todo.py
```

- 15 Python en verde. 50 reutilizables, 12 únicas y 30 tablas. Cero instancias de campaña, aviso honesto de AV18-05.
- Cinco casos nativos: préstamo de cartas sin aprendizaje, dos ranuras sin duplicados, ausencia de ficha/catálogo, ejecución de las doce piezas en BattleState y 832 generaciones con el catálogo real (26×32).
- test_contenedores, test_catalog_content y test_validar_todo también pasan. Para los validadores con gráficos se usa el Python empaquetado con NumPy.
- Copia limpia `/private/tmp/codex-av1803-limpio`: 15 Python, cinco C++ y validar_todo 0 errores/22 avisos. Logs locales codex_av1803_cierre_validar.log y codex_av1803_cierre_limpio.log.
- Esta revisión no cambia C++, habilidades ni equipo: solo la guardia, su regresión y la documentación. No requiere una segunda compilación de fuentes que no han cambiado; la entrega inicial ya compiló Release completa.

Las piezas usan los estados existentes; los cuatro estados futuros siguen marcados como pendientes. La carta se presta al montar la batalla por la pieza equipada y no se aprende permanentemente. Qwen conserva el reparto y los metadatos eventId/botin. Ni PO-49 ni el trabajo sin commit de Claude en el árbol compartido se modifican.
