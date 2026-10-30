# pixel (PixelLab) — tarea actual
**Asignada:** 14/10 por Claude · **Quién la ejecuta:** Oriol con PixelLab (o su IA), que es la suscripción para los sprites · **Plan:** `planificacion_alcance_y_vision/` AV14-13, AV14-10 y AV14-18 · `COMBATE_BG3.md` §13

Oriol (14/10): «estoy pagando el Pixel para que hagamos sprites»; «haz sentarse a los personajes o haz animaciones más dignas»; «la animación de andar es horrible». Hoy los personajes de la historia salen de capas (`tools/actores_pnj.py`) con un ciclo de **3 frames** por dirección. Con PixelLab se hacen como los actores que ya hay (Luisarda, Ben Kafka…: `assets/textures/pixellab/actores.json`), que el importador ya sabe leer.

## Cómo se entrega (igual que los lotes de antes)
- **Carpeta:** `MotorGraphico/assets/textures/pixellab/lote16/<id>/`, una por personaje.
- **Tamaño y fondo:** lienzo de 64 px, fondo transparente, los pies abajo (como los de `lote0/luisarda`).
- **Nombres:** como los exporta PixelLab: `south.png`, `north.png`, `east.png`, `west.png` (quieto) y `<animacion>_<dir>_<n>.png` (`walk_south_1.png`…).
- **Personajes y aspecto:** el que tienen hoy en el juego. Lo tienes en `assets/textures/pnj_boundington*.png`; el canon, en `planificacion/CANON_BOUNDINGTON_CAMPANA01.md` §13.
- Claude los importa con `tools/pixellab/importar_actores.py` y los pone en el juego.

## 1. Primero: las poses (AV14-13)
Para estos personajes:
- **Venides**, de joven (756) y de caballero (757);
- **Verina, Edul y Sandes** (756);
- **Sho-Noco**;
- **la tabernera de la Taberna Humilde**;
- **dos parroquianos.**

| Animación | Frames | Direcciones | Para qué |
|---|---|---|---|
| `sit` (sentado, quieto) | 1–2 | las 4 | sillas y bancos de las tabernas |
| `toast` (brindar, sentado) | 4 | abajo y derecha | el brindis de Ceaseton, 756 |
| `write` (escribir, sentado) | 4 | abajo | Venides con el cuaderno |
| `knock` (llamar a la puerta) | 4 | izquierda y derecha | la puerta de 757 |
| `lean` (apoyado en la barra) | 1–2 | las 4 | taberna |
| `sleep` (dormido sobre la mesa) | 2 | abajo | el pescador de Ceaseton |

## 2. Después: andar de verdad (AV14-10)
- **Ciclo de andar de 6 frames** (`walk`) en las 4 direcciones, para los mismos personajes y para los **cuerpos base** del creador (empieza por humano y elfo; la lista de razas está en `assets/textures/razas_andando.json`).
- Si PixelLab ofrece 8 frames, mejor 8.

## 2b. Para el prólogo (AV14-21, Oriol 14/10, D0015 §12)

Los tres amigos de 757 (los que hoy llevan capucha morada): la misma figura **con túnica y sin
capucha** (4 direcciones, quieto y andar), y **el gesto de ponerse la capucha** (3–4 frames, mirando
abajo). La cinemática los enseña sin capucha, se la ponen y salen andando. Entrega en
`lote16/amigos_757_sin_capucha/` con el formato de arriba.

## 3. Por último: iconos (AV14-18)
Iconos de 32 px, en `lote16/iconos/`:
- cada objeto a la venta en las tiendas de Boundington (`assets/objects/boundington_objetos.json`, campo de tienda);
- la economía del turno: acción, movimiento, canalización, reacción y terminar turno;
- los tipos de daño: cortante, contundente, perforante, fuego, frío, rayo, ácido, veneno, sagrado, profano y psíquico.

## 3b. Emblemas de los lugares de Boundington (AV15-02b)

Encargo de datos de Codex, tras la aprobación PO-39…41 de Oriol (14/10, 13:01).
**Estos once son de 16×16 px**, fondo transparente, un objeto reconocible en
cinco segundos, sin letras ni símbolos abstractos. Mantener el contorno oscuro
y la paleta del atlas `assets/textures/iconos.png`. Entrega en
`assets/textures/pixellab/lote16/iconos/` con los nombres siguientes.

| Archivo / futuro ID | Objeto | Lugar y función |
|---|---|---|
| `lugar_tina.png` / `lugar_tina` | Tina con agua | Las Aguas Claras · baños y descanso |
| `lugar_torre.png` / `lugar_torre` | Torre | Castillo de Boundington · corte y audiencia |
| `lugar_yunque.png` / `lugar_yunque` | Yunque | Lingote de Oro (Aigren) · forja y reparación |
| `lugar_campana.png` / `lugar_campana` | Campana | Iglesia de la Luz · culto y oración |
| `lugar_cesta_pan.png` / `lugar_cesta_pan` | Cesta con pan | El Peso Justo · víveres y provisiones |
| `lugar_laud.png` / `lugar_laud` | Laúd | El Ruiseñor · ópera y teatro |
| `lugar_cama.png` / `lugar_cama` | Cama | Taberna Rica (`interior_posada`) · cama y comida |
| `lugar_carrete.png` / `lugar_carrete` | Carrete de hilo | La Boutique de Madame (Luisarda) · sastre y tela |
| `lugar_jarra.png` / `lugar_jarra` | Jarra | Taberna Humilde · bebida y comida |
| `lugar_pergamino.png` / `lugar_pergamino` | Pergamino | La Casa del Saber · estudio y enseñanza |
| `lugar_tetera.png` / `lugar_tetera` | Tetera | La Hoja Verde · té y reposo |

**Laúd y pergamino:** Oriol pide la prueba de cinco segundos cuando exista el
dibujo; evaluar si se reconoce el uso del lugar. No se da esa revisión por hecha.

Hasta que existan y se incorporen al atlas, `emblema` es `""` en `lugares.json`:
los IDs anteriores son encargos, no referencias publicadas. Claude incorporará
el dibujo al atlas y pondrá su ID en los datos. El atlas es generado; no editar
`iconos.json` a mano.

Ya se reutilizan seis objetos del atlas: `oro` (moneda), `lore` (libro abierto),
`ranura_cabeza` (casco), `ranura_accesorio` (anillo), `ranura_mano_principal`
(espada) y `ranura_mano_izquierda` (escudo). No se solicitan dibujos duplicados.

**Hecho cuando:** cada carpeta está entregada y en el INFORME pones qué personajes y animaciones has hecho y cuáles no se pudieron (y por qué).

---
## INFORME (lo rellena el agente al terminar)
- **Estado:** hecho / a medias / bloqueado — por qué
- **Rama y commits:** `agente/...` · hashes
- **Qué ha cambiado:** ficheros y en una frase cada uno
- **Cómo lo pruebo (comandos exactos, copiar y pegar):**
- **Qué tengo que mirar yo (Oriol):** decisiones tomadas, dudas, riesgos
- **Fichero de sesión:** `sesiones/...`
