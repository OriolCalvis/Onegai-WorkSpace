# Ventanas y utilidades del proyecto

Inventario ligero y riguroso de lo que ofrece actualmente la app. Revisado contra controladores, plantillas y scripts del repo.

Nota de nomenclatura: el diseno del proyecto habla de 4 stats. En la implementacion actual aparecen `CON`, `DES`, `INT` y `CAR` en formularios/modelos; si el stat definitivo debe llamarse `SAB`, queda pendiente una migracion de nombres.

## 1. Mapa rapido

| Zona | Ruta | Estado | Uso principal |
|---|---:|---|---|
| Inicio | `/` | Activa | Dashboard y accesos rapidos. |
| Mapa Mundi | `/mapa` | Activa | Exploracion y edicion de geografia/juego. |
| Personajes | `/personatges` | Activa | CRUD de personajes, ficha, PDF y JSON. |
| Cartas | `/cartas` | Activa | Resumen del sistema de cartas. |
| Cartas jugables | `/cartas/<tipo>` | Activa | CRUD de clases, razas, trasfondos, habilidades, equipo, etc. |
| Monstruos | `/cartas/enemigos` | Solo lectura | Album de enemigos y villanos. |
| Historias | `/historias` | Solo lectura | Catalogo narrativo enlazado a facciones, enemigos, tesoros y PNJs. |
| PNJs | `/npcs` | Solo lectura | Album de reparto narrativo. |
| Tesoros | `/tesoros` | Solo lectura | Tablas de botin. |
| Aventuras | `/aventuras` | Activa | CRUD de aventuras y vista imprimible de cartas. |
| Eventos | `/eventos` | Activa | CRUD de eventos del mundo. |
| Exportaciones | `/exportacions` | Activa | Indice de endpoints de exportacion. |
| Diagnostico | `/diagnostico` | Activa | Integridad, metricas, sesiones, memoria. |
| Configuracion | `/configuracio` | Pendiente | Pantalla placeholder. |

## 2. Navegacion comun

| Elemento | Archivo | Funcion |
|---|---|---|
| Header | `templates/Blocks/header-v2.html` | Accesos principales: Inicio, Personajes, Cartas, Exportaciones, Configuracion. |
| Sidebar | `templates/Blocks/sidebar-v2.html` | Menu completo de todas las zonas visibles. |
| Subnav de cartas | `templates/Blocks/cartas-subnav-v2.html` | Navegacion interna entre tipos de carta. |
| Theme toggle | `static/js/theme.js` | Alterna tema visual. |
| Paneles plegables | `static/js/panels.js` | Abre/cierra secciones y paneles laterales. |
| Selector CSV | `static/js/selector-csv.js` | Selector reutilizable para campos de referencias por ID. |

## 3. Ventanas principales

### 3.1 Inicio

| Ruta | Vista | Datos mostrados | Acciones |
|---|---|---|---|
| `/` | `home.html` | Contadores de personajes, clases, razas y trasfondos. | Crear personaje, entrar a Cartas. |

### 3.2 Mapa Mundi

| Ruta | Vista | Datos mostrados | Acciones |
|---|---|---|---|
| `/mapa` | `mapa/index.html` | Regiones/facciones, naciones, puntos, ciudades, zonas, cronologia, historias, aventuras, eventos, PNJs, razas y deidades. | Filtrar/explorar, enlazar puntos personalizados, editar geografia en cliente. |
| `POST /mapa/guardar` | Sin vista | `data/mapa/geografia.json` | Guarda poligonos, marcadores, puntos, ciudades, zonas y cronologia. |

Notas:
- Las regiones se agrupan automaticamente desde `data/historias`.
- Las aventuras se vinculan a facciones por las historias que contienen.
- Los eventos pueden vincularse por faccion propia o por punto personalizado.

### 3.3 Personajes

| Ruta | Vista | Uso |
|---|---|---|
| `/personatges` | `personatges/llista.html` | Lista personajes con filtros y datos calculados. |
| `/personatges/crear` | `personatges/formulari.html` | Crea un personaje eligiendo cartas existentes. |
| `/personatges/{id}` | `personatges/detall.html` | Ficha completa con stats finales y cartas asignadas. |
| `/personatges/{id}/editar` | `personatges/formulari.html` | Edita personaje existente. |
| `POST /personatges/{id}/eliminar` | Redireccion | Elimina personaje. |
| `/personatges/{id}/export/pdf` | Descarga | Exporta ficha en PDF. |
| `/personatges/{id}/export/json` | Descarga | Exporta personaje en JSON. |

Validaciones al guardar:
- Slots de equipo.
- Compatibilidad de equipo con clase.
- Limite de mano por tier.
- Tier maximo de habilidades, equipo y hechizos.
- Limite de dotes por tier.
- Limite de cartas divinas.

Persistencia: `data/personatges/{id}.json`.

### 3.4 Cartas: resumen

| Ruta | Vista | Uso |
|---|---|---|
| `/cartas` | `cartas/index.html` | Dashboard de tipos de carta y contadores. |

Incluye contadores de clases, razas, trasfondos, habilidades, armas/objetos, dotes, pasivas, hechizos, consumibles, rasgos, invocaciones, deidades, condiciones y monstruos.

## 4. CRUD de cartas

Todas estas zonas comparten patron:

`GET listado` -> `GET nueva` -> `POST crear` -> `GET detalle` -> `GET editar` -> `POST editar` -> `POST eliminar`.

| Tipo | Ruta base | Datos en disco | Campos principales |
|---|---|---|---|
| Clases | `/cartas/clases` | `data/cartas/clases` | Rol, tier, vida base, escalado CON, stats, recursos, equipo inicial, pasivas, habilidades, hechizos, restricciones, especializaciones. |
| Razas | `/cartas/razas` | `data/cartas/razas` | Tier, bonos de stat, rasgo pasivo, rasgo activo, afinidades, limitaciones, tags narrativos. |
| Trasfondos | `/cartas/transfondos` | `data/cartas/transfondos` | Tier, bonos, habilidades narrativas, contactos, equipo, pasiva narrativa, complicacion. |
| Habilidades | `/cartas/habilidades` | `data/cartas/habilidades` | Tier, rareza, tags, requisitos, coste, recuperacion, accion, alcance, duracion, defensa, efecto, escalado, evolucion. |
| Armas y objetos | `/cartas/armas` | `data/cartas/armas` | Slot, tier, rareza, peso, bonos, penalizaciones, tags, habilidad vinculada, requisitos, restricciones. |
| Dotes | `/cartas/dotes` | `data/cartas/dotes` | Tier, rareza, tags de clase, requisitos, incompatibilidades, tags concedidos, efecto, limitaciones. |
| Pasivas | `/cartas/pasivas` | `data/cartas/pasivas` | Tier, tags de clase, disparador, efecto, escalado, limitaciones, sinergias, unicidad. |
| Hechizos | `/cartas/hechizos` | `data/cartas/hechizos` | Escuela, tier, rareza, stat de lanzamiento, recuperacion, alcance, area, duracion, tags, efecto, evolucion. |
| Consumibles | `/cartas/consumibles` | `data/cartas/consumibles` | Tier, rareza, tipo de accion, efecto, usos, texto de sabor. |
| Rasgos | `/cartas/rasgos` | `data/cartas/rasgos` | Origen, tier, efecto, limitaciones, texto de sabor. |
| Invocaciones | `/cartas/invocaciones` | `data/cartas/invocaciones` | Invocada por, tier, vida, ataques, movimiento, pasiva, duracion, control. |
| Deidades | `/cartas/deidades` | `data/cartas/deidades` | Dominio, favor, escalado, compatibilidades, obligaciones, texto de sabor. |
| Condiciones | `/cartas/condiciones` | `data/cartas/condiciones` | Categoria, fuente, acumulable, duracion, efectos, curas, texto de sabor. |

Utilidad especial:
- En detalle de clase se calcula una prevision de vida con `CalculadoraVida`.
- Los campos que referencian otras cartas usan selectores, no texto libre, cuando hay catalogo disponible.

## 5. Albumes de solo lectura

### 5.1 Monstruos

| Ruta | Vista | Uso |
|---|---|---|
| `/cartas/enemigos` | `cartas/enemigos/lista.html` | Lista villanos y enemigos regulares ordenados por tier/nombre. |
| `/cartas/enemigos/{id}` | `cartas/enemigos/detalle.html` | Ficha completa del enemigo. |

Fuente: `data/cartas/enemigos`.

Motivo de solo lectura: el esquema de enemigo es muy variable (fases, jefes, arenas, acciones especiales), por eso no hay formulario rigido.

### 5.2 Historias

| Ruta | Vista | Uso |
|---|---|---|
| `/historias` | `historias/lista.html` | Lista historias y facciones. |
| `/historias/{id}` | `historias/detalle.html` | Detalle narrativo con antagonista, recompensa y PNJs relacionados. |

Fuente: `data/historias`.

Enlaces resueltos:
- Antagonista -> `/cartas/enemigos/{id}` si existe.
- Recompensa -> `/tesoros/{id}` si existe.
- PNJs con `secretHook` apuntando a la historia.
- Meses afines -> trasfondos.

### 5.3 PNJs

| Ruta | Vista | Uso |
|---|---|---|
| `/npcs` | `npcs/lista.html` | Album de PNJs con filtro por texto/rol. |
| `/npcs/{id}` | `npcs/detalle.html` | Ficha con agenda, actitud, frases, servicios y secreto. |

Fuente: `data/npcs`.

### 5.4 Tesoros

| Ruta | Vista | Uso |
|---|---|---|
| `/tesoros` | `tesoros/lista.html` | Album de tablas de botin. |
| `/tesoros/{id}` | `tesoros/detalle.html` | Detalle de oro, drops y objetos enlazados. |

Fuente: `data/loot`.

## 6. Aventuras

| Ruta | Vista | Uso |
|---|---|---|
| `/aventuras` | `aventuras/llista.html` | Lista aventuras. |
| `/aventuras/crear` | `aventuras/formulari.html` | Crea aventura componiendo historias, deidades, PNJs, villanos, enemigos, loot y trampas. |
| `/aventuras/{id}` | `aventuras/detall.html` | Detalle con historias, facciones tocadas y puntos del mapa vinculados. |
| `/aventuras/{id}/editar` | `aventuras/formulari.html` | Edita aventura. |
| `/aventuras/{id}/imprimir` | `aventuras/imprimir.html` | Prepara cartas imprimibles de aventura. |
| `POST /aventuras/{id}/eliminar` | Redireccion | Elimina aventura. |

Persistencia: `data/aventuras/{id}.json`.

La impresion combina:
- Historias.
- Deidades.
- PNJs.
- Villanos.
- Enemigos.
- Loot.
- Trampas.

Formato de impresion: cartas 63,5 x 88,9 mm, 9 cartas por pagina A4. El servidor trocea la lista antes de renderizar para evitar cortes raros de CSS en impresion.

## 7. Eventos

| Ruta | Vista | Uso |
|---|---|---|
| `/eventos` | `eventos/lista.html` | Lista eventos. |
| `/eventos/crear` | `eventos/formulario.html` | Crea evento de mundo. |
| `/eventos/{id}` | `eventos/detalle.html` | Detalle con disparador, efectos, opciones del grupo y enlaces al mapa. |
| `/eventos/{id}/editar` | `eventos/formulario.html` | Edita evento. |
| `POST /eventos/{id}/eliminar` | Redireccion | Elimina evento. |

Persistencia: `data/eventos/{id}.json`.

Campos funcionales:
- Faccion opcional.
- Tier.
- Disparador.
- Efectos como lista de lineas.
- Opciones del grupo.

## 8. Exportaciones

| Ruta | Vista/Salida | Uso |
|---|---|---|
| `/exportacions` | `exportacions/index.html` | Lista endpoints disponibles. |
| `/personatges/{id}/export/json` | `application/json` | Descarga personaje. |
| `/personatges/{id}/export/pdf` | `application/pdf` | Descarga ficha PDF. |

Nota: la pagina de exportaciones es un indice. Las exportaciones reales cuelgan de personajes.

## 9. Diagnostico y errores

| Ruta | Vista | Uso |
|---|---|---|
| `/diagnostico` | `diagnostico.html` | Panel de integridad, ultimas peticiones, sesiones activas, memoria y entorno. |
| `POST /diagnostico/reejecutar` | Redireccion | Repite chequeo de integridad. |
| Error global | `error.html` | Pantalla de error con enlace a inicio y diagnostico. |

El diagnostico revisa referencias rotas del catalogo y complementa los logs de `logs/onegai.log`.

## 10. Configuracion

| Ruta | Vista | Estado |
|---|---|---|
| `/configuracio` | `configuracio.html` | Placeholder: "Panell pendent". |

No hay ajustes funcionales implementados en esta pantalla.

## 11. Utilidades de scripts

| Script | Tipo | Salida/efecto | Uso recomendado |
|---|---|---|---|
| `scripts/auditar_campos_referencia.py` | Auditoria | Sale 0 si las referencias de formularios/advice estan bien. | Validar que los campos `*Id` y similares no se tratan como texto libre. |
| `scripts/exportar_catalogo_ids.py` | Inventario | Imprime IDs por catalogo. | Consultar IDs disponibles; admite categorias como argumentos. |
| `scripts/generar_equipo_data.py` | Generador | `data/cartas/armas`, `data/kits/kits_iniciales.json`. | Generar equipo base y kits sin pisar IDs existentes. |
| `scripts/generar_equipo_data_2.py` | Generador | Armas, habilidades y consumibles. | Ampliar equipo y cartas ligadas a objetos. |
| `scripts/generar_artefactos.py` | Generador | Equipo artefacto ligado a deidades. | Crear artefactos por dominio/deidad. |
| `scripts/generar_panteon.py` | Generador | `data/cartas/deidades`, `data/cartas/hechizos`. | Crear panteon y habilidades divinas. |
| `scripts/generar_trasfondos_v2.py` | Generador | `data/cartas/transfondos`. | Crear trasfondos por meses/energia narrativa. |
| `scripts/generar_invocaciones_trampas_monturas.py` | Generador | Invocaciones, trampas y monturas. | Poblar catalogos auxiliares de aventura/combate. |
| `scripts/generar_enemigos.py` | Generador | `data/cartas/enemigos`. | Crear bestiario amplio. |
| `scripts/generar_jefes.py` | Generador | `data/cartas/enemigos`. | Enriquecer jefes existentes con perfiles/fases. |
| `scripts/generar_loot.py` | Generador | `data/loot`. | Crear tablas de botin. |
| `scripts/generar_historias_npcs.py` | Generador | `data/historias`, `data/npcs`. | Crear historias por faccion y reparto de PNJs. |
| `scripts/migrar_a_tiers.py` | Migracion | `data/cartas/{clases,razas,transfondos}`. | Migrar contenido legacy; revisar antes de usar porque contiene una ruta absoluta antigua. |
| `scripts/prueba_rendimiento.sh` | QA/performance | Reporte por ruta: OK/KO, media, minimo, maximo, p95. | Ejecutar con la app levantada: `bash scripts/prueba_rendimiento.sh 20 http://localhost:8080`. |

## 12. Utilidades internas de app

| Utilidad | Clase/archivo | Funcion |
|---|---|---|
| Calculo de ficha | `PersonatgeService`, `FichaPersonatge` | Resuelve cartas, stats finales, vida y validaciones. |
| Calculo de vida | `CalculadoraVida` | Previsualiza/deriva vida desde clase, CON, tier y equipo. |
| Export PDF | `PdfService` | Genera PDF de personaje. |
| Catalogo de aventura | `CatalogoAventuraRepository` | Indexa enemigos, PNJs, loot y trampas para aventuras/mapa. |
| Geografia editable | `GeografiaMapaService` | Lee/escribe `data/mapa/geografia.json`. |
| Diagnostico | `DiagnosticoService` | Revisa integridad de referencias. |
| Metricas HTTP | `MetricasPeticiones` | Guarda ultimas peticiones, lentas y errores. |
| Sesiones | `RegistroSesiones` | Lista sesiones activas. |
| Trazado | `TrazadoAspect` | Log transversal de servicios/repositorios. |
| Errores globales | `ManejadorErroresGlobal` | Normaliza pantallas de error. |

## 13. Persistencia por carpeta

| Carpeta | Contenido | Editable desde web |
|---|---|---|
| `data/personatges` | Personajes | Si |
| `data/cartas/clases` | Clases | Si |
| `data/cartas/razas` | Razas | Si |
| `data/cartas/transfondos` | Trasfondos | Si |
| `data/cartas/habilidades` | Habilidades | Si |
| `data/cartas/armas` | Equipo | Si |
| `data/cartas/dotes` | Dotes | Si |
| `data/cartas/pasivas` | Pasivas | Si |
| `data/cartas/hechizos` | Hechizos | Si |
| `data/cartas/consumibles` | Consumibles | Si |
| `data/cartas/rasgos` | Rasgos | Si |
| `data/cartas/invocaciones` | Invocaciones | Si |
| `data/cartas/deidades` | Deidades | Si |
| `data/cartas/condiciones` | Condiciones | Si |
| `data/cartas/enemigos` | Enemigos/villanos | No, solo lectura. |
| `data/historias` | Historias | No, solo lectura. |
| `data/npcs` | PNJs | No, solo lectura. |
| `data/loot` | Tesoros | No, solo lectura. |
| `data/aventuras` | Aventuras | Si |
| `data/eventos` | Eventos | Si |
| `data/mapa/geografia.json` | Mapa, puntos, ciudades, zonas, cronologia | Si, desde `/mapa`. |

## 14. Arranque y QA basico

| Accion | Comando |
|---|---|
| Arrancar app | `./mvnw spring-boot:run` |
| Ejecutar tests Maven | `./mvnw test` |
| Auditar referencias de formularios | `python3 scripts/auditar_campos_referencia.py` |
| Ver IDs de catalogo | `python3 scripts/exportar_catalogo_ids.py` |
| Probar rendimiento | `bash scripts/prueba_rendimiento.sh 20 http://localhost:8080` |

## 15. Pendientes detectados

| Pendiente | Impacto |
|---|---|
| `/configuracio` no tiene funcionalidad real. | La navegacion la muestra, pero aun no configura nada. |
| Enemigos, historias, PNJs y tesoros son solo lectura. | Para editarlos hay que tocar/generar JSON. |
| `scripts/migrar_a_tiers.py` contiene ruta absoluta antigua. | Puede fallar o escribir fuera del repo actual si se ejecuta sin revisar. |
| Nomenclatura `CAR` vs `SAB`. | Puede crear confusion entre diseno y datos actuales. |
| Algunas docs antiguas pueden estar desactualizadas respecto a las pantallas actuales. | Este archivo debe tomarse como inventario vigente de ventanas/utilidades. |
