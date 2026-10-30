# Encàrrec E — contracte mínim delegable

No enviar la plantilla amb camps pendents. Una E no passa a `preparada` fins a
tenir contracte v1, rutes autoritzades i prerequisits acceptats. Aquest paquet
de context complementa les instruccions obligatòries del repo; no les substitueix.

```text
ID i resultat observable:
Paquet pare / consumidor real / E d’integració:
Versió del contracte: 1
Base: commit + SHA256 dels fragments efectivament llegits; indicar WIP si n’hi ha.
Caixa: __ minuts (màxim proposat 90; inclou prova local).

Operació exacta (namespace, signatura i cabecera):
Tipus auxiliars necessaris, camps, unitats i propietari:
Entrades vàlides / nul·les / absents / fora de rang:
Retorn exacte i significat de cada cas:
Errors: Result<T>, missatge i categoria del log quan pertoqui:
Efectes permesos / estat que es pot modificar:
Vida útil de referències i política de concurrència:
Determinisme, ordre d’iteració i pressupost de recursos:

Fitxers que pots escriure (llista tancada):
Fitxers de context per llegir (fragments de símbols, no el repo sencer):
Dependències acceptades i versions:
Cas positiu amb entrada → sortida:
Cas de frontera amb entrada → sortida:
Cas negatiu amb entrada → error/no-efecte:
Error plausible que la prova ha de detectar:
Comanda de prova real amb cwd i target existents:

Lliura: diff acotat + prova + ordre i resultat + hash de base.
No modifiquis cap altre fitxer ni dissenyis una altra API per desbloquejar-te.
Si falta un tipus, prerequisit o decisió: retorna BLOQUEJADA amb el símbol
que falta, la comprovació feta i el mínim canvi recomanat. No improvisis.
Cap push ni merge. Git d’escriptura sempre amb herramientas/git-safe.sh del workspace.
```

El coordinador omple també `fitxers_permesos`, `contracte_versio=1` i la persona
responsable de la integració. «Fes una classe que gestioni...» no és un encàrrec E.
Una prova de parsing, una funció pura o un adapter petit sí que poden ser-ho,
sempre que aquest contracte en tanqui el comportament.

El retorn de l’agent ha d’ocupar preferentment 10–20 línies, amb enllaços al diff
i als logs íntegres. Si cal més context del previst, es registra per corregir
el tall de les següents tasques; no es continua fent créixer el prompt.
