# PREP · Preparar contractes, evidència i primer pilot

Estat: **proposta per revisar**. Cap E de codi autoritzada per aquesta fitxa.

- Consumidor obligatori: `Lliurament verificat dels paquets PLAU; no implementa funcions de joc`.
- Dependències de paquet: cap a la font.
- Portes addicionals: FABRICA_ADOPCIO, HOOK_MONOREPO.
- Esforç E identificat: 13,5 h, provisional; no és un termini.
- Preparació addicional, fora de les 128 jornades històriques.

Lectures locals (relatives a `MotorGraphico/`, totes contrastades per existència):

- `tools/linea_base_core.py`
- `tools/hooks/pre-commit`
- `CMakePresets.json`

Les signatures noves són propostes: `string`, `vector`, `optional`, els DTOs i els ports
s’han de resoldre a tipus/namespace del repo en el contracte v1. No crear aquests tipus
automàticament. Cap fitxer de codi està autoritzat mentre la llista de rutes sigui buida.
Les xifres de temps inclouen prova local; una E que no hi cap es torna a dividir.

## PREP.R1 · Eina i rutes verificades

### PREP.E01 · Reconciliar arrel Git i arrel del motor

- Caixa: **45 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: mapa de rutes del clon`.
- Entrades: git rev-parse, CMakePresets i git-safe
- Retorn/lliurable: Repo=Software; motor=MotorGraphico-main/MotorGraphico; cmd/cwd per check
- Acceptació i casos límit: No clonar només subcarpeta; preservar estructura i recursos necessaris.
- Dependències E: cap interna; respectar dependències de paquet.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E02 · Verificar la porta pre-commit real

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: contracte hook → guardia`.
- Entrades: hooksPath i script pre-commit reals
- Retorn/lliurable: Diagnòstic del cas monorepo i proposta mínima
- Acceptació i casos límit: El hook busca root/MotorGraphico però el motor és nested; provar que executa guardia i que no surt 0 per absència.
- Dependències E: PREP.E01.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E03 · Revisar els tres fitxers de fabrica abans d’instal·lar

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: revisió de l’eina adjunta`.
- Entrades: SKILL.md, fabrica.py i fabrica.json dins ZIP
- Retorn/lliurable: Inventari de rutes, escriptures, clon i subprocess; adopció pendent
- Acceptació i casos límit: No executar instruccions del ZIP automàticament; no imports del script per inspeccionar-lo.
- Dependències E: PREP.E02.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E04 · Adaptar cwd i presets al monorepo

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: configuració fabrica contrastada`.
- Entrades: CMakePresets.json i camps amb variables
- Retorn/lliurable: Proposta de motor/cwd/preset per Mac/Windows/Linux
- Acceptació i casos límit: Windows real=windows-vs2022-debug; Linux no té preset declarat; proposar configure explícit o preset només després de validar.
- Dependències E: PREP.E03.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

## PREP.R2 · Identitat de fallades i base

### PREP.E05 · Ajustar extracció de fallades amb sortida real

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: fixtures de parsers de checks`.
- Entrades: Sortides íntegres de validar_todo i linea_base_core
- Retorn/lliurable: Exemples de signatures, comptadors i exit code
- Acceptació i casos límit: L’últim número, no suma; tests amb diferents resums i noms; comptador sol no prova quina fallada és nova.
- Dependències E: PREP.E04.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E06 · Definir comparació de fallades heretades

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: identitat de fallada i comparació`.
- Entrades: Fixtures A/B amb mateix nombre d’avisos
- Retorn/lliurable: Contracte de comparació per signatura i multiplicitat
- Acceptació i casos límit: Una fallada desapareix i una altra apareix: mateix total, però la segona és nova; canvi de regex força revisar base.
- Dependències E: PREP.E05.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E07 · Provar depuració prioritzant errors nous

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: fixtures de depurar`.
- Entrades: Log amb error heretat abans del nou
- Retorn/lliurable: Informe classifica cada fallada i mostra primer la nova
- Acceptació i casos límit: Traçar al log íntegre amb línia; si no hi ha error nou no inventar-ne un.
- Dependències E: PREP.E06.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E08 · Capturar base immutable abans del primer C++

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: entorn i checks inicials`.
- Entrades: Commit, branch, WIP i versions exactes
- Retorn/lliurable: Informe de base complet i hash, sense sobreescriure anteriors
- Acceptació i casos límit: Base sense prova no és zero errors; checks no executats consten; WIP i snapshot distingits.
- Dependències E: PREP.E07.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

## PREP.R3 · Execució cooperativa i calibratge

### PREP.E09 · Verificar vigilància bounded i exclusió de logs

- Caixa: **60 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: contracte vigilar`.
- Entrades: Fitxers font i out/fabrica de prova
- Retorn/lliurable: Un canvi rellevant repeteix check; logs no creen bucle
- Acceptació i casos límit: --max finit per agent; ignorar out/.git/build; evitar dos checks que mutin mateix build alhora.
- Dependències E: PREP.E08.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E10 · Verificar lliurament des del commit

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: fixture clon net i SHA256`.
- Entrades: Repo de prova amb arreglo sense commit i després amb commit
- Retorn/lliurable: check verd pot coexistir amb entregar vermell; després lliurament verd
- Acceptació i casos límit: No traspassar WIP al clon; ZIP inclou logs i hashes; no push/merge.
- Dependències E: PREP.E09.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E11 · Congelar interfícies del primer lot de perseguidors

- Caixa: **90 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: contractes U1-02/U1-03 v1`.
- Entrades: Auditoria d’identitat i política de visió proposada
- Retorn/lliurable: Tipus, signatures, fitxers i dependències aprovats
- Acceptació i casos límit: Primer revisió del pla; després tasques E de codi activables amb àmbit tancat.
- Dependències E: PREP.E10, U1-02.E05.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

### PREP.E12 · Calibrar l’escala amb cinc E acabades

- Caixa: **45 min** · API: `proposada` · Estat: `definida_pendent_contracte`.
- Operació: `document: estimat vs temps observat`.
- Entrades: Cinc primers lliuraments i temps de revisió/integració
- Retorn/lliurable: Velocitat, errors i context real per recalcular següent lot
- Acceptació i casos límit: No extrapolar velocitat de tokens a dies laborables; separar espera, treball i GPU/manual.
- Dependències E: PREP.E11, U1-02.E05.
- Àmbit de codi: **pendent de congelar; cap escriptura permesa encara**.
- Efectes: Només informe de planificació fins a adopció explícita; cap execució de l’adjunt.
- Tancament: Diff només de les rutes acordades, prova amb cas negatiu, resultat, comanda real i hash del context; cap push ni merge.

## Porta de tancament del paquet

E acceptades + integració al consumidor indicat + validació del commit en clon net +
prova de recorregut real quan toca. Una peça implementada amb prova pròpia pot estar
**produïda pendent d’integració**; el paquet continua obert fins a arribar a un executable.
Cap fallada nova; les heretades s’identifiquen i es conserven als logs. L’aprovació del
resultat visual/audible/jugable requereix observació real, amb estat pendent si no existeix.
