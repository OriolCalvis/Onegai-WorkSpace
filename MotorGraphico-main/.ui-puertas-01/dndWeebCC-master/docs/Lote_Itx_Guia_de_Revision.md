# Lot de prova "Itx" — guia de revisió

*Contingut sonda per veure què mostra bé la interfície i què no.*

Tot el que genera `scripts/generar_lote_itx.py` porta **Itx** al nom i **itx_** a l'id, així
que es filtra d'un cop d'ull a qualsevol àlbum i es retira amb una sola ordre.

```bash
python3 scripts/generar_lote_itx.py            # crear
python3 scripts/generar_lote_itx.py --limpiar  # retirar-ho tot
```

---

## 1. Què hi ha i on mirar-ho

| Peça | Ruta a l'app | Què s'hi prova |
|---|---|---|
| Itx Centinela de Salmuera | `/cartas/enemigos` | Criatura completa: atacs amb defensa, pasiva, condicions i botí |
| Itx Matriarca de la Sal | `/cartas/enemigos` | **Jefe amb tot**: 3 fases, 2 accions legendàries, arena, perfil de villà, `storyCard` i `lootTable` |
| Itx Alijo de Sal | `/tesoros` | Drops amb **condició** (el camp que la majoria de taules deixen buit), or i pista |
| Itx Red de Salmuera | *(sense pantalla pròpia)* | Trampa amb les **dues tirades** diferents: detecció i desarmament |
| Itx Guardián de Salmuera | `/cartas/invocaciones` | Invocació amb dos atacs i pasiva |
| Itx Adiel de Mijorn | `/npcs` | PNJ amb `services` i `secretHook` plens |
| Itx Marea Negra | `/eventos` | **Primer esdeveniment del catàleg**: disparador, efectes i 3 opcions de jugador |
| Itx La Marea que Recuerda | `/historias` | Història **multi-escena**: 2 escenes amb repartiment i accions→conseqüències |
| Itx El Pacto de Sal | `/historias` | Història **plana** a propòsit: per comparar amb l'anterior |
| Itx Vareda de Mijorn (901) | `/personatges` | Personatge tier 3 amb els 6 slots d'equip, dots, hechizo i les **7 eleccions narratives** |
| Itx Borrén del Muelle (902) | `/personatges` | Personatge tier 2, rol tanc, per contrastar |
| 11 habilitats Itx | `/cartas/habilidades` | Cadena `evolvesInto` completa de tier 3 a 5 per a *tejedora de mareas* |

Estan **enllaçades entre elles**: la història apunta al jefe i al seu botí, el PNJ guarda
el secret de la història, el jefe deixa caure la taula de botí, l'habilitat invoca la
invocació i els personatges fan servir cartes reals del catàleg. Si un enllaç es
visualitza malament, es veurà navegant d'una peça a l'altra.

---

## 2. Cobertura de camps

Comprovat automàticament: cap peça deixa camps buits, tret de dos casos volguts.

| Peça | Camps | Buits |
|---|---:|---|
| Enemic jefe | 22 | cap |
| Personatge | 22 | cap |
| Història multi-escena | 16 | cap |
| Història plana | 16 | `escenas` *(a propòsit, per comparar)* |
| PNJ | 13 | cap |
| Trampa | 13 | cap |
| Invocació | 12 | cap |
| Esdeveniment | 10 | cap |
| Botí | 8 | cap |
| Habilitat d'invocació | 21 | `defenseStat` i `evolvesInto` *(una invocació no obliga a tirada de defensa ni evoluciona)* |

---

## 3. Què buscar en pantalla

Preguntes concretes per anotar mentre es revisa:

1. **Fitxa del jefe** — les 3 fases, les 2 accions legendàries i l'arena, ¿es veuen totes
   sense haver d'obrir acordions? ¿El perfil de villà destaca prou?
2. **Taula de botí** — els drops amb `condition` ¿mostren la condició, o només el
   percentatge? És el camp que més contingut real deixa buit.
3. **Història multi-escena** — la línia de temps ¿es llegeix bé amb dues escenes? ¿Els xips
   del repartiment enllacen a les fitxes correctes?
4. **Comparació entre les dues històries** — la plana ¿queda visualment coixa al costat de
   la multi-escena, o s'aguanta?
5. **Personatge 901** — les 7 eleccions narratives ¿es mostren totes a la fitxa? ¿La
   silueta d'equipament omple els 6 slots?
6. **Esdeveniment** — les 3 opcions de jugador amb requisit i conseqüència ¿es distingeixen
   bé entre elles?
7. **Cadena d'habilitats** — des de la de tier 3, ¿es pot arribar a la de tier 4 clicant
   `evolvesInto`, o l'id apareix en brut?

---

## 4. El que ja ha detectat aquesta sonda

Trobat en construir el lot, abans fins i tot d'obrir l'aplicació:

1. **`villainProfile.vinculo` no apunta enlloc.** Els 50 jefes del catàleg tenen aquest
   camp, i **cap** conté un id de deïtat real: hi ha ids inventats (`deity_wolf_king`,
   `deity_veiled_queen`), ids de classe (`hoja_venenosa`, `cambiaformas`) i ids de facció
   (`hechiceros_del_vacio`). El camp es comporta com a text lliure. La Matriarca Itx fa
   servir a propòsit `death`, un id real, per ensenyar com hauria de quedar.
2. **`data/eventos/` estava completament buit**, tot i tenir CRUD, model i pantalles. Itx
   Marea Negra és el primer esdeveniment del catàleg.
3. **Cap història del catàleg tenia `escenas`.** El camp existeix al model des de fa poc,
   però les 300 històries generades en lot són planes.
4. **`cost` continua present a les 458 habilitats** com a llegat de l'edició 1, sempre amb
   `resource: none, amount: 0`. No fa mal, però embruta cada fitxa.
5. **Hi ha una classe brossa al catàleg**: `vsdvsvsd`, sense cap habilitat associada.
   Convé esborrar-la.
6. **Dos personatges de proves amb noms brossa** (`sdfsdfsd`) a `data/personatges/`.

---

## 5. Després de revisar

Anota què falla i, quan hagis acabat:

```bash
python3 scripts/generar_lote_itx.py --limpiar
grep -rl 'itx_' data/cartas/habilidades | xargs rm     # habilitats Itx generades
```

O deixa-ho posat: el prefix fa que no es confongui mai amb contingut de joc real.
