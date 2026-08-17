# Convencions de llengua i codi

Aquest projecte queda amb una regla simple:

- Documentació, guies, README, Javadocs i comentaris: català.
- Codi nou: anglès per a noms de classes, mètodes, variables, camps JSON nous, constants, fitxers i APIs.
- Codi existent: no es reanomena només per traduir-lo. Molts noms actuals (`Aventura`, `Personatge`, `cartas`, `hechizos`, `ConstructorAventuraService`) ja formen part de rutes, dades JSON i plantilles; canviar-los sense una migració formal trencaria el projecte.
- Text visible de la UI: català quan sigui una eina interna del projecte; només es manté en castellà si forma part d'un contingut narratiu, d'una carta ja escrita o d'una dada importada.
- Prompts i contingut de món: català per a instruccions i documentació; els valors de sistema o claus JSON han de seguir l'esquema tècnic definit.

## Exemple de codi nou

```java
public class AdventureCardValidator {
    public boolean isAlwaysIncluded(AdventureCard card) {
        return card.kind() == CardKind.BASE || card.kind() == CardKind.INJECTED;
    }
}
```

## Exemple de comentari correcte

```java
// Manté compatibilitat amb les aventures antigues sense cartes per actes.
```

## Migracions futures

Si algun dia es vol passar el domini sencer a anglès, cal fer-ho com una migració controlada:

1. Inventariar rutes, noms de camps JSON, plantilles i scripts afectats.
2. Afegir aliases Jackson o adaptadors temporals.
3. Migrar dades amb script idempotent.
4. Executar tests i diagnòstic.
5. Eliminar compatibilitat antiga només quan les dades ja estiguin netes.

