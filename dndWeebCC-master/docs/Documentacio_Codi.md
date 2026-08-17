# Documentació del codi - ONEGAI / dndWeebCC

Aquest document és el mapa de lectura del codi. No substitueix el GDD (`docs/Sistema_Cartas_Tiers.md`) ni els plans de producte: explica on viu cada responsabilitat, quin flux segueix una petició, com es persisteixen les dades i on s'ha de tocar quan vulguis afegir una funció nova.

Estat revisat el 2026-07-27 sobre el codi real del repositori.

## 1. Resum executiu

El projecte és una aplicació Spring Boot + Thymeleaf orientada a dades. La font de veritat del joc són fitxers JSON a `data/`; Java els carrega en repositoris en memòria, els serveis resolen regles i referències, els controladors preparen models de vista, i Thymeleaf pinta pantalles de gestió, fitxes i mapa.

Principis actuals:

- Cada element important del joc és una carta o una referència a carta: classe, raça, trasfons, habilitat, encanteri, equip, talent, passiva, consumible, invocació, deitat, condició, enemic, trampa, botí, història, PNJ, aventura i personatge.
- No hi ha base de dades relacional. La persistència és JSON pla sota `data/`, un fitxer per entitat.
- El sistema usa CON, DES, INT i CAR. CAR substitueix la SAB antiga.
- La vida es calcula, no s'edita a mà. La fórmula viu a `CalculadoraVida` i la fitxa resolta a `FichaPersonatge` / `PersonatgeService`.
- El mapa és la pantalla més complexa: combina dades de geografia, històries, aventures, PNJs, botí, monstres i events.

## 2. Estructura de capes

```text
Navegador
  Thymeleaf templates + CSS + JS modular
      |
Controladors (cat.dnd.cc.controller)
      |
Serveis (cat.dnd.cc.service)
      |
Repositoris JSON (cat.dnd.cc.repository)
      |
Models POJO (cat.dnd.cc.model) <-> data/**/*.json
```

### 2.1 Models

`cat.dnd.cc.model` conté POJOs serialitzables amb Jackson. Són simples de forma intencionada: camps, getters/setters i helpers petits per vista o càlcul. No haurien de contenir lògica pesada de negoci.

Families principals:

- Cartes de tier: `TierClass`, `TierRace`, `TierBackground`, `TierSkill`, `TierSpell`, `TierEquipment`, `TierFeat`, `TierPassive`, `TierConsumable`, `TierSpecialTrait`, `TierSummon`, `TierDeity`, `TierCondition`.
- Construcció de partida: `Personatge`, `Aventura`, `CartaAventura`, `Historia`, `Evento`, `CartaImpresion`.
- Mapa: `GeografiaMapa`, `RegionMapa`, `PuntoMapa`.
- Suport/legat: `Exportacio`, `Subraca`.

Regla pràctica: si un camp existeix en JSON i s'ha d'editar des de formulari, ha d'existir al model i, si cal, al seu `web/form`.

### 2.2 Repositoris

Els repositoris llegeixen/escriuen fitxers JSON. El patró és repetit:

1. `Path CARPETA = Paths.get("data", ...)`.
2. Al constructor, `Files.createDirectories` i càrrega de tots els `.json`.
3. Cache en memòria (`Map` o `List`).
4. `findAll`, `findById`, `save`, `delete`.

Repositoris de cartes:

| Tipus | Repositori | Carpeta |
|---|---|---|
| Classes | `TierClassRepository` | `data/cartas/clases` |
| Races | `TierRaceRepository` | `data/cartas/razas` |
| Trasfons | `TierBackgroundRepository` | `data/cartas/transfondos` |
| Habilitats | `TierSkillRepository` | `data/cartas/habilidades` |
| Encanteris | `TierSpellRepository` | `data/cartas/hechizos` |
| Equip | `TierEquipmentRepository` | `data/cartas/armas` |
| Talents | `TierFeatRepository` | `data/cartas/dotes` |
| Passives | `TierPassiveRepository` | `data/cartas/pasivas` |
| Consumibles | `TierConsumableRepository` | `data/cartas/consumibles` |
| Trets | `TierSpecialTraitRepository` | `data/cartas/rasgos` |
| Invocacions | `TierSummonRepository` | `data/cartas/invocaciones` |
| Deitats | `TierDeityRepository` | `data/cartas/deidades` |
| Condicions | `TierConditionRepository` | `data/cartas/condiciones` |

Repositoris narratius i de joc:

| Tipus | Repositori | Carpeta |
|---|---|---|
| Personatges | `PersonatgeRepository` | `data/personatges` |
| Aventures | `AventuraRepository` | `data/aventuras` |
| Events | `EventoRepository` | `data/eventos` |
| Històries | `HistoriaRepository` | `data/historias` |
| Catàleg lleuger d'aventures | `CatalogoAventuraRepository` | `data/npcs`, `data/cartas/enemigos`, `data/loot`, `data/cartas/trampas` |

`CatalogoAventuraRepository` és especial: no és un CRUD normal. Indexa cartes grosses com enemics, PNJs, botí i trampes amb un resum lleuger per pickers i pantalles d'aventura. Quan cal detall, exposa `completa(id)`.

### 2.3 Serveis

Els serveis són la capa on viu la regla d'aplicació. Molts serveis de carta són CRUD prims, però n'hi ha de centrals:

- `PersonatgeService`: resol la fitxa completa del personatge, suma stats d'equip/raça, calcula vida/defenses i valida slots, compatibilitat de pes, límits de mà, tier de cartes, talents i divines.
- `AventuraService`: agrupa històries, consulta catàleg lleuger i dona noms llegibles a ids.
- `ConstructorAventuraService`: gestiona la baralla d'història per actes dins d'una aventura (`CartaAventura`), amb alta, edició, moviment entre actes i eliminació.
- `HistoriaService`: llista, guarda i agrupa històries per facció.
- `GeografiaMapaService`: carrega i guarda `data/mapa/geografia.json`.
- `DiagnosticoService`: auditoria del catàleg i integritat de referències; és la revisió de salut de projecte.
- `PdfService`: genera PDF de personatge.
- `SistemaHabilidades`: motor antic/auxiliar de requisits i habilitats JSON. Encara conviu amb els models de tier moderns.

Regla pràctica: si una validació afecta dades guardades, hauria de viure al servei, no només al formulari o al JavaScript.

### 2.4 Controladors

Els controladors són MVC Thymeleaf. Fan tres coses: obtenir dades de serveis, muntar `Model`, retornar nom de template.

Rutes principals:

| Ruta | Controlador | Funció |
|---|---|---|
| `/` | `HomeController` | Dashboard |
| `/configuracio` | `HomeController` | Configuració placeholder |
| `/cartas` | `CartasController` | Índex de catàleg |
| `/cartas/<tipo>` | `Tier*Controller` | CRUD de 13 tipus de carta |
| `/cartas/enemigos` | `MonstruoController` | Àlbum d'enemics, només lectura |
| `/personatges` | `PersonatgeController` | CRUD, fitxa, PDF i JSON |
| `/aventuras` | `AventuraController` | CRUD, detall i impressió |
| `/aventuras/{id}/constructor` | `AventuraController` + `ConstructorAventuraService` | Baralla d'història per actes |
| `/historias` | `HistoriaController` | Llistat, creació manual i detall |
| `/npcs` | `NpcController` | Llistat, creació manual i fitxa GM |
| `/tesoros` | `TesoroController` | Taules de botí, només lectura |
| `/eventos` | `EventoController` | CRUD d'events del mon |
| `/mapa` | `MapaController` | Mapa Mundi interactiu |
| `POST /mapa/guardar` | `MapaController` | Persistir geografia del mapa |
| `/diagnostico` | `DiagnosticoController` | Informe d'integritat |
| `/exportacions` | `ExportacioController` | Index d'exports |

### 2.5 Formularis web

`cat.dnd.cc.web.form` conté DTOs de formulari per evitar exposar directament tota la complexitat dels models a Thymeleaf. Les llistes s'editen sovint com text separat per comes o camps repetits, i es transformen cap al model al controlador.

Quan afegeixis un camp nou a una carta editable, revisa sempre en aquest ordre:

1. Model `TierX`.
2. `TierXForm`.
3. `TierXController` (`desdeForm` / `aForm`, si existeixen).
4. `templates/cartas/<tipo>/formulario.html`.
5. `templates/cartas/<tipo>/detalle.html` i `lista.html`.
6. `DiagnosticoService` si el camp referencia altres ids.

### 2.6 Llengua i noms de codi

La norma del projecte queda documentada a `docs/Convencions_Llengua_i_Codi.md`:

- Documentació, README, Javadocs i comentaris: català.
- Codi nou, identificadors, camps JSON nous i APIs noves: anglès.
- Codi existent: no es reanomena només per traduir-lo; cal migració formal perquè moltes rutes i dades depenen dels noms actuals.

## 3. Fluxos de dades principals

### 3.1 Crear o editar una carta

```text
Usuari -> /cartas/<tipo>/nueva
Controller crea Form buit + opcions globals
Template envia POST
Controller transforma Form -> Model
Service.guardar
Repository.save -> data/cartas/<tipo>/<id>.json
Redirect a detall o llista
```

La validació encara està repartida: alguns errors els frena HTML/Thymeleaf, altres servei/repositori. Si el camp és crític per integritat, cal afegir validació al servei o a `DiagnosticoService`.

### 3.2 Crear personatge

```text
/personatges/crear
  PersonatgeController demana catàlegs disponibles a PersonatgeService
  formulari selecciona ids existents
POST /personatges/crear
  PersonatgeService.guardar
  PersonatgeRepository escriu data/personatges/<id>.json
/personatges/{id}
  PersonatgeService.calcularFicha resol ids -> cartes, suma stats i calcula derivats
```

Validacions importants a `PersonatgeService`:

- `validarSlotsEquipo`: evita dos objectes al mateix slot.
- `validarCompatibilidadEquipo`: pes/compatibilitat segons classe.
- `validarLimiteMano`: límit d'habilitats per tier.
- `validarTierCartas`: no equipar/aprendre cartes per sobre del tier del personatge.
- `validarLimiteDotes`: controla nombre de talents.
- `validarLimiteDivinas`: controla divines/encanteris marcats.

### 3.3 Crear aventura heretada

```text
/aventuras/crear
  Wizard Thymeleaf + JS
  Històries agrupades per facció
  Pickers de PNJs, loot, trampes i combats des de CatalogoAventuraRepository
POST /aventuras/crear
  AventuraRepository escriu data/aventuras/<id>.json
/aventuras/{id}/imprimir
  AventuraController resol totes les cartes a CartaImpresion
```

Aventura no copia contingut: guarda ids i resol noms/detalls quan pinta.

### 3.4 Construir aventura per actes

```text
GET /aventuras/{id}/constructor
  AventuraController carrega aventura + catalegs + CartaAventuraForm
  templates/aventuras/constructor.html mostra tres actes, comptadors i formulari lateral
POST /aventuras/{id}/cartas
  ConstructorAventuraService crea CartaAventura i assigna codi per acte (A1-, B2-, C3-)
POST /aventuras/{id}/cartas/{code}/editar
  Actualitza la carta mantenint el codi
POST /aventuras/{id}/cartas/{code}/mover
  Canvia acte i recalcula el codi disponible de destí
POST /aventuras/{id}/cartas/{code}/borrar
  Elimina la carta de la baralla
```

`CartaAventura` suporta tipus `BASE`, `CONDICIONAL`, `INYECTADA`, `CADENA` i `BALANCE_FINAL`. El formulari actual cobreix els camps operatius principals: acte, tipus, títol, escena, ganxo d'ignorar, requisit condicional, injecció, cadena i referències a enemics, botí i localització.

Regles importants del GDD §20 que ja apareixen al model:

- Una aventura sense `cartasHistoria` continua sent compatible amb el format heretat.
- `Aventura.cartasDeActo(acto)` filtra cartes per acte per a Thymeleaf.
- `Aventura.porcentajeBase(acto)` calcula la vàlvula de seguretat de cartes que entren sempre (`BASE` o `INYECTADA`); la UI mostra el mínim recomanat del 40%.
- `CardKind` i `FichaEstado` serialitzen en minúscules per mantenir JSON llegible.

### 3.5 Mapa Mundi

```text
GET /mapa
  MapaController carrega geografia, històries, aventures, events i catàleg lleuger
  templates/mapa/index.html renderitza SVG + datalists + panels
  static/js/mapa/*.js controla edició, filtres, focus, ciutats, zones i cronologia
POST /mapa/guardar
  GeografiaMapaService.guardar -> data/mapa/geografia.json
```

La geografia és l'únic lloc on es guarden coordenades. Les històries, aventures, PNJs i botí venen de les seves carpetes i només s'enllacen pel seu id/facció.

### 3.6 Diagnostico

`/diagnostico` és el panell de salut. Recorre dades i detecta referències trencades, comptadors i entorn. Hauria de ser el primer lloc a mirar després de generar contingut en massa.

## 4. Frontend

### 4.1 CSS

| Fitxer | Responsabilitat |
|---|---|
| `tokens.css` | Variables de color, spacing, tipografia i radi |
| `base.css` | Reset/base app |
| `components.css` | Components generals: cartes, panels, botons, badges, layout |
| `utilities.css` | Utilitats petites i classes helper |
| `mapa.css` | Estils especifics del Mapa Mundi |
| `paneles-overlay.css` | Sistema de panells superposats al mapa |
| `carta-fisica.css` | Variants de carta física per esquema real |
| `wizard.css` | Wizard d'aventures i passos |
| `ficha-print.css` | Impressió de fitxa |
| `style.css` / `app.css` | Estils antics o globals que encara conviuen |

### 4.2 JavaScript general

| Fitxer | Responsabilitat |
|---|---|
| `theme.js` | Toggle de tema |
| `panels.js` | Drawer mobil, panels i previews generics |
| `selector-csv.js` | Selectors reutilitzables per ids |
| `paneles-overlay.js` | Rail i panells modulars sobre el mapa |
| `mapa-paneles.js` | Mou eines/filtres/llegenda/cronologia dins dels panells overlay |

### 4.3 JavaScript del mapa

Els moduls de `static/js/mapa/` comparteixen l'espai `window.GMMapa`.

| Modul | Responsabilitat |
|---|---|
| `mapa-nucleo.js` | Estat compartit, refs DOM i utilitats de datalist |
| `mapa-filtros.js` | Ocultar/mostrar nacions, faccions i tiers |
| `mapa-puntos.js` | Punts personalitzats i editor de punt |
| `mapa-vertices.js` | Arrossegament de vertexs i marcadors |
| `mapa-borrador.js` | Draft a localStorage, payload i mode edició |
| `mapa-dialogo.js` | Modal reutilitzable del mapa |
| `mapa-ciudades.js` | Crear/editar ciutats |
| `mapa-zonas.js` | Crear/editar zones del terreny |
| `mapa-cronologia.js` | Events historics del mon |
| `mapa-nacion.js` | Fitxa i identitat de nacio |
| `mapa-teclado.js` | Accessibilitat de marcadors amb Enter/Espai |
| `mapa-leyenda.js` | Llegenda 4a: capes, foc i cercador |
| `mapa-init.js` | Connecta listeners i arrenca tot |

Regla pràctica: cap mòdul hauria d'executar efectes globals grans en carregar-se, excepte `mapa-init.js`. La resta defineixen funcions o estat.

## 5. Templates

Les plantilles estan organitzades per area:

- `Blocks/`: header, sidebar, footer, subnav, layout i selector de referències.
- `cartas/<tipo>/`: `lista.html`, `detalle.html`, `formulario.html` per tipus editable; enemics només tenen llista/detall.
- `personatges/`: llista, formulari i detall/playmat.
- `aventuras/`: llista, formulari wizard, detall, imprimible i constructor per actes.
- `historias/`, `npcs/`, `tesoros/`, `eventos/`: catàlegs narratius.
- `mapa/index.html`: SVG i anchors de dades per al mapa modular.

El patró recomanat per una pantalla nova és: layout compartit + header de pàgina + filtres + llista o detall + scripts específics al final.

## 6. Scripts

Els scripts són eines de contingut. Molts escriuen centenars de JSON; abans d'executar-los, fes commit.

| Script | Ús |
|---|---|
| `auditar_campos_referencia.py` | Auditar camps que semblen ids o referències |
| `exportar_catalogo_ids.py` | Exportar ids reals per prompts |
| `generar_habilidades_clases.py` | Generar habilitats per classe/tier amb dry-run |
| `generar_canonicos_ed2.py` | Generar lots canònics ED2 alineats amb les quatre stats |
| `generar_lote_itx.py` | Crear o netejar el lot Itx de prova |
| `generar_historias_npcs.py` | Generar històries i PNJs en massa |
| `generar_enemigos.py`, `generar_jefes.py` | Bestiari i caps |
| `generar_loot.py` | Taules de botí |
| `generar_invocaciones_trampas_monturas.py` | Invocacions, trampes i montures |
| `generar_panteon.py`, `generar_trasfondos_v2.py`, `generar_artefactos.py` | Contingut de món |
| `migrar_a_tiers.py` | Migració antiga a sistema de tiers |
| `renombrar_canonicos_es_en.py` | Normalitzar o migrar noms canònics entre variants ES/EN |
| `desar.sh` | Rutina de commit local |
| `prueba_rendimiento.sh` | Prova rapida de rendiment |

## 7. Llegat i zones delicades

- `src/main/java/com/onegai/*` sembla heretat o prototip antic. No forma part del paquet Spring principal (`cat.dnd.cc`) i no s'hauria d'ampliar sense decidir si es migra o s'elimina.
- `SistemaHabilidades` existeix en dues versions (`cat.dnd.cc.service` i `com.onegai`). Abans de tocar-lo, comprova quin flux real l'usa.
- `style.css` i `app.css` conviuen amb el sistema nou de tokens/components. Si fas UI nova, prefereix `tokens.css`, `components.css` i utilitats.
- Les dades es guarden en JSON sense transaccions. Un generador pot tocar moltes coses; fes commit abans.
- `data/cartas/clases/vsdvsvsd.json` i alguns personatges amb noms de prova són brossa coneguda si encara existeixen.

## 8. Com afegir una entitat nova

### 8.1 Nou tipus de carta editable

1. Crear model `TierX`.
2. Crear form `TierXForm`.
3. Crear repository amb carpeta `data/cartas/<x>`.
4. Crear service CRUD.
5. Crear controller `/cartas/<x>`.
6. Crear templates `lista`, `detalle`, `formulario`.
7. Afegir link a `Blocks/cartas-subnav-v2.html` i comptador a `CartasController`.
8. Afegir checks a `DiagnosticoService` si té referències.
9. Crear dades de mostra.
10. Tests si hi ha validació o càlcul.

### 8.2 Nou camp en una carta existent

1. Afegir camp al model + getter/setter.
2. Afegir camp al form si editable.
3. Mapar form <-> model al controller.
4. Pintar-lo a detall/lista si aporta valor.
5. Actualitzar scripts generadors.
6. Actualitzar docs de prompt/esquema.
7. Executar `/diagnostico`.

### 8.3 Nova pantalla

1. Controlador petit que només prepara model.
2. Servei si hi ha regla de negoci.
3. Template amb layout compartit.
4. JS separat si hi ha interaccio complexa.
5. CSS componentitzat; evitar styles inline grans.

## 9. Tests i verificacio

Tests actuals:

- `DndWeebCcApplicationTests`: arrencada de context.
- `CalculadoraVidaTest`: formula de vida.
- `PersonatgeServiceTest`: validacions i calcul de fitxa.

Comandes recomanades:

```bash
./mvnw test
node --check src/main/resources/static/js/mapa/*.js
python3 -m json.tool data/personatges/901.json >/dev/null
python3 scripts/auditar_campos_referencia.py
```

Si `./mvnw` ha de descarregar dependències, cal xarxa. En entorns restringits pot fallar sense que el codi estigui malament.

## 10. Index de lectura rapida

- Vull entendre regles de personatge: `PersonatgeService`, `FichaPersonatge`, `CalculadoraVida`, `Personatge`.
- Vull tocar cartes: model `TierX`, repository `TierXRepository`, service `TierXService`, controller `TierXController`, templates `cartas/<tipo>/`.
- Vull tocar mapa: `MapaController`, `GeografiaMapaService`, `GeografiaMapa`, `templates/mapa/index.html`, `static/js/mapa/*`.
- Vull tocar aventures heretades: `AventuraController`, `AventuraService`, `AventuraRepository`, `CatalogoAventuraRepository`, `CartaImpresion`.
- Vull tocar el constructor per actes: `ConstructorAventuraService`, `CartaAventura`, `CardKind`, `FichaEstado`, `CartaAventuraForm`, `templates/aventuras/constructor.html`.
- Vull generar contingut: `scripts/` i `docs/Guia_Prompts_Mundo_y_Creacion.md`.
- Vull revisar salut: `DiagnosticoService` i `/diagnostico`.
