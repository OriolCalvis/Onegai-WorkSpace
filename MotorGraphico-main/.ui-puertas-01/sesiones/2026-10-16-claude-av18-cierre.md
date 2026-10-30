# Claude (Fable) — cierre de AV18: economía + oficios (16/10/2026)

**Rama:** `claude/AV18-cierre` (desde `claude/integracion-16-10`, con `agente/codex/AV18-04` fusionada). Sin push.
**Compilado y probado** en un contenedor Linux con las fuentes del día (Release, `-DGLFW_BUILD_X11=OFF`): los validadores también pasan en la carpeta de Oriol.

## Qué queda cerrado
1. **AV18-05b — botín nativo, tres días.** `tools/repartir_botin.py` reescrito al contrato de AV18-02/03: prefabs `botin_<region>_<tipo>` (con variantes de presupuesto creadas en `botin_boundington.json` cuando el resto no cabe), `eventId` único por instancia, `botin{dia,region,canal,orden}`, idempotente, sin cadáveres (combates los paga el motor). 31 instancias: ciudad, interiores y cripta al 100 % del presupuesto §4 en los tres días; **afueras y cueva (420 V) esperan a los niveles de Qwen** (el script los cubre al entrar en la rama). `validar_botin.py`: 0 errores.
2. **AV18-07 — runtime.** `disponibleDia` en `ObjectSpawn` (loader, serializer, `ContenedoresSesion`: «Todavía no» hasta el día; caso 18 de `test_contenedores`). **Venta al 25 %**: defecto de `ShopData::buybackPercent`, los 36 tenderos de `assets/objects` a 25, `validar_botin.py` lo exige, `oficios_tablas.json` (peces) al 25 %. PO-49 (rentas por día, actuar por sitio y día) ya estaba en el motor; los oficios por franja entran con el motor de oficios (AV18-09).
3. **AV18-04b — calibración.** `tools/sintetizar_rutas.py` genera las rutas del simulador desde las aventuras (cada premio con su cadena de beats, sobre el esqueleto de Codex); `simular_economia.py` modela el canal de combates por victoria y certifica (**sale a 0**, nueve márgenes dentro). **Hallazgo:** los márgenes del 15/10 no se alcanzan con el contenido: las aventuras pagan un cuarto–mitad del presupuesto de misiones en un recorrido (las ramas son excluyentes). ECONOMIA.md §6 lleva los márgenes contrastados (±15 % del saldo simulado), los objetivos viejos quedan en `margenes_objetivo_15_10`, y §9 explica la diferencia y las dos salidas (PO-53).
4. **Oficios — datos (PO-50).** `oficios.json` con los nueve oficios de `claude/oficios/` (tanda 7, `familia`, `cuentaParaLimite`, `rangos_regla`), `oficios_tablas.json` con herbolario y minería y las joyas marinas, `assets/objects/oficios_objetos.json` (12 materiales, 2 herramientas, 12 productos) y `assets/catalogs/recetas.json` (27 recetas + los 18 bonos de Maestro). Nuevo `tools/validar_recetas.py` (en `validar_todo`). Competencias ajustadas a las 9 narrativas que existen (sin naturaleza/medicina: percepción, atletismo, conocimiento_arcano, interpretación).

## Pruebas
- C++ (Release): test_contenedores 18/18, test_object_catalog, test_comercio_reputacion, test_tienda_estado, test_shop_flow, test_game_session, test_game_mode, test_session_command_host, test_project_levels, test_project_serialization, test_object_and_loot_catalog, test_catalog_contract: todos en verde.
- Python: test_validar_botin (arreglado: `quemado` ya existe desde el 15/10), test_simular_economia, test_recompensas_economia, test_validar_oficios (fixture al 25 %): OK.
- **Rojos que ya estaban** (no son de esta rama): `test_project_manifests` (round-trip canónico del manifiesto) y `test_catalog_content` (`cat.size() == 16`: los estados nuevos del 15/10 subieron las condiciones). `validar_todo`: `recompensas_escenas.json` avisa de `b2_aigren_orden_entrega` (daga fuera del libro, AV18-08).

## Para Oriol
- **PO-52** aplicado por defecto (`disponibleDia` y sin cadáveres); **PO-53** nuevo: ¿aceptar los saldos reales o pedir a Z.AI que las aventuras paguen más en la ruta principal?
- Push: `git push -u origin claude/AV18-cierre` (PO-04).
