# MON-01 i VEG-01 — Els edificis grans i la vegetació amb el kit (05/10/2026)

L'Oriol demana els edificis grans i la vegetació amb l'estètica d'Habbo, com
la resta de la ciutat. Les respostes:

- **Mida:** la de l'empremta actual.
- **Disseny:** cada monument conserva la seva identitat i hi suma la geometria
  d'Aegroum.
- **Vegetació:** tota.

En veure el primer castell, l'Oriol va demanar mantenir més l'essència dels
d'abans (formes i colors) i afegir-hi els anells que suren i els elements de
fantasia.

## Els set monuments (`tools/iso/monuments_ciutat.py`)

| Monument | Empremta | Com era i com és | La fantasia |
| --- | --- | --- | --- |
| Castell | 13×10 | recinte quadrat de pedra grisa, merlets, quatre torres rodones amb el con de pissarra blava, porta d'arc | anells d'or que suren al voltant dels cons, puntes de cristall, el cristall dels Guardians surant al pati |
| Òpera | 10×9 | pedra clara amb pilastres, gran teulada de pissarra blava amb cordons d'or, lucanes, pòrtic de columnes | claraboia de vidre i una corona d'or i cristall que hi sura a sobre |
| Universitat | 10×9 | pedra grisa gòtica, teulades de coure verd, pavelló amb el rellotge, torre emmerletada amb agulla | dos anells d'or a l'agulla, punta de cristall, orbe de llum sobre el pavelló |
| Biblioteca | 12×9 | bloc de terrat pla amb merlets, òculs, nau de teula vermella fins al pòrtic, cúpula sobre tambor | llibres que suren en rotllana al voltant de la cúpula, anell d'or, alambins de cristall |
| Església | 10×9 | pedra fosca, campanar amb pinacles i creu, rosassa, sostre esfondrat (en queden les bigues) | el cercle de ciris dels Perduts i un anell de pedra trencat que encara sura |
| Coliseu | 8×6 | amfiteatre oval amb tres pisos d'arcs i grades, mig caigut pel darrere | anells d'or sobre la sorra, pals amb gallarets, braseres de cristall |
| Quarter | 11×8 | fortí de pedra grisa amb merlets, barracó, torre de guaita amb teulat de pissarra | veles de senyals al pal de la torre, anell d'or, punta de cristall |

Els de PixelLab (lot 10) queden retirats al seu índex, però es conserven.
Sota cada monument, on no tapa l'empremta, es veu el terra del carrer.

## La vegetació (`tools/iso/vegetacio.py`, `bardissa.py`, `sols.py`)

- **Arbres:** roure (dues variants), fruiter (pomes vermelles o fruits grocs),
  avet (dos), salze amb les tires que pengen i arbre sec. Tenen la capçada de
  lòbuls amb bandes de llum netes i la vora fosca entre lòbuls.
- **Mates (dues), flors (dues) i joncs.** El generador alterna les variants
  segons la cel·la.
- **Bardisses:** un bloc de boix sense línia entre bloc i bloc, de manera que
  una filera es veu com una sola bardissa.
- **Empalissada del Barrio Militar:** ara és de troncs, amb portell.
- **Gespa i herba amb flors:** periòdiques, sense quadrícula.

## Fitxers

- `vm_00_vegetacio.png`: tota la vegetació amb la Skilla a escala.
- `vm_01_monuments_abans_ara.png`: els set monuments, el de PixelLab al costat
  del del kit. `vm_09` és el castell sol.
- `vm_02` a `vm_08`: captures del joc (jardins de Barrios Altos, Pico Dragón,
  ribera de Surysal, castell i òpera, església i coliseu, universitat i
  biblioteca, quarter i empalissada).

## Com es refà

```
python3 tools/iso/exportar_monuments.py
python3 tools/iso/exportar_decorats.py
python3 tools/iso/bardissa.py
python3 tools/pixellab/importar_bloques.py
python3 tools/iso/sols.py
python3 tools/iso/rajoles_kit.py
python3 tools/pixellab/importar_tiles.py assets/textures/pixellab/tiles.json
python3 tools/pixellab/importar_monumentos.py
python3 tools/gen_ciudad.py
```
