## Imported Claude Cowork project instructions

Estoy creando una readaptación de DnD 5e basada en cartas y tiers. No quiero usar niveles tradicionales. Cada elemento importante del personaje debe representarse como una carta: clase, raza, trasfondo, pasiva, habilidades, hechizos, equipo y objetos.

El sistema usa solo cuatro stats: CON, DES, INT y CAR (edición 2 del GDD: CAR sustituye a la antigua SAB). La vida se calcula mediante clase + CON + CAR + tier + equipo, sin tiradas aleatorias, y está acotada por un techo fijo por tier (t1=35 · t2=50 · t3=70 · t4=95 · t5=125) para que ningún personaje, por mucho equipo o bonos que acumule, pueda superar la vida máxima de su tier. Los tanques tienen mucha Constitución y vida alta, los magos y personajes frágiles tienen poca vida, y el equipo sirve para ajustar stats y balancear el personaje. Ver fórmula exacta y tabla de techos en el GDD sección 5.

Cada clase debe tener un rol claro, un set de equipo inicial, una carta pasiva inicial y un número determinado de habilidades o hechizos iniciales. La progresión se hace por tiers, desbloqueando nuevas cartas, mejoras, slots, pasivas o especializaciones.

## Reglas obligatorias de frontend (UI/UX)

**OBLIGATORI: mai CSS inline dins les plantilles Thymeleaf.** Tot el CSS ha d'anar a fitxers `.css` externs sota `src/main/resources/static/css/`. Els `<style>` inline només s'accepten en prototips exploratoris (`*.dc.html` del wireframe), mai a l'app de producció.

Criteris d'organització dels CSS:
- **Un fitxer per àmbit funcional** quan l'estil és específic d'una pantalla/zona: `constructor.css` (constructor d'aventures), `ficha-print.css` (ficha de personaje imprimible), `carta-fisica.css` (cartes físiques per imprimir), `wizard.css` (wizard d'aventures), `mapa.css` + `paneles-overlay.css` (mapa mundi), `app.css` (legacy Bootstrap).
- **`components.css` és el CSS de components generals reutilitzables** que apareixen a múltiples pàgines i comparteixen tipografia/aparença global: botons (`.gm-btn`), cartes (`.gm-card`), panells flotants (`.gm-panel`), badges (`.gm-badge`), toast (`.gm-toast`), selectors CSV (`.gm-csvsel`). **Si un objecte comparteix tipografia o apareix a més d'una pantalla, va aquí, no en un CSS dedicat.**
- **`tokens.css` és la font única de veritat** per colors, tipografia, espaiat i layout. Cap hardcode de hex/px en altres CSS: sempre `var(--...)`.
- **`base.css` és el reset + layout del shell** (header, sidebar, main, drawer mòbil).
- **`utilities.css` són classes utilitàries** (`.u-*`) d'una sola propietat, tipus Tailwind però reduït.

Quan s'afegeix una nova pantalla/feature amb CSS propi, crear un `nom-zona.css` i carregar-lo via `<link>` al `<head>` (o via `extra-css` fragment del layout decorator). Mai incrustar `<style>` al `.html`.

Vull que el sistema mantingui inspiració en DnD 5e, però sense copiar regles exactes ni contingut propietari. Necessito que m'ajudis a dissenyar l'arquitectura del sistema, les regles base, les plantilles de cartes, les classes inicials, el balance, les fórmules de vida, els límits per tier, les restriccions d'equip i els exemples de personatges.
