# Compañeros en combate, el crítico de dos dados y la escalera de fuerza · 06/10/2026

Lo que respondiste y pediste después. Está **aplicado en el árbol del Mac, sin commit**. El
registro de la sesión está en `sesiones/2026-10-06-claude-acompanantes-dificultad.md` y el
seguimiento en `planificacion/seguimiento/ACOMPANANTES-01.json`.

El estudio de dificultad, con las tablas, va en un documento vivo aparte: «Dificultad: la
escalera de fuerza».

## Qué mirar

`capturas/` (el juego real, en una copia del prólogo con un combate de prueba; Xila cura antes
de lo normal para que se vea):

| | |
|---|---|
| 01 | Tres devotos y, en el orden de turno, Venides (su vida debajo) y Xila (sus dos curas en verde) |
| 02 | La tirada de Venides («1/2»), con el marco en él; Xila ha curado a Nerea |
| 03 | El devoto al que pegó Venides va a por Venides; los otros, a por Nerea |
| 04 | Xila gasta su última cura; el devoto caído sale tachado |

## Qué ha cambiado

1. **El crítico de «todos 6» pide dos dados.** Un 6 con un dado es un éxito (10), no 22.
2. **Acompañantes de la historia.** La aventura dice quién puede pelear contigo y cada combate
   pide los suyos:
   - actúan solos después de tu turno: Venides da dos golpes al último enemigo; Xila cura
     1d4+1 a quien esté en apuros, dos veces en toda la noche;
   - los enemigos van a por quien les ha hecho daño el último;
   - a Xila no se le ataca; Venides se retira a 0 PV; si caes tú, se pierde.
   El compañero personalizable que se creará con el personaje es otra cosa: queda para después.
3. **El Ocaso pelea con Venides al lado** en sus siete combates, y con Xila si le conociste el
   primer día.
4. **Los dados de los enemigos van en los datos** (`combat.stats`): el devoto, el fanático y el
   carcelero del Ocaso ya están en su escalón.
5. **El simulador** (`simulador_dificultad`, en `build/`) rehace las cuentas cuando cambien las
   cartas, el equipo o los enemigos.

## Lo que dicen los números

Con el kit del cofre, contra tres enemigos de 12 PV y un dado de defensa (el escalón F2):

| | Cae el héroe |
|---|---|
| 1 contra 3 | 41 % |
| 2 contra 3 | 5,5 % |
| 3 contra 3 | 1,6 % |
| 3 contra 3, un escalón más fuertes | 4 % |

Cada nivel de equipo pide un escalón más: con el máximo de Tier 1 (4, 4, 2, 2) el 40 % lo da el
escalón F5, que es como los monstruos de Tier 1 del catálogo. La raza pesa más que la clase
(del 4 % al 88 % en 1 contra 3).

## Para Oriol

1. ¿Fijamos la escalera F1-F8 y la dificultad como el escalón del enemigo respecto al equipo?
2. ¿Las cartas tiran con el atributo principal de la clase? Hoy todas tiran con DES.
3. ¿Bajamos el peso de la CON (vida ×3 y dados de defensa)?
4. ¿Gorran más duro? Hoy mata un 4 % de las veces.
5. Siguen abiertas: los topes 2, 4, 6, 8, 8 y 8, y si ponerse el kit cura.
