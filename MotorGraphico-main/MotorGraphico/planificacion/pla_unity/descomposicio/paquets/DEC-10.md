# DEC-10 · Decisió: quina pàgina d'event guanya

Estat: **proposta per revisar**. Cap E de codi autoritzada per aquesta fitxa.

- Consumidor obligatori: `ActivePageWithInventory → MapEventRunner → juego`.
- Dependències de paquet: cap a la font.
- Portes addicionals: DEC10.
- Esforç E identificat: 3 h, provisional; no és un termini.
- Referència anterior: 0 jornades de ~6 h.

Lectures locals (relatives a `MotorGraphico/`, totes contrastades per existència):

- `include/RPG/MapEventRunner.h`
- `src/RPG/Eventos/CondicionesYPaginas.cpp`

Les signatures noves són propostes: `string`, `vector`, `optional`, els DTOs i els ports
s’han de resoldre a tipus/namespace del repo en el contracte v1. No crear aquests tipus
automàticament. Cap fitxer de codi està autoritzat mentre la llista de rutes sigui buida.
Les xifres de temps inclouen prova local; una E que no hi cap es torna a dividir.

## DEC-10.R1 · Decisió de pàgines i rearmament

### DEC-10.E01 · Comparar política real i opcions

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: taula de decisió DEC-10`.
- Entrades: Selector existent i 3 events amb pàgines simultànies
- Retorn/lliurable: Opcions amb conseqüència i recomanació explícita
- Acceptació i casos límit: Separar quina pàgina guanya de quan es repeteix Autorun/Parallel; no decidir pel nom
- Dependències E: PREP.E10.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### DEC-10.E02 · Fixar exemples entrada-sortida

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: fixtures de prioritat i rearmament`.
- Entrades: Pàgines, condicions fiables/desconegudes i memòria d’execució
- Retorn/lliurable: Taula aprovada o marcada pendent
- Acceptació i casos límit: Dues pàgines certes, cap certa, error de condició, pàgina que no canvia, sortida/reentrada
- Dependències E: DEC-10.E01.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### DEC-10.E03 · Registrar el vist de la política

- Caixa: **30 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: resolució d’Oriol o estat pendent`.
- Entrades: Opcions i resposta humana si arriba
- Retorn/lliurable: Contracte acceptat o bloqueig declarat
- Acceptació i casos límit: No convertir temps transcorregut ni silenci en aprovació; no executar U3-01 abans del contracte
- Dependències E: DEC-10.E02.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Cap fora de l’àmbit congelat; els efectes externs s’injecten. Només la tasca d’integració modifica el consumidor.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

## Porta de tancament del paquet

E acceptades + integració al consumidor indicat + validació del commit en clon net +
prova de recorregut real quan toca. Una peça implementada amb prova pròpia pot estar
**produïda pendent d’integració**; el paquet continua obert fins a arribar a un executable.
Cap fallada nova; les heretades s’identifiquen i es conserven als logs. L’aprovació del
resultat visual/audible/jugable requereix observació real, amb estat pendent si no existeix.
