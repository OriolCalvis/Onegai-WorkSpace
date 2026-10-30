# Desglossament complet S–E

Els temps són caixes provisionals del treball identificat. Cada E enllaça
amb el paquet que conté les seves entrades, retorn i acceptació.

## S · U0 · Preparació addicional · durada indeterminada

### A · A-U0-01 · Preparar evidència i cooperació · 13,5 h identificades

#### B · PREP · Preparar contractes, evidència i primer pilot · 13,5 h identificades

##### C · PREP.R1.C1 · Eina i rutes verificades · 3,75 h identificades

###### D · PREP.R1.C1.D1 · Reconciliar arrel Git i arrel del motor + Verificar la porta pre-commit real + Revisar els tres fitxers de fabrica abans d’instal·lar + Adaptar cwd i presets al monorepo · 3,75 h identificades

- **[PREP.E01](paquets/PREP.md)** · Reconciliar arrel Git i arrel del motor · 45 min
- **[PREP.E02](paquets/PREP.md)** · Verificar la porta pre-commit real · 60 min
- **[PREP.E03](paquets/PREP.md)** · Revisar els tres fitxers de fabrica abans d’instal·lar · 60 min
- **[PREP.E04](paquets/PREP.md)** · Adaptar cwd i presets al monorepo · 60 min


##### C · PREP.R2.C1 · Identitat de fallades i base · 5 h identificades

###### D · PREP.R2.C1.D1 · Ajustar extracció de fallades amb sortida real + Definir comparació de fallades heretades + Provar depuració prioritzant errors nous · 3,5 h identificades

- **[PREP.E05](paquets/PREP.md)** · Ajustar extracció de fallades amb sortida real · 90 min
- **[PREP.E06](paquets/PREP.md)** · Definir comparació de fallades heretades · 60 min
- **[PREP.E07](paquets/PREP.md)** · Provar depuració prioritzant errors nous · 60 min

###### D · PREP.R2.C1.D2 · Capturar base immutable abans del primer C++ · 1,5 h identificades

- **[PREP.E08](paquets/PREP.md)** · Capturar base immutable abans del primer C++ · 90 min


##### C · PREP.R3.C1 · Execució cooperativa i calibratge · 4,75 h identificades

###### D · PREP.R3.C1.D1 · Verificar vigilància bounded i exclusió de logs + Verificar lliurament des del commit · 2,5 h identificades

- **[PREP.E09](paquets/PREP.md)** · Verificar vigilància bounded i exclusió de logs · 60 min
- **[PREP.E10](paquets/PREP.md)** · Verificar lliurament des del commit · 90 min

###### D · PREP.R3.C1.D2 · Congelar interfícies del primer lot de perseguidors + Calibrar l’escala amb cinc E acabades · 2,25 h identificades

- **[PREP.E11](paquets/PREP.md)** · Congelar interfícies del primer lot de perseguidors · 90 min
- **[PREP.E12](paquets/PREP.md)** · Calibrar l’escala amb cinc E acabades · 45 min





## S · U1 · Encendre el que ja hi ha · durada indeterminada

### A · A-U1-01 · Entitats estables i enemics que persegueixen · 34,25 h identificades

#### B · U1-02 · World guarda les entitats en HandlePool · 17 h identificades

##### C · U1-02.R1.C1 · Identitat i vida útil abans de canviar el magatzem · 5,5 h identificades

###### D · U1-02.R1.C1.D1 · Fixar la invalidació per creixement + Caracteritzar la creació diferida + Caracteritzar la baixa immediata · 3 h identificades

- **[U1-02.E01](paquets/U1-02.md)** · Fixar la invalidació per creixement · 60 min
- **[U1-02.E02](paquets/U1-02.md)** · Caracteritzar la creació diferida · 60 min
- **[U1-02.E03](paquets/U1-02.md)** · Caracteritzar la baixa immediata · 60 min

###### D · U1-02.R1.C1.D2 · Provar la identitat després de reutilitzar slot + Fixar l’estratègia d’adaptació · 2,5 h identificades

- **[U1-02.E04](paquets/U1-02.md)** · Provar la identitat després de reutilitzar slot · 60 min
- **[U1-02.E05](paquets/U1-02.md)** · Fixar l’estratègia d’adaptació · 90 min


##### C · U1-02.R2.C1 · Adaptació del magatzem mantenint l’API pública · tall 1 · 6,75 h identificades

###### D · U1-02.R2.C1.D1 · Convertir identitat a la frontera + Reservar sense publicar entitats + Resoldre només entitats visibles · 3,75 h identificades

- **[U1-02.E06](paquets/U1-02.md)** · Convertir identitat a la frontera · 45 min
- **[U1-02.E07](paquets/U1-02.md)** · Reservar sense publicar entitats · 90 min
- **[U1-02.E08](paquets/U1-02.md)** · Resoldre només entitats visibles · 90 min

###### D · U1-02.R2.C1.D2 · Aplicar altes i baixes al límit del tick + Recórrer amb mutacions diferides · 3 h identificades

- **[U1-02.E09](paquets/U1-02.md)** · Aplicar altes i baixes al límit del tick · 90 min
- **[U1-02.E10](paquets/U1-02.md)** · Recórrer amb mutacions diferides · 90 min


##### C · U1-02.R2.C2 · Adaptació del magatzem mantenint l’API pública · tall 2 · 1 h identificades

###### D · U1-02.R2.C2.D1 · Consultar l’espai a través del magatzem · 1 h identificades

- **[U1-02.E11](paquets/U1-02.md)** · Consultar l’espai a través del magatzem · 60 min


##### C · U1-02.R3.C1 · Consumidors i compatibilitat · 3,75 h identificades

###### D · U1-02.R3.C1.D1 · Auditar el mirall de posició + Connectar el magatzem a World + Regressió de sessió i desat · 3,75 h identificades

- **[U1-02.E12](paquets/U1-02.md)** · Auditar el mirall de posició · 45 min
- **[U1-02.E13](paquets/U1-02.md)** · Connectar el magatzem a World · 90 min
- **[U1-02.E14](paquets/U1-02.md)** · Regressió de sessió i desat · 90 min



#### B · U1-03 · Els enemics persegueixen (estat kChase) · 17,25 h identificades

##### C · U1-03.R1.C1 · Percepció pura i criteri d’abandonament · 4,5 h identificades

###### D · U1-03.R1.C1.D1 · Fixar distància, visió i abandonament + Calcular distància de caselles + Classificar el camp de visió + Adquirir objectiu · 3,5 h identificades

- **[U1-03.E01](paquets/U1-03.md)** · Fixar distància, visió i abandonament · 60 min
- **[U1-03.E02](paquets/U1-03.md)** · Calcular distància de caselles · 30 min
- **[U1-03.E03](paquets/U1-03.md)** · Classificar el camp de visió · 60 min
- **[U1-03.E04](paquets/U1-03.md)** · Adquirir objectiu · 60 min

###### D · U1-03.R1.C1.D2 · Determinar pèrdua d’objectiu · 1 h identificades

- **[U1-03.E05](paquets/U1-03.md)** · Determinar pèrdua d’objectiu · 60 min


##### C · U1-03.R2.C1 · Ruta i temps de moviment · 7 h identificades

###### D · U1-03.R2.C1.D1 · Escollir el següent pas transitable + No travessar actors ni desnivells · 3 h identificades

- **[U1-03.E06](paquets/U1-03.md)** · Escollir el següent pas transitable · 90 min
- **[U1-03.E07](paquets/U1-03.md)** · No travessar actors ni desnivells · 90 min

###### D · U1-03.R2.C1.D2 · Consumir temps sense dependre dels FPS + Canviar patrulla a persecució · 2,5 h identificades

- **[U1-03.E08](paquets/U1-03.md)** · Consumir temps sense dependre dels FPS · 60 min
- **[U1-03.E09](paquets/U1-03.md)** · Canviar patrulla a persecució · 90 min

###### D · U1-03.R2.C1.D3 · Retornar a una patrulla vàlida · 1,5 h identificades

- **[U1-03.E10](paquets/U1-03.md)** · Retornar a una patrulla vàlida · 90 min


##### C · U1-03.R3.C1 · Connexió al joc i prova visible · 5,75 h identificades

###### D · U1-03.R3.C1.D1 · Demanar combat en el contacte + Connectar percepció i col·lisió en la sessió + Reiniciar IA després de carregar · 3,5 h identificades

- **[U1-03.E11](paquets/U1-03.md)** · Demanar combat en el contacte · 60 min
- **[U1-03.E12](paquets/U1-03.md)** · Connectar percepció i col·lisió en la sessió · 90 min
- **[U1-03.E13](paquets/U1-03.md)** · Reiniciar IA després de carregar · 60 min

###### D · U1-03.R3.C1.D2 · Decidir el senyal de detecció + Recórrer el bucle jugable de persecució · 2,25 h identificades

- **[U1-03.E14](paquets/U1-03.md)** · Decidir el senyal de detecció · 45 min
- **[U1-03.E15](paquets/U1-03.md)** · Recórrer el bucle jugable de persecució · 90 min




### A · A-U1-02 · Guardar, recuperar i continuar · 25,25 h identificades

#### B · G-11 · Pantalles de guardar i carregar (resta) · 14 h identificades

##### C · G-11.R1.C1 · Model de pantalla sense canviar el servei · 4,5 h identificades

###### D · G-11.R1.C1.D1 · Resoldre sis ranures existents vs tres proposades + Projectar una fila de ranura + Seleccionar una ranura amb teclat + Decidir si guardar requereix consentiment · 3,5 h identificades

- **[G-11.E01](paquets/G-11.md)** · Resoldre sis ranures existents vs tres proposades · 45 min
- **[G-11.E02](paquets/G-11.md)** · Projectar una fila de ranura · 60 min
- **[G-11.E03](paquets/G-11.md)** · Seleccionar una ranura amb teclat · 45 min
- **[G-11.E04](paquets/G-11.md)** · Decidir si guardar requereix consentiment · 60 min

###### D · G-11.R1.C1.D2 · Descriure la pèrdua en carregar · 1 h identificades

- **[G-11.E05](paquets/G-11.md)** · Descriure la pèrdua en carregar · 60 min


##### C · G-11.R2.C1 · Servei i representació · 6,5 h identificades

###### D · G-11.R2.C1.D1 · Verificar round-trip entre ranures de projecte + Carregar sense destruir la sessió bona + Registrar el servei en una biblioteca · 3,5 h identificades

- **[G-11.E06](paquets/G-11.md)** · Verificar round-trip entre ranures de projecte · 60 min
- **[G-11.E07](paquets/G-11.md)** · Carregar sense destruir la sessió bona · 60 min
- **[G-11.E08](paquets/G-11.md)** · Registrar el servei en una biblioteca · 90 min

###### D · G-11.R2.C1.D2 · Dibuixar les files amb el sistema de menú actual + Connectar pausa i selector · 3 h identificades

- **[G-11.E09](paquets/G-11.md)** · Dibuixar les files amb el sistema de menú actual · 90 min
- **[G-11.E10](paquets/G-11.md)** · Connectar pausa i selector · 90 min


##### C · G-11.R3.C1 · Confirmacions i acceptació · 3 h identificades

###### D · G-11.R3.C1.D1 · Executar una sola càrrega confirmada + Provar les tres sortides de la pantalla · 3 h identificades

- **[G-11.E11](paquets/G-11.md)** · Executar una sola càrrega confirmada · 90 min
- **[G-11.E12](paquets/G-11.md)** · Provar les tres sortides de la pantalla · 90 min



#### B · G-01 · Autosave i recuperació · 11,25 h identificades

##### C · G-01.R1.C1 · Política d’autoguardat i compatibilitat · 3,75 h identificades

###### D · G-01.R1.C1.D1 · Identificar els punts de publicació segura + Classificar la petició d’autosave + Seleccionar una ruta automàtica estable + Validar escriptura atòmica existent · 3,75 h identificades

- **[G-01.E01](paquets/G-01.md)** · Identificar els punts de publicació segura · 60 min
- **[G-01.E02](paquets/G-01.md)** · Classificar la petició d’autosave · 60 min
- **[G-01.E03](paquets/G-01.md)** · Seleccionar una ruta automàtica estable · 45 min
- **[G-01.E04](paquets/G-01.md)** · Validar escriptura atòmica existent · 60 min


##### C · G-01.R2.C1 · Recuperació i títol · 7,5 h identificades

###### D · G-01.R2.C1.D1 · Provar interrupció abans de publicar + Triar el guardat més recent que es pot carregar + Importar la ranura ràpida antiga sense pèrdua · 3 h identificades

- **[G-01.E05](paquets/G-01.md)** · Provar interrupció abans de publicar · 60 min
- **[G-01.E06](paquets/G-01.md)** · Triar el guardat més recent que es pot carregar · 60 min
- **[G-01.E07](paquets/G-01.md)** · Importar la ranura ràpida antiga sense pèrdua · 60 min

###### D · G-01.R2.C1.D2 · Connectar Continuar amb el mateix servei + Connectar els dos punts d’autoguardat · 3 h identificades

- **[G-01.E08](paquets/G-01.md)** · Connectar Continuar amb el mateix servei · 90 min
- **[G-01.E09](paquets/G-01.md)** · Connectar els dos punts d’autoguardat · 90 min

###### D · G-01.R2.C1.D3 · Verificar recuperació completa · 1,5 h identificades

- **[G-01.E10](paquets/G-01.md)** · Verificar recuperació completa · 90 min




### A · A-U1-03 · So de joc i opcions connectats · 15,25 h identificades

#### B · U1-01 · L'àudio sona al joc (AudioDirector + assets/audio) · 15,25 h identificades

##### C · U1-01.R1.C1 · Dades i dependències de so · 4,25 h identificades

###### D · U1-01.R1.C1.D1 · Inventariar backend i recursos de so + Llegir correspondències de cues en dades + Seleccionar música del lloc · 3,25 h identificades

- **[U1-01.E01](paquets/U1-01.md)** · Inventariar backend i recursos de so · 45 min
- **[U1-01.E02](paquets/U1-01.md)** · Llegir correspondències de cues en dades · 90 min
- **[U1-01.E03](paquets/U1-01.md)** · Seleccionar música del lloc · 60 min

###### D · U1-01.R1.C1.D2 · Seleccionar l’efecte d’un resultat · 1 h identificades

- **[U1-01.E04](paquets/U1-01.md)** · Seleccionar l’efecte d’un resultat · 60 min


##### C · U1-01.R2.C1 · Director, volums i fallades · 6 h identificades

###### D · U1-01.R2.C1.D1 · Mantenir la música idempotent + Definir el contracte del fundido · 2,5 h identificades

- **[U1-01.E05](paquets/U1-01.md)** · Mantenir la música idempotent · 60 min
- **[U1-01.E06](paquets/U1-01.md)** · Definir el contracte del fundido · 90 min

###### D · U1-01.R2.C1.D2 · Avançar un fundido amb el temps + Aplicar els volums ja guardats + Tolerar falta de dispositiu o fitxer · 3,5 h identificades

- **[U1-01.E07](paquets/U1-01.md)** · Avançar un fundido amb el temps · 90 min
- **[U1-01.E08](paquets/U1-01.md)** · Aplicar els volums ja guardats · 60 min
- **[U1-01.E09](paquets/U1-01.md)** · Tolerar falta de dispositiu o fitxer · 60 min


##### C · U1-01.R3.C1 · Connexió i prova audible · 5 h identificades

###### D · U1-01.R3.C1.D1 · Crear i destruir el servei al límit correcte + Connectar canvi de lloc i resultats de combat · 3 h identificades

- **[U1-01.E10](paquets/U1-01.md)** · Crear i destruir el servei al límit correcte · 90 min
- **[U1-01.E11](paquets/U1-01.md)** · Connectar canvi de lloc i resultats de combat · 90 min

###### D · U1-01.R3.C1.D2 · Connectar Opcions al servei viu + Registrar acceptació funcional i dependència de recursos · 2 h identificades

- **[U1-01.E12](paquets/U1-01.md)** · Connectar Opcions al servei viu · 60 min
- **[U1-01.E13](paquets/U1-01.md)** · Registrar acceptació funcional i dependència de recursos · 60 min




### A · A-U1-04 · PNJ i llum segueixen el moment narratiu · 21,75 h identificades

#### B · U1-04 · Rutines de PNJ segons la franja del dia · 13,5 h identificades

##### C · U1-04.R1.C1 · Contracte narratiu i dades horàries · 4,25 h identificades

###### D · U1-04.R1.C1.D1 · Resoldre tres franges reals vs quatre descrites + Llegir un horari sense moure el PNJ + Triar el destí d’una franja · 3,25 h identificades

- **[U1-04.E01](paquets/U1-04.md)** · Resoldre tres franges reals vs quatre descrites · 45 min
- **[U1-04.E02](paquets/U1-04.md)** · Llegir un horari sense moure el PNJ · 90 min
- **[U1-04.E03](paquets/U1-04.md)** · Triar el destí d’una franja · 60 min

###### D · U1-04.R1.C1.D2 · Validar ubicacions nocturnes com a proposta · 1 h identificades

- **[U1-04.E04](paquets/U1-04.md)** · Validar ubicacions nocturnes com a proposta · 60 min


##### C · U1-04.R2.C1 · Moviment coherent amb el món · 6,75 h identificades

###### D · U1-04.R2.C1.D1 · Ajornar un canvi si el PNJ està ocupat + Escollir caminar o trasllat fora de vista + Demanar un pas al navegador existent · 3,75 h identificades

- **[U1-04.E05](paquets/U1-04.md)** · Ajornar un canvi si el PNJ està ocupat · 45 min
- **[U1-04.E06](paquets/U1-04.md)** · Escollir caminar o trasllat fora de vista · 90 min
- **[U1-04.E07](paquets/U1-04.md)** · Demanar un pas al navegador existent · 90 min

###### D · U1-04.R2.C1.D2 · Aplicar el canvi només un cop per franja + Preservar identitat en carregar i canviar mapa · 3 h identificades

- **[U1-04.E08](paquets/U1-04.md)** · Aplicar el canvi només un cop per franja · 90 min
- **[U1-04.E09](paquets/U1-04.md)** · Preservar identitat en carregar i canviar mapa · 90 min


##### C · U1-04.R3.C1 · Integració de franja i demostració · 2,5 h identificades

###### D · U1-04.R3.C1.D1 · Connectar el moment narratiu a la sessió + Comprovar dia, conversa i nit al joc · 2,5 h identificades

- **[U1-04.E10](paquets/U1-04.md)** · Connectar el moment narratiu a la sessió · 60 min
- **[U1-04.E11](paquets/U1-04.md)** · Comprovar dia, conversa i nit al joc · 90 min



#### B · U1-05 · Llum global segons la franja del dia · 8,25 h identificades

##### C · U1-05.R1.C1 · Dades i interpolació de llum · 4,75 h identificades

###### D · U1-05.R1.C1.D1 · Localitzar el tint global de món i les capes UI + Llegir colors de franja + Triar el color objectiu + Interpolar un tint amb límits · 3,75 h identificades

- **[U1-05.E01](paquets/U1-05.md)** · Localitzar el tint global de món i les capes UI · 60 min
- **[U1-05.E02](paquets/U1-05.md)** · Llegir colors de franja · 60 min
- **[U1-05.E03](paquets/U1-05.md)** · Triar el color objectiu · 45 min
- **[U1-05.E04](paquets/U1-05.md)** · Interpolar un tint amb límits · 60 min

###### D · U1-05.R1.C1.D2 · Reorientar una transició en curs · 1 h identificades

- **[U1-05.E05](paquets/U1-05.md)** · Reorientar una transició en curs · 60 min


##### C · U1-05.R2.C1 · Aplicació i llegibilitat · 3,5 h identificades

###### D · U1-05.R2.C1.D1 · Avançar tint amb dt sense alterar franja + Passar el tint al render de món + Mostrar paleta a Oriol amb el mateix enquadrament · 3,5 h identificades

- **[U1-05.E06](paquets/U1-05.md)** · Avançar tint amb dt sense alterar franja · 60 min
- **[U1-05.E07](paquets/U1-05.md)** · Passar el tint al render de món · 90 min
- **[U1-05.E08](paquets/U1-05.md)** · Mostrar paleta a Oriol amb el mateix enquadrament · 60 min





## S · U2 · Separar motor i joc · durada indeterminada

### A · A-U2-01 · Límits i configuració de projecte · 20 h identificades

#### B · U2-01 · Inventari motor / mòdul RPG / dades Onegai + tests de base · 8 h identificades

##### C · U2-01.R1.C1 · Inventari verificable de dependències · 4,5 h identificades

###### D · U2-01.R1.C1.D1 · Localitzar propietaris dels serveis + Mesurar dependències prohibides · 3 h identificades

- **[U2-01.E01](paquets/U2-01.md)** · Localitzar propietaris dels serveis · 90 min
- **[U2-01.E02](paquets/U2-01.md)** · Mesurar dependències prohibides · 90 min

###### D · U2-01.R1.C1.D2 · Fixar el contracte entre capes · 1,5 h identificades

- **[U2-01.E03](paquets/U2-01.md)** · Fixar el contracte entre capes · 90 min


##### C · U2-01.R2.C1 · Caracterització i prova de separació · 3,5 h identificades

###### D · U2-01.R2.C1.D1 · Caracteritzar l’inici actual de Boundington + Construir una prova sense contingut Onegai + Publicar el mapa dels punts de tall · 3,5 h identificades

- **[U2-01.E04](paquets/U2-01.md)** · Caracteritzar l’inici actual de Boundington · 60 min
- **[U2-01.E05](paquets/U2-01.md)** · Construir una prova sense contingut Onegai · 90 min
- **[U2-01.E06](paquets/U2-01.md)** · Publicar el mapa dels punts de tall · 60 min



#### B · U2-02 · ProjectSettings: escena inicial, catàlegs, idioma · 12 h identificades

##### C · U2-02.R1.C1 · Contracte de configuració compatible · 7 h identificades

###### D · U2-02.R1.C1.D1 · Auditar ajustos existents i llegat + Llegir escena inicial i idioma · 2,5 h identificades

- **[U2-02.E01](paquets/U2-02.md)** · Auditar ajustos existents i llegat · 60 min
- **[U2-02.E02](paquets/U2-02.md)** · Llegir escena inicial i idioma · 90 min

###### D · U2-02.R1.C1.D2 · Normalitzar rutes del projecte + Validar escena inicial resolta · 3 h identificades

- **[U2-02.E03](paquets/U2-02.md)** · Normalitzar rutes del projecte · 90 min
- **[U2-02.E04](paquets/U2-02.md)** · Validar escena inicial resolta · 90 min

###### D · U2-02.R1.C1.D3 · Resoldre catàlegs declarats · 1,5 h identificades

- **[U2-02.E05](paquets/U2-02.md)** · Resoldre catàlegs declarats · 90 min


##### C · U2-02.R2.C1 · Escriptura i ús real · 5 h identificades

###### D · U2-02.R2.C1.D1 · Serialitzar ajustos additius + Propagar idioma i títol · 2,5 h identificades

- **[U2-02.E06](paquets/U2-02.md)** · Serialitzar ajustos additius · 90 min
- **[U2-02.E07](paquets/U2-02.md)** · Propagar idioma i títol · 60 min

###### D · U2-02.R2.C1.D2 · Connectar lector i escriptor del projecte + Verificar dues configuracions diferents · 2,5 h identificades

- **[U2-02.E08](paquets/U2-02.md)** · Connectar lector i escriptor del projecte · 90 min
- **[U2-02.E09](paquets/U2-02.md)** · Verificar dues configuracions diferents · 60 min




### A · A-U2-02 · Arrencada i mòdul de joc · 24,5 h identificades

#### B · G-08 · El joc arrenca des del manifest (juego --proyecto) · 8,5 h identificades

##### C · G-08.R1.C1 · Arguments i entrada del projecte · 4,5 h identificades

###### D · G-08.R1.C1.D1 · Auditar CLI i títol ja existents + Interpretar --proyecto + Preparar l’arrencada des del manifest · 3,5 h identificades

- **[G-08.E01](paquets/G-08.md)** · Auditar CLI i títol ja existents · 60 min
- **[G-08.E02](paquets/G-08.md)** · Interpretar --proyecto · 60 min
- **[G-08.E03](paquets/G-08.md)** · Preparar l’arrencada des del manifest · 90 min

###### D · G-08.R1.C1.D2 · Escollir l’opció de títol · 1 h identificades

- **[G-08.E04](paquets/G-08.md)** · Escollir l’opció de títol · 60 min


##### C · G-08.R2.C1 · Execució i errors llegibles · 4 h identificades

###### D · G-08.R2.C1.D1 · Publicar el pla d’arrencada + Connectar el CLI a la ruta única · 3 h identificades

- **[G-08.E05](paquets/G-08.md)** · Publicar el pla d’arrencada · 90 min
- **[G-08.E06](paquets/G-08.md)** · Connectar el CLI a la ruta única · 90 min

###### D · G-08.R2.C1.D2 · Verificar arrencada fora del checkout · 1 h identificades

- **[G-08.E07](paquets/G-08.md)** · Verificar arrencada fora del checkout · 60 min



#### B · U2-03 · IGameModule i mòdul RPG separat a CMake · 16 h identificades

##### C · U2-03.R1.C1 · Port mínim i relleus · 7 h identificades

###### D · U2-03.R1.C1.D1 · Auditar el límit de mòdul sobre el graf real + Congelar serveis disponibles per al joc · 3 h identificades

- **[U2-03.E01](paquets/U2-03.md)** · Auditar el límit de mòdul sobre el graf real · 90 min
- **[U2-03.E02](paquets/U2-03.md)** · Congelar serveis disponibles per al joc · 90 min

###### D · U2-03.R1.C1.D2 · Definir inicialització del mòdul + Definir tick del mòdul · 3 h identificades

- **[U2-03.E03](paquets/U2-03.md)** · Definir inicialització del mòdul · 90 min
- **[U2-03.E04](paquets/U2-03.md)** · Definir tick del mòdul · 90 min

###### D · U2-03.R1.C1.D3 · Definir tancament idempotent · 1 h identificades

- **[U2-03.E05](paquets/U2-03.md)** · Definir tancament idempotent · 60 min


##### C · U2-03.R2.C1 · Mòdul RPG i dependències de compilació · 6 h identificades

###### D · U2-03.R2.C1.D1 · Adaptar regles RPG al port + Construir el mòdul RPG separat · 3 h identificades

- **[U2-03.E06](paquets/U2-03.md)** · Adaptar regles RPG al port · 90 min
- **[U2-03.E07](paquets/U2-03.md)** · Construir el mòdul RPG separat · 90 min

###### D · U2-03.R2.C1.D2 · Instanciar un mòdul des de configuració + Desplaçar el primer relleu de composició · 3 h identificades

- **[U2-03.E08](paquets/U2-03.md)** · Instanciar un mòdul des de configuració · 90 min
- **[U2-03.E09](paquets/U2-03.md)** · Desplaçar el primer relleu de composició · 90 min


##### C · U2-03.R3.C1 · Prova dels dos camins de producte · 3 h identificades

###### D · U2-03.R3.C1.D1 · Connectar el mòdul a juego + Verificar límit motor/RPG al build · 3 h identificades

- **[U2-03.E10](paquets/U2-03.md)** · Connectar el mòdul a juego · 90 min
- **[U2-03.E11](paquets/U2-03.md)** · Verificar límit motor/RPG al build · 90 min




### A · A-U2-03 · Crear i navegar projectes · 18,75 h identificades

#### B · U2-04 · Plantilles de projecte (buit, RPG isomètric, demo) · 8,75 h identificades

##### C · U2-04.R1.C1 · Plantilles i generació segura · 5 h identificades

###### D · U2-04.R1.C1.D1 · Localitzar la creació real de projectes + Validar els paràmetres de plantilla + Expandir una plantilla en memòria · 3,5 h identificades

- **[U2-04.E01](paquets/U2-04.md)** · Localitzar la creació real de projectes · 60 min
- **[U2-04.E02](paquets/U2-04.md)** · Validar els paràmetres de plantilla · 60 min
- **[U2-04.E03](paquets/U2-04.md)** · Expandir una plantilla en memòria · 90 min

###### D · U2-04.R1.C1.D2 · Publicar un projecte complet · 1,5 h identificades

- **[U2-04.E04](paquets/U2-04.md)** · Publicar un projecte complet · 90 min


##### C · U2-04.R2.C1 · Selecció i ús · 3,75 h identificades

###### D · U2-04.R2.C1.D1 · Llistar les tres plantilles amb descripció + Connectar elecció al Hub + Verificar crear, reobrir i jugar · 3,75 h identificades

- **[U2-04.E05](paquets/U2-04.md)** · Llistar les tres plantilles amb descripció · 45 min
- **[U2-04.E06](paquets/U2-04.md)** · Connectar elecció al Hub · 90 min
- **[U2-04.E07](paquets/U2-04.md)** · Verificar crear, reobrir i jugar · 90 min



#### B · E-11 · Arbre de mapes i panell de projecte (resta) · 10 h identificades

##### C · E-11.R1.C1 · Arbre i accions del projecte · 6 h identificades

###### D · E-11.R1.C1.D1 · Auditar panell i estat existents + Construir nodes de l’arbre + Filtrar sense perdre selecció · 3,5 h identificades

- **[E-11.E01](paquets/E-11.md)** · Auditar panell i estat existents · 60 min
- **[E-11.E02](paquets/E-11.md)** · Construir nodes de l’arbre · 90 min
- **[E-11.E03](paquets/E-11.md)** · Filtrar sense perdre selecció · 60 min

###### D · E-11.R1.C1.D2 · Resoldre acció d’obrir node + Aplicar renombrat amb referències · 2,5 h identificades

- **[E-11.E04](paquets/E-11.md)** · Resoldre acció d’obrir node · 60 min
- **[E-11.E05](paquets/E-11.md)** · Aplicar renombrat amb referències · 90 min


##### C · E-11.R2.C1 · Interfície i persistència · 4 h identificades

###### D · E-11.R2.C1.D1 · Dibuixar arbre amb focus accessible + Connectar el node a document real · 3 h identificades

- **[E-11.E06](paquets/E-11.md)** · Dibuixar arbre amb focus accessible · 90 min
- **[E-11.E07](paquets/E-11.md)** · Connectar el node a document real · 90 min

###### D · E-11.R2.C1.D2 · Verificar recorregut d’autor · 1 h identificades

- **[E-11.E08](paquets/E-11.md)** · Verificar recorregut d’autor · 60 min




### A · A-U2-04 · Editor únic amb paritat · 14 h identificades

#### B · U2-05 · Un sol editor: el de level_editor passa a motor_editor · 14 h identificades

##### C · U2-05.R1.C1 · Paritat abans de retirar una entrada · 5,5 h identificades

###### D · U2-05.R1.C1.D1 · Inventariar funcionalitats dels dos editors + Decidir host únic i compatibilitat de CLI · 2,5 h identificades

- **[U2-05.E01](paquets/U2-05.md)** · Inventariar funcionalitats dels dos editors · 90 min
- **[U2-05.E02](paquets/U2-05.md)** · Decidir host únic i compatibilitat de CLI · 60 min

###### D · U2-05.R1.C1.D2 · Extreure el primer servei de document + Compartir encaminament de comandes · 3 h identificades

- **[U2-05.E03](paquets/U2-05.md)** · Extreure el primer servei de document · 90 min
- **[U2-05.E04](paquets/U2-05.md)** · Compartir encaminament de comandes · 90 min


##### C · U2-05.R2.C1 · Migració incremental · 5,5 h identificades

###### D · U2-05.R2.C1.D1 · Connectar vista de mapa al host únic + Connectar objectes i propietats · 3 h identificades

- **[U2-05.E05](paquets/U2-05.md)** · Connectar vista de mapa al host únic · 90 min
- **[U2-05.E06](paquets/U2-05.md)** · Connectar objectes i propietats · 90 min

###### D · U2-05.R2.C1.D2 · Connectar events i validació + Unificar ajuda i controls anunciats · 2,5 h identificades

- **[U2-05.E07](paquets/U2-05.md)** · Connectar events i validació · 90 min
- **[U2-05.E08](paquets/U2-05.md)** · Unificar ajuda i controls anunciats · 60 min


##### C · U2-05.R3.C1 · Retirada condicionada a paritat · 3 h identificades

###### D · U2-05.R3.C1.D1 · Eliminar el camí duplicat verificat + Fer una sessió d’autor completa · 3 h identificades

- **[U2-05.E09](paquets/U2-05.md)** · Eliminar el camí duplicat verificat · 90 min
- **[U2-05.E10](paquets/U2-05.md)** · Fer una sessió d’autor completa · 90 min





## S · U3 · Autoria sense JSON · durada indeterminada

### A · A-U3-01 · Semàntica comuna d’events i regions · 26,5 h identificades

#### B · DEC-10 · Decisió: quina pàgina d'event guanya · 3 h identificades

##### C · DEC-10.R1.C1 · Decisió de pàgines i rearmament · 3 h identificades

###### D · DEC-10.R1.C1.D1 · Comparar política real i opcions + Fixar exemples entrada-sortida + Registrar el vist de la política · 3 h identificades

- **[DEC-10.E01](paquets/DEC-10.md)** · Comparar política real i opcions · 90 min
- **[DEC-10.E02](paquets/DEC-10.md)** · Fixar exemples entrada-sortida · 60 min
- **[DEC-10.E03](paquets/DEC-10.md)** · Registrar el vist de la política · 30 min



#### B · U3-01 · Beats i events de mapa en un sol sistema · 13,5 h identificades

##### C · U3-01.R1.C1 · Semàntica comuna sense duplicar intèrpret · 7,5 h identificades

###### D · U3-01.R1.C1.D1 · Inventariar els dos camins narratius + Normalitzar una condició de beat · 3 h identificades

- **[U3-01.E01](paquets/U3-01.md)** · Inventariar els dos camins narratius · 90 min
- **[U3-01.E02](paquets/U3-01.md)** · Normalitzar una condició de beat · 90 min

###### D · U3-01.R1.C1.D2 · Adaptar una llista d’efectes + Seleccionar pàgina segons contracte · 3 h identificades

- **[U3-01.E03](paquets/U3-01.md)** · Adaptar una llista d’efectes · 90 min
- **[U3-01.E04](paquets/U3-01.md)** · Seleccionar pàgina segons contracte · 90 min

###### D · U3-01.R1.C1.D3 · Traduir un beat al model comú · 1,5 h identificades

- **[U3-01.E05](paquets/U3-01.md)** · Traduir un beat al model comú · 90 min


##### C · U3-01.R2.C1 · Compatibilitat en execució real · 6 h identificades

###### D · U3-01.R2.C1.D1 · Executar a través del host existent + Preservar progrés dels guardats · 3 h identificades

- **[U3-01.E06](paquets/U3-01.md)** · Executar a través del host existent · 90 min
- **[U3-01.E07](paquets/U3-01.md)** · Preservar progrés dels guardats · 90 min

###### D · U3-01.R2.C1.D2 · Connectar un recorregut vertical de campanya + Desactivar el camí antic només en paritat · 3 h identificades

- **[U3-01.E08](paquets/U3-01.md)** · Connectar un recorregut vertical de campanya · 90 min
- **[U3-01.E09](paquets/U3-01.md)** · Desactivar el camí antic només en paritat · 90 min



#### B · U3-02 · Triggers de regió: entrada, permanència, sortida · 10 h identificades

##### C · U3-02.R1.C1 · Regions i fronteres · 5 h identificades

###### D · U3-02.R1.C1.D1 · Auditar capa de regions existent + Consultar pertinença d’una casella + Calcular entrada i sortida · 3,5 h identificades

- **[U3-02.E01](paquets/U3-02.md)** · Auditar capa de regions existent · 60 min
- **[U3-02.E02](paquets/U3-02.md)** · Consultar pertinença d’una casella · 60 min
- **[U3-02.E03](paquets/U3-02.md)** · Calcular entrada i sortida · 90 min

###### D · U3-02.R1.C1.D2 · Produir permanència sense inundar events · 1,5 h identificades

- **[U3-02.E04](paquets/U3-02.md)** · Produir permanència sense inundar events · 90 min


##### C · U3-02.R2.C1 · Connexió, pausa i persistència · 5 h identificades

###### D · U3-02.R2.C1.D1 · Ordenar fets simultanis + Trametre al runner existent + Reiniciar presència en transició de mapa · 3,5 h identificades

- **[U3-02.E05](paquets/U3-02.md)** · Ordenar fets simultanis · 60 min
- **[U3-02.E06](paquets/U3-02.md)** · Trametre al runner existent · 90 min
- **[U3-02.E07](paquets/U3-02.md)** · Reiniciar presència en transició de mapa · 60 min

###### D · U3-02.R2.C1.D2 · Connectar i provar tres disparadors · 1,5 h identificades

- **[U3-02.E08](paquets/U3-02.md)** · Connectar i provar tres disparadors · 90 min




### A · A-U3-02 · Control de flux i utilitat temporal · 18,5 h identificades

#### B · U3-03 · Events comuns, bucles i etiquetes · 10 h identificades

##### C · U3-03.R1.C1 · Crides comunes i retorn · 4,5 h identificades

###### D · U3-03.R1.C1.D1 · Auditar crides, etiquetes i bucles existents + Validar referència a event comú + Apilar una crida i el seu retorn · 3,5 h identificades

- **[U3-03.E01](paquets/U3-03.md)** · Auditar crides, etiquetes i bucles existents · 60 min
- **[U3-03.E02](paquets/U3-03.md)** · Validar referència a event comú · 60 min
- **[U3-03.E03](paquets/U3-03.md)** · Apilar una crida i el seu retorn · 90 min

###### D · U3-03.R1.C1.D2 · Retornar d’una crida comuna · 1 h identificades

- **[U3-03.E04](paquets/U3-03.md)** · Retornar d’una crida comuna · 60 min


##### C · U3-03.R2.C1 · Bucles limitats i diagnòstic · 5,5 h identificades

###### D · U3-03.R2.C1.D1 · Indexar etiquetes sense duplicats + Resoldre salt i trencament de bucle · 2,5 h identificades

- **[U3-03.E05](paquets/U3-03.md)** · Indexar etiquetes sense duplicats · 60 min
- **[U3-03.E06](paquets/U3-03.md)** · Resoldre salt i trencament de bucle · 90 min

###### D · U3-03.R2.C1.D2 · Limitar treball per tick + Connectar al catàleg i host del joc · 3 h identificades

- **[U3-03.E07](paquets/U3-03.md)** · Limitar treball per tick · 90 min
- **[U3-03.E08](paquets/U3-03.md)** · Connectar al catàleg i host del joc · 90 min



#### B · U3-07 · Utilitat de tweens compartida · 8,5 h identificades

##### C · U3-07.R1.C1 · Interpolació i curs temporal · 4,5 h identificades

###### D · U3-07.R1.C1.D1 · Inventariar interpolacions i rellotges reals + Normalitzar el progrés temporal + Calcular easing definit en dades + Interpolar escalar · 3,5 h identificades

- **[U3-07.E01](paquets/U3-07.md)** · Inventariar interpolacions i rellotges reals · 60 min
- **[U3-07.E02](paquets/U3-07.md)** · Normalitzar el progrés temporal · 45 min
- **[U3-07.E03](paquets/U3-07.md)** · Calcular easing definit en dades · 60 min
- **[U3-07.E04](paquets/U3-07.md)** · Interpolar escalar · 45 min

###### D · U3-07.R1.C1.D2 · Interpolar vector amb la mateixa política · 1 h identificades

- **[U3-07.E05](paquets/U3-07.md)** · Interpolar vector amb la mateixa política · 60 min


##### C · U3-07.R2.C1 · Control i consumidor · 4 h identificades

###### D · U3-07.R2.C1.D1 · Avançar i completar una sola vegada + Cancel·lar amb política explícita · 2,5 h identificades

- **[U3-07.E06](paquets/U3-07.md)** · Avançar i completar una sola vegada · 90 min
- **[U3-07.E07](paquets/U3-07.md)** · Cancel·lar amb política explícita · 60 min

###### D · U3-07.R2.C1.D2 · Connectar un consumidor de producte · 1,5 h identificades

- **[U3-07.E08](paquets/U3-07.md)** · Connectar un consumidor de producte · 90 min




### A · A-U3-03 · Ordres de joc i autoría de pàgines · 27 h identificades

#### B · U3-04 · Ordres de moure actor, càmera, so i triar opció · 13 h identificades

##### C · U3-04.R1.C1 · Ordres petites amb final explícit · 7 h identificades

###### D · U3-04.R1.C1.D1 · Congelar DTOs i comandes suportades + Validar una ordre de moure actor · 3 h identificades

- **[U3-04.E01](paquets/U3-04.md)** · Congelar DTOs i comandes suportades · 90 min
- **[U3-04.E02](paquets/U3-04.md)** · Validar una ordre de moure actor · 90 min

###### D · U3-04.R1.C1.D2 · Avançar moviment d’actor + Validar i llançar una ordre de càmera · 3 h identificades

- **[U3-04.E03](paquets/U3-04.md)** · Avançar moviment d’actor · 90 min
- **[U3-04.E04](paquets/U3-04.md)** · Validar i llançar una ordre de càmera · 90 min

###### D · U3-04.R1.C1.D3 · Validar i llançar una ordre de so · 1 h identificades

- **[U3-04.E05](paquets/U3-04.md)** · Validar i llançar una ordre de so · 60 min


##### C · U3-04.R2.C1 · Elecció, espera i integració · 6 h identificades

###### D · U3-04.R2.C1.D1 · Preparar opcions d’un diàleg + Recollir resultat d’una acció asíncrona · 3 h identificades

- **[U3-04.E06](paquets/U3-04.md)** · Preparar opcions d’un diàleg · 90 min
- **[U3-04.E07](paquets/U3-04.md)** · Recollir resultat d’una acció asíncrona · 90 min

###### D · U3-04.R2.C1.D2 · Connectar accions al EventHost existent + Fer una escena amb les quatre ordres · 3 h identificades

- **[U3-04.E08](paquets/U3-04.md)** · Connectar accions al EventHost existent · 90 min
- **[U3-04.E09](paquets/U3-04.md)** · Fer una escena amb les quatre ordres · 90 min



#### B · E-05 · Editor de pàgines d'esdeveniment · 14 h identificades

##### C · E-05.R1.C1 · Pàgines com a document editable · 6,5 h identificades

###### D · E-05.R1.C1.D1 · Auditar operacions de pàgines ja existents + Crear una pàgina vàlida + Modificar una condició · 3,5 h identificades

- **[E-05.E01](paquets/E-05.md)** · Auditar operacions de pàgines ja existents · 60 min
- **[E-05.E02](paquets/E-05.md)** · Crear una pàgina vàlida · 60 min
- **[E-05.E03](paquets/E-05.md)** · Modificar una condició · 90 min

###### D · E-05.R1.C1.D2 · Reordenar amb prioritat visible + Eliminar pàgina amb desfer · 3 h identificades

- **[E-05.E04](paquets/E-05.md)** · Reordenar amb prioritat visible · 90 min
- **[E-05.E05](paquets/E-05.md)** · Eliminar pàgina amb desfer · 90 min


##### C · E-05.R2.C1 · Vista i diagnòstic d’autor · 7,5 h identificades

###### D · E-05.R2.C1.D1 · Construir l’explicació de pàgina activa + Dibuixar pestanyes i camps de condició · 3 h identificades

- **[E-05.E06](paquets/E-05.md)** · Construir l’explicació de pàgina activa · 90 min
- **[E-05.E07](paquets/E-05.md)** · Dibuixar pestanyes i camps de condició · 90 min

###### D · E-05.R2.C1.D2 · Aplicar una edició al document + Connectar trigger de regió al formulari · 3 h identificades

- **[E-05.E08](paquets/E-05.md)** · Aplicar una edició al document · 90 min
- **[E-05.E09](paquets/E-05.md)** · Connectar trigger de regió al formulari · 90 min

###### D · E-05.R2.C1.D3 · Verificar dues pàgines jugables · 1,5 h identificades

- **[E-05.E10](paquets/E-05.md)** · Verificar dues pàgines jugables · 90 min




### A · A-U3-04 · Comandes visuals i depuració · 22,5 h identificades

#### B · E-06 · Editor vertical de comandes · 15,5 h identificades

##### C · E-06.R1.C1 · Llista tipada i estructura · tall 1 · 6,5 h identificades

###### D · E-06.R1.C1.D1 · Auditar catàleg de comandes i llista existents + Projectar comandes en files + Crear una comanda amb defaults vàlids · 3,5 h identificades

- **[E-06.E01](paquets/E-06.md)** · Auditar catàleg de comandes i llista existents · 60 min
- **[E-06.E02](paquets/E-06.md)** · Projectar comandes en files · 90 min
- **[E-06.E03](paquets/E-06.md)** · Crear una comanda amb defaults vàlids · 60 min

###### D · E-06.R1.C1.D2 · Editar un paràmetre amb validació + Inserir comanda en camí vàlid · 3 h identificades

- **[E-06.E04](paquets/E-06.md)** · Editar un paràmetre amb validació · 90 min
- **[E-06.E05](paquets/E-06.md)** · Inserir comanda en camí vàlid · 90 min


##### C · E-06.R1.C2 · Llista tipada i estructura · tall 2 · 1,5 h identificades

###### D · E-06.R1.C2.D1 · Moure bloc sense trencar l’arbre · 1,5 h identificades

- **[E-06.E06](paquets/E-06.md)** · Moure bloc sense trencar l’arbre · 90 min


##### C · E-06.R2.C1 · Autoría, ajuda i integració · 7,5 h identificades

###### D · E-06.R2.C1.D1 · Copiar i enganxar amb nous IDs quan calgui + Dibuixar resum i formulari d’una ordre · 3 h identificades

- **[E-06.E07](paquets/E-06.md)** · Copiar i enganxar amb nous IDs quan calgui · 90 min
- **[E-06.E08](paquets/E-06.md)** · Dibuixar resum i formulari d’una ordre · 90 min

###### D · E-06.R2.C1.D2 · Mostrar errors al comandament exacte + Aplicar edició amb una sola operació d’undo · 3 h identificades

- **[E-06.E09](paquets/E-06.md)** · Mostrar errors al comandament exacte · 90 min
- **[E-06.E10](paquets/E-06.md)** · Aplicar edició amb una sola operació d’undo · 90 min

###### D · E-06.R2.C1.D3 · Fer event sense editar JSON · 1,5 h identificades

- **[E-06.E11](paquets/E-06.md)** · Fer event sense editar JSON · 90 min



#### B · E-08 · Panell del depurador pas a pas (resta) · 7 h identificades

##### C · E-08.R1.C1 · Depurador sobre estat ja existent · 7 h identificades

###### D · E-08.R1.C1.D1 · Auditar depurador i trace runtime + Projectar el punt d’execució + Resoldre el comandament clicat a breakpoint · 3 h identificades

- **[E-08.E01](paquets/E-08.md)** · Auditar depurador i trace runtime · 60 min
- **[E-08.E02](paquets/E-08.md)** · Projectar el punt d’execució · 60 min
- **[E-08.E03](paquets/E-08.md)** · Resoldre el comandament clicat a breakpoint · 60 min

###### D · E-08.R1.C1.D2 · Aplicar pas únic des del panell + Dibuixar trace i valors inspeccionables · 2,5 h identificades

- **[E-08.E04](paquets/E-08.md)** · Aplicar pas únic des del panell · 60 min
- **[E-08.E05](paquets/E-08.md)** · Dibuixar trace i valors inspeccionables · 90 min

###### D · E-08.R1.C1.D3 · Connectar i comprovar un breakpoint real · 1,5 h identificades

- **[E-08.E06](paquets/E-08.md)** · Connectar i comprovar un breakpoint real · 90 min




### A · A-U3-05 · Clips i estats d’animació · 19 h identificades

#### B · U3-05 · Clips d'animació en JSON amb events de fotograma · 10 h identificades

##### C · U3-05.R1.C1 · Clips declarats i events · 6 h identificades

###### D · U3-05.R1.C1.D1 · Auditar reproductors i formats existents + Llegir un clip en JSON + Indexar events de fotograma · 3,5 h identificades

- **[U3-05.E01](paquets/U3-05.md)** · Auditar reproductors i formats existents · 60 min
- **[U3-05.E02](paquets/U3-05.md)** · Llegir un clip en JSON · 90 min
- **[U3-05.E03](paquets/U3-05.md)** · Indexar events de fotograma · 60 min

###### D · U3-05.R1.C1.D2 · Avançar cursor d’un clip + Canviar clip amb política de reinici · 2,5 h identificades

- **[U3-05.E04](paquets/U3-05.md)** · Avançar cursor d’un clip · 90 min
- **[U3-05.E05](paquets/U3-05.md)** · Canviar clip amb política de reinici · 60 min


##### C · U3-05.R2.C1 · Integració de temps i consumidor · 4 h identificades

###### D · U3-05.R2.C1.D1 · Separar event lògic d’event visual + Connectar actor al reproductor de clip · 3 h identificades

- **[U3-05.E06](paquets/U3-05.md)** · Separar event lògic d’event visual · 90 min
- **[U3-05.E07](paquets/U3-05.md)** · Connectar actor al reproductor de clip · 90 min

###### D · U3-05.R2.C1.D2 · Verificar loop i clip no cíclic · 1 h identificades

- **[U3-05.E08](paquets/U3-05.md)** · Verificar loop i clip no cíclic · 60 min



#### B · U3-06 · Màquina d'estats d'animació en dades · 9 h identificades

##### C · U3-06.R1.C1 · Estats declaratius · 5,5 h identificades

###### D · U3-06.R1.C1.D1 · Auditar qui tria animacions avui + Llegir estats i transicions · 2,5 h identificades

- **[U3-06.E01](paquets/U3-06.md)** · Auditar qui tria animacions avui · 60 min
- **[U3-06.E02](paquets/U3-06.md)** · Llegir estats i transicions · 90 min

###### D · U3-06.R1.C1.D2 · Seleccionar transició + Aplicar canvi d’estat sense reinici indegut · 3 h identificades

- **[U3-06.E03](paquets/U3-06.md)** · Seleccionar transició · 90 min
- **[U3-06.E04](paquets/U3-06.md)** · Aplicar canvi d’estat sense reinici indegut · 90 min


##### C · U3-06.R2.C1 · Connexió amb actor · 3,5 h identificades

###### D · U3-06.R2.C1.D1 · Construir inputs d’un actor de producte + Connectar la màquina al cursor de clip + Verificar idle/caminar/acció · 3,5 h identificades

- **[U3-06.E05](paquets/U3-06.md)** · Construir inputs d’un actor de producte · 60 min
- **[U3-06.E06](paquets/U3-06.md)** · Connectar la màquina al cursor de clip · 90 min
- **[U3-06.E07](paquets/U3-06.md)** · Verificar idle/caminar/acció · 60 min





## S · U4 · Eines tipus Unity · durada indeterminada

### A · A-U4-01 · Host de playtest i origen seleccionat · 21,25 h identificades

#### B · DEC-06 · Decisió: joc dins l'editor o en finestra a part · 3 h identificades

##### C · DEC-06.R1.C1 · Decisió de procés o vista incrustada · 3 h identificades

###### D · DEC-06.R1.C1.D1 · Auditar el playtest actual + Comparar dues opcions de host + Registrar resolució i límit d’implementació · 3 h identificades

- **[DEC-06.E01](paquets/DEC-06.md)** · Auditar el playtest actual · 60 min
- **[DEC-06.E02](paquets/DEC-06.md)** · Comparar dues opcions de host · 90 min
- **[DEC-06.E03](paquets/DEC-06.md)** · Registrar resolució i límit d’implementació · 30 min



#### B · F-01 · Playtest des de l'editor · 12 h identificades

##### C · F-01.R1.C1 · Sessió de prova aïllada · 6,5 h identificades

###### D · F-01.R1.C1.D1 · Congelar contracte host i documents bruts + Preparar una petició de prova · 2,5 h identificades

- **[F-01.E01](paquets/F-01.md)** · Congelar contracte host i documents bruts · 60 min
- **[F-01.E02](paquets/F-01.md)** · Preparar una petició de prova · 90 min

###### D · F-01.R1.C1.D2 · Crear el paquet temporal de prova + Llançar amb arguments sense shell · 3 h identificades

- **[F-01.E03](paquets/F-01.md)** · Crear el paquet temporal de prova · 90 min
- **[F-01.E04](paquets/F-01.md)** · Llançar amb arguments sense shell · 90 min

###### D · F-01.R1.C1.D3 · Llegir estat i final del playtest · 1 h identificades

- **[F-01.E05](paquets/F-01.md)** · Llegir estat i final del playtest · 60 min


##### C · F-01.R2.C1 · Controls, retorn i prova d’autor · 5,5 h identificades

###### D · F-01.R2.C1.D1 · Aturar només la prova activa + Restaurar la vista d’autor · 2,5 h identificades

- **[F-01.E06](paquets/F-01.md)** · Aturar només la prova activa · 60 min
- **[F-01.E07](paquets/F-01.md)** · Restaurar la vista d’autor · 90 min

###### D · F-01.R2.C1.D2 · Connectar Play/Stop a motor_editor + Verificar crear → provar → tornar · 3 h identificades

- **[F-01.E08](paquets/F-01.md)** · Connectar Play/Stop a motor_editor · 90 min
- **[F-01.E09](paquets/F-01.md)** · Verificar crear → provar → tornar · 90 min



#### B · F-02 · Provar des de la casella seleccionada · 6,25 h identificades

##### C · F-02.R1.C1 · Origen de prova concret · 6,25 h identificades

###### D · F-02.R1.C1.D1 · Auditar entrada i selecció existents + Validar casella seleccionada + Afegir origen temporal al playtest · 2,75 h identificades

- **[F-02.E01](paquets/F-02.md)** · Auditar entrada i selecció existents · 45 min
- **[F-02.E02](paquets/F-02.md)** · Validar casella seleccionada · 60 min
- **[F-02.E03](paquets/F-02.md)** · Afegir origen temporal al playtest · 60 min

###### D · F-02.R1.C1.D2 · Interpretar origen al runtime + Connectar acció Provar des d’aquí + Verificar casella bona i bloquejada · 3,5 h identificades

- **[F-02.E04](paquets/F-02.md)** · Interpretar origen al runtime · 90 min
- **[F-02.E05](paquets/F-02.md)** · Connectar acció Provar des d’aquí · 60 min
- **[F-02.E06](paquets/F-02.md)** · Verificar casella bona i bloquejada · 60 min




### A · A-U4-02 · Recàrrega observable del joc · 22,5 h identificades

#### B · F-03 · Recàrrega en calent · 13 h identificades

##### C · F-03.R1.C1 · Recàrrega transaccional · 7,5 h identificades

###### D · F-03.R1.C1.D1 · Classificar què es pot recarregar + Detectar canvis estables · 3 h identificades

- **[F-03.E01](paquets/F-03.md)** · Classificar què es pot recarregar · 90 min
- **[F-03.E02](paquets/F-03.md)** · Detectar canvis estables · 90 min

###### D · F-03.R1.C1.D2 · Preparar versió nova fora de l’estat viu + Comprovar dependències afectades · 3 h identificades

- **[F-03.E03](paquets/F-03.md)** · Preparar versió nova fora de l’estat viu · 90 min
- **[F-03.E04](paquets/F-03.md)** · Comprovar dependències afectades · 90 min

###### D · F-03.R1.C1.D3 · Publicar al límit de frame · 1,5 h identificades

- **[F-03.E05](paquets/F-03.md)** · Publicar al límit de frame · 90 min


##### C · F-03.R2.C1 · Política d’estat i observabilitat · 5,5 h identificades

###### D · F-03.R2.C1.D1 · Migrar o reiniciar estat dependent + Mostrar resultat al creador · 2,5 h identificades

- **[F-03.E06](paquets/F-03.md)** · Migrar o reiniciar estat dependent · 90 min
- **[F-03.E07](paquets/F-03.md)** · Mostrar resultat al creador · 60 min

###### D · F-03.R2.C1.D2 · Connectar watcher al playtest + Provar canvi vàlid i canvi trencat · 3 h identificades

- **[F-03.E08](paquets/F-03.md)** · Connectar watcher al playtest · 90 min
- **[F-03.E09](paquets/F-03.md)** · Provar canvi vàlid i canvi trencat · 90 min



#### B · F-04 · Consola i inspector d'estat · 9,5 h identificades

##### C · F-04.R1.C1 · Snapshots i filtratge · 4,5 h identificades

###### D · F-04.R1.C1.D1 · Inventariar logs i inspector existents + Capturar un estat de joc llegible + Filtrar registres acotats · 3,5 h identificades

- **[F-04.E01](paquets/F-04.md)** · Inventariar logs i inspector existents · 60 min
- **[F-04.E02](paquets/F-04.md)** · Capturar un estat de joc llegible · 90 min
- **[F-04.E03](paquets/F-04.md)** · Filtrar registres acotats · 60 min

###### D · F-04.R1.C1.D2 · Resoldre selecció d’entitat per handle · 1 h identificades

- **[F-04.E04](paquets/F-04.md)** · Resoldre selecció d’entitat per handle · 60 min


##### C · F-04.R2.C1 · Panells i diagnòstic útil · 5 h identificades

###### D · F-04.R2.C1.D1 · Projectar error a enllaç d’autor + Dibuixar consola i inspector · 2,5 h identificades

- **[F-04.E05](paquets/F-04.md)** · Projectar error a enllaç d’autor · 60 min
- **[F-04.E06](paquets/F-04.md)** · Dibuixar consola i inspector · 90 min

###### D · F-04.R2.C1.D2 · Connectar runtime de playtest al panell + Verificar error rastrejable al creador · 2,5 h identificades

- **[F-04.E07](paquets/F-04.md)** · Connectar runtime de playtest al panell · 90 min
- **[F-04.E08](paquets/F-04.md)** · Verificar error rastrejable al creador · 60 min




### A · A-U4-03 · Càmera i pistes executables · 22,5 h identificades

#### B · U4-01 · Càmera: seguiment, sacsejada, límits i dead zone · 9 h identificades

##### C · U4-01.R1.C1 · Càmera per dades i geometria · 5,5 h identificades

###### D · U4-01.R1.C1.D1 · Auditar càmera i escala isomètrica + Calcular seguiment amb zona morta · 2,5 h identificades

- **[U4-01.E01](paquets/U4-01.md)** · Auditar càmera i escala isomètrica · 60 min
- **[U4-01.E02](paquets/U4-01.md)** · Calcular seguiment amb zona morta · 90 min

###### D · U4-01.R1.C1.D2 · Limitar càmera a mapa real + Calcular sacsejada determinista · 3 h identificades

- **[U4-01.E03](paquets/U4-01.md)** · Limitar càmera a mapa real · 90 min
- **[U4-01.E04](paquets/U4-01.md)** · Calcular sacsejada determinista · 90 min


##### C · U4-01.R2.C1 · Composició i aplicació · 3,5 h identificades

###### D · U4-01.R2.C1.D1 · Composar base i efectes de càmera + Connectar tween i càmera existent + Verificar cantonada i zona morta · 3,5 h identificades

- **[U4-01.E05](paquets/U4-01.md)** · Composar base i efectes de càmera · 60 min
- **[U4-01.E06](paquets/U4-01.md)** · Connectar tween i càmera existent · 90 min
- **[U4-01.E07](paquets/U4-01.md)** · Verificar cantonada i zona morta · 60 min



#### B · U4-02 · Cinemàtiques amb pistes en paral·lel · 13,5 h identificades

##### C · U4-02.R1.C1 · Pistes tipades i rellotge comú · 7,5 h identificades

###### D · U4-02.R1.C1.D1 · Auditar cinemàtiques actuals abans d’estendre + Llegir una línia de temps · 3 h identificades

- **[U4-02.E01](paquets/U4-02.md)** · Auditar cinemàtiques actuals abans d’estendre · 90 min
- **[U4-02.E02](paquets/U4-02.md)** · Llegir una línia de temps · 90 min

###### D · U4-02.R1.C1.D2 · Determinar clips actius al temps t + Avançar pistes en un sol rellotge · 3 h identificades

- **[U4-02.E03](paquets/U4-02.md)** · Determinar clips actius al temps t · 90 min
- **[U4-02.E04](paquets/U4-02.md)** · Avançar pistes en un sol rellotge · 90 min

###### D · U4-02.R1.C1.D3 · Arbitrar pistes sobre el mateix canal · 1,5 h identificades

- **[U4-02.E05](paquets/U4-02.md)** · Arbitrar pistes sobre el mateix canal · 90 min


##### C · U4-02.R2.C1 · Efectes, cancel·lació i ús · 6 h identificades

###### D · U4-02.R2.C1.D1 · Executar accions pels ports existents + Finalitzar i tornar control al jugador · 3 h identificades

- **[U4-02.E06](paquets/U4-02.md)** · Executar accions pels ports existents · 90 min
- **[U4-02.E07](paquets/U4-02.md)** · Finalitzar i tornar control al jugador · 90 min

###### D · U4-02.R2.C1.D2 · Connectar una escena paral·lela de producte + Verificar independència dels FPS · 3 h identificades

- **[U4-02.E08](paquets/U4-02.md)** · Connectar una escena paral·lela de producte · 90 min
- **[U4-02.E09](paquets/U4-02.md)** · Verificar independència dels FPS · 90 min




### A · A-U4-04 · Autoría de timelines i salt segur · 22 h identificades

#### B · U4-03 · Editor de línia de temps · 15 h identificades

##### C · U4-03.R1.C1 · Edició reversible de pistes · 7,5 h identificades

###### D · U4-03.R1.C1.D1 · Auditar document i lloc del panell + Projectar temps i píxels + Inserir un clip validat · 3,5 h identificades

- **[U4-03.E01](paquets/U4-03.md)** · Auditar document i lloc del panell · 60 min
- **[U4-03.E02](paquets/U4-03.md)** · Projectar temps i píxels · 60 min
- **[U4-03.E03](paquets/U4-03.md)** · Inserir un clip validat · 90 min

###### D · U4-03.R1.C1.D2 · Moure un clip amb snap + Redimensionar durada · 3 h identificades

- **[U4-03.E04](paquets/U4-03.md)** · Moure un clip amb snap · 90 min
- **[U4-03.E05](paquets/U4-03.md)** · Redimensionar durada · 90 min

###### D · U4-03.R1.C1.D3 · Reordenar pistes sense canviar IDs · 1 h identificades

- **[U4-03.E06](paquets/U4-03.md)** · Reordenar pistes sense canviar IDs · 60 min


##### C · U4-03.R2.C1 · Previsualització i persistència · 7,5 h identificades

###### D · U4-03.R2.C1.D1 · Previsualitzar un temps sense efectes irreversibles + Dibuixar pistes amb focus i escala · 3 h identificades

- **[U4-03.E07](paquets/U4-03.md)** · Previsualitzar un temps sense efectes irreversibles · 90 min
- **[U4-03.E08](paquets/U4-03.md)** · Dibuixar pistes amb focus i escala · 90 min

###### D · U4-03.R2.C1.D2 · Aplicar canvi i guardar/reobrir + Connectar amb l’editor de comandes · 3 h identificades

- **[U4-03.E09](paquets/U4-03.md)** · Aplicar canvi i guardar/reobrir · 90 min
- **[U4-03.E10](paquets/U4-03.md)** · Connectar amb l’editor de comandes · 90 min

###### D · U4-03.R2.C1.D3 · Verificar escena autorada sencera · 1,5 h identificades

- **[U4-03.E11](paquets/U4-03.md)** · Verificar escena autorada sencera · 90 min



#### B · U4-04 · Saltar cinemàtiques i EscenaNoqueo com a cinemàtica · 7 h identificades

##### C · U4-04.R1.C1 · Salt amb efectes finals controlats · 7 h identificades

###### D · U4-04.R1.C1.D1 · Auditar efectes de Noqueo i escenes saltables + Construir pla de salt · 2,5 h identificades

- **[U4-04.E01](paquets/U4-04.md)** · Auditar efectes de Noqueo i escenes saltables · 60 min
- **[U4-04.E02](paquets/U4-04.md)** · Construir pla de salt · 90 min

###### D · U4-04.R1.C1.D2 · Aplicar final segur + Convertir EscenaNoqueo a dades · 3 h identificades

- **[U4-04.E03](paquets/U4-04.md)** · Aplicar final segur · 90 min
- **[U4-04.E04](paquets/U4-04.md)** · Convertir EscenaNoqueo a dades · 90 min

###### D · U4-04.R1.C1.D3 · Connectar acció de salt i provar-la · 1,5 h identificades

- **[U4-04.E05](paquets/U4-04.md)** · Connectar acció de salt i provar-la · 90 min




### A · A-U4-05 · Grups reutilitzables i overlay de depuració · 18,5 h identificades

#### B · U4-05 · Prefabs de grups d'objectes (MAP-08) · 12 h identificades

##### C · U4-05.R1.C1 · Definició i instanciació · 6,5 h identificades

###### D · U4-05.R1.C1.D1 · Auditar objectes agrupats i MAP-08 + Capturar un grup com a prefab · 2,5 h identificades

- **[U4-05.E01](paquets/U4-05.md)** · Auditar objectes agrupats i MAP-08 · 60 min
- **[U4-05.E02](paquets/U4-05.md)** · Capturar un grup com a prefab · 90 min

###### D · U4-05.R1.C1.D2 · Validar recursos i espai del prefab + Instanciar amb identitats noves · 3 h identificades

- **[U4-05.E03](paquets/U4-05.md)** · Validar recursos i espai del prefab · 90 min
- **[U4-05.E04](paquets/U4-05.md)** · Instanciar amb identitats noves · 90 min

###### D · U4-05.R1.C1.D3 · Serialitzar definició i referències · 1 h identificades

- **[U4-05.E05](paquets/U4-05.md)** · Serialitzar definició i referències · 60 min


##### C · U4-05.R2.C1 · Edició i ús d’autor · 5,5 h identificades

###### D · U4-05.R2.C1.D1 · Aplicar grup amb una operació d’undo + Definir vincle i overrides · 2,5 h identificades

- **[U4-05.E06](paquets/U4-05.md)** · Aplicar grup amb una operació d’undo · 90 min
- **[U4-05.E07](paquets/U4-05.md)** · Definir vincle i overrides · 60 min

###### D · U4-05.R2.C1.D2 · Connectar biblioteca de grups al panell + Verificar dues còpies i desfer · 3 h identificades

- **[U4-05.E08](paquets/U4-05.md)** · Connectar biblioteca de grups al panell · 90 min
- **[U4-05.E09](paquets/U4-05.md)** · Verificar dues còpies i desfer · 90 min



#### B · U4-06 · Overlay de depuració: col·lisions, regions, rutes · 6,5 h identificades

##### C · U4-06.R1.C1 · Geometria de depuració · 6,5 h identificades

###### D · U4-06.R1.C1.D1 · Auditar formes i overlays existents + Projectar col·lisions sense recomputar regles + Projectar regions i IDs · 3 h identificades

- **[U4-06.E01](paquets/U4-06.md)** · Auditar formes i overlays existents · 60 min
- **[U4-06.E02](paquets/U4-06.md)** · Projectar col·lisions sense recomputar regles · 60 min
- **[U4-06.E03](paquets/U4-06.md)** · Projectar regions i IDs · 60 min

###### D · U4-06.R1.C1.D2 · Projectar rutes i casella bloquejada + Connectar controls i render debug + Cotejar dibuix i comportament · 3,5 h identificades

- **[U4-06.E04](paquets/U4-06.md)** · Projectar rutes i casella bloquejada · 60 min
- **[U4-06.E05](paquets/U4-06.md)** · Connectar controls i render debug · 90 min
- **[U4-06.E06](paquets/U4-06.md)** · Cotejar dibuix i comportament · 60 min




### A · A-U4-06 · Emissor genèric amb límit de recursos · 9,5 h identificades

#### B · U4-07 · Emissor de partícules genèric · 9,5 h identificades

##### C · U4-07.R1.C1 · Partícules amb pressupost finit · 6 h identificades

###### D · U4-07.R1.C1.D1 · Auditar partícules i helpers ja existents + Llegir paràmetres de l’emissor + Calcular emissions amb residu · 3,5 h identificades

- **[U4-07.E01](paquets/U4-07.md)** · Auditar partícules i helpers ja existents · 60 min
- **[U4-07.E02](paquets/U4-07.md)** · Llegir paràmetres de l’emissor · 90 min
- **[U4-07.E03](paquets/U4-07.md)** · Calcular emissions amb residu · 60 min

###### D · U4-07.R1.C1.D2 · Crear partícula amb llavor explícita + Avançar estat i caducar partícules · 2,5 h identificades

- **[U4-07.E04](paquets/U4-07.md)** · Crear partícula amb llavor explícita · 60 min
- **[U4-07.E05](paquets/U4-07.md)** · Avançar estat i caducar partícules · 90 min


##### C · U4-07.R2.C1 · Presentació i ús real · 3,5 h identificades

###### D · U4-07.R2.C1.D1 · Projectar estat a instàncies visuals + Connectar emissor a una escena real + Mesurar límit i repetibilitat · 3,5 h identificades

- **[U4-07.E06](paquets/U4-07.md)** · Projectar estat a instàncies visuals · 60 min
- **[U4-07.E07](paquets/U4-07.md)** · Connectar emissor a una escena real · 90 min
- **[U4-07.E08](paquets/U4-07.md)** · Mesurar límit i repetibilitat · 60 min





