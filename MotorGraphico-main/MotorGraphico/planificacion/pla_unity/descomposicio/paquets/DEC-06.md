# DEC-06 · Decisió: joc dins l'editor o en finestra a part

Estat: **proposta per revisar**. Cap E de codi autoritzada per aquesta fitxa.

- Consumidor obligatori: `motor_editor → host de playtest aprovat`.
- Dependències de paquet: cap a la font.
- Portes addicionals: DEC06.
- Esforç E identificat: 3 h, provisional; no és un termini.
- Referència anterior: 0 jornades de ~6 h.

Lectures locals (relatives a `MotorGraphico/`, totes contrastades per existència):

- `editor/src/EditorApplication.cpp`
- `examples/juego.cpp`

Les signatures noves són propostes: `string`, `vector`, `optional`, els DTOs i els ports
s’han de resoldre a tipus/namespace del repo en el contracte v1. No crear aquests tipus
automàticament. Cap fitxer de codi està autoritzat mentre la llista de rutes sigui buida.
Les xifres de temps inclouen prova local; una E que no hi cap es torna a dividir.

## DEC-06.R1 · Decisió de procés o vista incrustada

### DEC-06.E01 · Auditar el playtest actual

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: estat real de llançament`.
- Entrades: Editor actual i comanda de joc
- Retorn/lliurable: Contracte existent i què falta per F-01
- Acceptació i casos límit: Revisar decisió de procés separat de la bitàcora abans de reobrir-la
- Dependències E: PREP.E10.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### DEC-06.E02 · Comparar dues opcions de host

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: decisió DEC-06 amb recomanació`.
- Entrades: Procés separat i vista incrustada
- Retorn/lliurable: Cost, aïllament, input, GL i desfer per opció
- Acceptació i casos límit: Recomanació: conservar procés separat mentre no s’aprovi incrustar; no introduir llibreria nova només per comparar
- Dependències E: DEC-06.E01.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### DEC-06.E03 · Registrar resolució i límit d’implementació

- Caixa: **30 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: contracte de host acceptat o pendent`.
- Entrades: Resposta d’Oriol i comparativa
- Retorn/lliurable: F-01 desbloquejat o bloqueig explícit
- Acceptació i casos límit: El silenci no canvia la decisió vigent; una nova opció necessita vist explícit
- Dependències E: DEC-06.E02.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

## Porta de tancament del paquet

E acceptades + integració al consumidor indicat + validació del commit en clon net +
prova de recorregut real quan toca. Una peça implementada amb prova pròpia pot estar
**produïda pendent d’integració**; el paquet continua obert fins a arribar a un executable.
Cap fallada nova; les heretades s’identifiquen i es conserven als logs. L’aprovació del
resultat visual/audible/jugable requereix observació real, amb estat pendent si no existeix.
