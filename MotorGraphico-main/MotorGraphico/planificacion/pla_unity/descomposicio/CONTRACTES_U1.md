# Exemples de contracte E amb entrades i retorn precisos

**Versió de disseny v0, pendent de revisió i de congelar rutes.** Les signatures
següents són propostes concretes, no APIs que ja estiguin implementades. `GridCoord`
sí que existeix a `include/Core/Math/GridCoord.h`. No s’ha escrit C++ de producte.

## U1-03.E02 · Distància de caselles

```cpp
int distanciaCaselles(GridCoord a, GridCoord b) noexcept;
```

- Entrades: coordenades de mapa ja validades, components enters entre 0 i 32.767.
- Retorn: `abs(a.x-b.x) + abs(a.y-b.y)`, unitat **caselles ortogonals**.
- Efectes: cap. No consulta el mapa, el jugador, el rellotge ni el món.
- Errors: no n’hi ha dins del domini anterior. La frontera de mapa valida el
  rang abans de cridar. No ampliar el domini a tot `int` sense revisar overflow.
- Exemples: `(2,3),(2,3) → 0`; `(2,3),(5,3) → 3`; `(2,3),(5,5) → 5`.
- Propietats: simetria i resultat no negatiu. La prova de diagonal ha de rebutjar
  una implementació accidentalment euclidiana o Chebyshev.
- Caixa proposada: 30 min amb prova. Rutes i namespace es congelen amb el
  mòdul de percepció; no afegir un membre a `Application` per fer aquesta funció.
- Consumidor: detecció i abandonament de la mateixa política; ús real mitjançant
  `GameSession::update`. No tancar el paquet amb una funció que només crida un test.

La mètrica és una **recomanació nova**, perquè «a quatre caselles» no resol per
si sola si la diagonal compta una o dues. L’acceptació de la política la fixa.

## U1-03.E03 · Camp visual

```cpp
// Tipus proposat del contracte, mapat explícitament al rumb real en integrar.
enum class Rumb { Nord, Est, Sud, Oest };
bool dinsCampVisual(GridCoord origen, Rumb rumb, GridCoord objectiu) noexcept;
```

- Entrades: coordenades del domini anterior i un valor enumerat vàlid.
- Retorn proposat: semicamp de front i costats inclosos. Nord: `dy<=0`;
  Est: `dx>=0`; Sud: `dy>=0`; Oest: `dx<=0`.
- La convenció Nord=`y` decreixent s’ha de confirmar contra el rumb d’EnemyBrain;
  no copiar la numeració de PNJ sense verificar-la.
- La mateixa casella dona `true`; l’operació no decideix si es pot combatre.
- No calcula distància ni línia de visió: són paràmetres d’altres operacions.
- Exemples amb origen `(5,5)`, Nord: `(5,3) → true`, `(7,5) → true`,
  `(5,6) → false`. Cas simètric per les quatre orientacions.
- Prova negativa: invertir Nord/Sud ha de fallar. Caixa: 60 min amb prova.

Aquest tall permet provar orientació independentment de llindars i col·lisions.
No s’encarrega a l’agent «fes un sistema de visió».

## U1-03.E04 · Detecció amb dades explícites

```cpp
struct Percepcio {
    GridCoord origen;
    Rumb rumb;
    GridCoord objectiu;
    bool liniaDeVisioLliure;
};
struct PoliticaPercepcio {
    int radiAdquisicio;
    bool exigeixLiniaDeVisio;
};
bool potDetectar(const Percepcio& observacio,
                const PoliticaPercepcio& politica) noexcept;
```

- Política validada abans de cridar; radi entre 0 i 32.767.
- Retorna `true` només si distància `<= radi`, camp visual vàlid i, quan es
  demana, línia de visió lliure. Combina E02 i E03, sense repetir-ne la regla.
- No obté globals ni calcula el raycast: qui coneix el mapa passa el resultat.
- Exemples amb radi 4: a distància 4 de front i lliure → `true`; a 5 → `false`;
  darrere a 2 → `false`; de front amb mur i exigència activa → `false`.
- Si l’exigència és falsa, el mur no afecta aquesta operació. Decidir si es
  vol permetre aquest cas és una decisió de producte, no de l’implementador.
- Error plausible: usar `<4` en lloc de `<=4`; la prova de frontera ho detecta.
- Caixa: 60 min. Consumidor: transició d’IA, a través del tick real de la sessió.

Els DTOs tenen quatre i dos camps; no formen un controlador que acumuli estat.
Aquest contracte no prescriu persistir la persecució ni canviar schema 1.

## U1-03.E06 · Següent pas: no imposar una regla falsa

```cpp
// ConsultaPas és conceptual fins a contrastar el navegador real del repo.
std::optional<GridCoord> seguentPas(GridCoord origen, GridCoord desti,
                                   const ConsultaPas& consulta);
```

El contracte v1 haurà de fixar bounds, ocupació, ordre dels veïns, cost i pressupost
de cerca. El retorn és un **veí ortogonal transitable** d’una ruta vàlida, o cap
pas si no hi ha ruta o s’ha consumit el pressupost acordat, amb el motiu separat
si el consumidor el necessita. No confondre objectiu assolit amb cerca interrompuda.

Un rodeig vàlid pot augmentar la distància Manhattan en el primer pas. Per tant,
«cada pas redueix Manhattan» no és una acceptació correcta davant d’un mur en U.
La prova ha de comprovar una ruta vàlida i finita, l’altura i les ocupacions.
Aquesta E continua pendent fins que la consulta i els límits siguin tipus reals.

## U1-02 · Identitat estable no significa adreça estable

`Engine::EntityHandle` i `HandlePool<T>::Handle` són dos tipus diferents amb
índex/generació. El pool guarda slots en `vector`; un creixement pot moure els
elements. `World` té `deque` i comenta explícitament que una alta durant un
callback no ha d’invalidar referències existents.

Abans d’encarregar la substitució s’ha de triar una política que preservi aquesta
garantia. Una E sobre `get(handle)` pot provar nul·litat per generació antiga,
però no demostra que una referència dins `eachEntity` sobrevisqui a `create()`.
La prova específica força creixement durant el callback i conserva la identitat
del segon enemic després de destruir el primer.

No s’autoritza copiar un altre pool, desfer la composició Enemy/EnemyBrain,
guardar punters a la partida ni canviar les regles de joc dins el refactor.
