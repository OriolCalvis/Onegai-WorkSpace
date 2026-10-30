# Microtasques E del Pla U

Generat des de `pla.json`; camps complets a la fitxa de cada paquet.

| ID | Min | Operació / resultat | Dependències E |
|---|---:|---|---|
| [PREP.E01](paquets/PREP.md) | 45 | Reconciliar arrel Git i arrel del motor | port de paquet |
| [PREP.E02](paquets/PREP.md) | 60 | Verificar la porta pre-commit real | PREP.E01 |
| [PREP.E03](paquets/PREP.md) | 60 | Revisar els tres fitxers de fabrica abans d’instal·lar | PREP.E02 |
| [PREP.E04](paquets/PREP.md) | 60 | Adaptar cwd i presets al monorepo | PREP.E03 |
| [PREP.E05](paquets/PREP.md) | 90 | Ajustar extracció de fallades amb sortida real | PREP.E04 |
| [PREP.E06](paquets/PREP.md) | 60 | Definir comparació de fallades heretades | PREP.E05 |
| [PREP.E07](paquets/PREP.md) | 60 | Provar depuració prioritzant errors nous | PREP.E06 |
| [PREP.E08](paquets/PREP.md) | 90 | Capturar base immutable abans del primer C++ | PREP.E07 |
| [PREP.E09](paquets/PREP.md) | 60 | Verificar vigilància bounded i exclusió de logs | PREP.E08 |
| [PREP.E10](paquets/PREP.md) | 90 | Verificar lliurament des del commit | PREP.E09 |
| [PREP.E11](paquets/PREP.md) | 90 | Congelar interfícies del primer lot de perseguidors | PREP.E10, U1-02.E05 |
| [PREP.E12](paquets/PREP.md) | 45 | Calibrar l’escala amb cinc E acabades | PREP.E11, U1-02.E05 |
| [U1-01.E01](paquets/U1-01.md) | 45 | Inventariar backend i recursos de so | PREP.E10 |
| [U1-01.E02](paquets/U1-01.md) | 90 | Llegir correspondències de cues en dades | U1-01.E01 |
| [U1-01.E03](paquets/U1-01.md) | 60 | Seleccionar música del lloc | U1-01.E02 |
| [U1-01.E04](paquets/U1-01.md) | 60 | Seleccionar l’efecte d’un resultat | U1-01.E03 |
| [U1-01.E05](paquets/U1-01.md) | 60 | Mantenir la música idempotent | U1-01.E04 |
| [U1-01.E06](paquets/U1-01.md) | 90 | Definir el contracte del fundido | U1-01.E05 |
| [U1-01.E07](paquets/U1-01.md) | 90 | Avançar un fundido amb el temps | U1-01.E06 |
| [U1-01.E08](paquets/U1-01.md) | 60 | Aplicar els volums ja guardats | U1-01.E07 |
| [U1-01.E09](paquets/U1-01.md) | 60 | Tolerar falta de dispositiu o fitxer | U1-01.E08 |
| [U1-01.E10](paquets/U1-01.md) | 90 | Crear i destruir el servei al límit correcte | U1-01.E09 |
| [U1-01.E11](paquets/U1-01.md) | 90 | Connectar canvi de lloc i resultats de combat | U1-01.E10 |
| [U1-01.E12](paquets/U1-01.md) | 60 | Connectar Opcions al servei viu | U1-01.E11 |
| [U1-01.E13](paquets/U1-01.md) | 60 | Registrar acceptació funcional i dependència de recursos | U1-01.E12 |
| [G-11.E01](paquets/G-11.md) | 45 | Resoldre sis ranures existents vs tres proposades | PREP.E10 |
| [G-11.E02](paquets/G-11.md) | 60 | Projectar una fila de ranura | G-11.E01 |
| [G-11.E03](paquets/G-11.md) | 45 | Seleccionar una ranura amb teclat | G-11.E02 |
| [G-11.E04](paquets/G-11.md) | 60 | Decidir si guardar requereix consentiment | G-11.E03 |
| [G-11.E05](paquets/G-11.md) | 60 | Descriure la pèrdua en carregar | G-11.E04 |
| [G-11.E06](paquets/G-11.md) | 60 | Verificar round-trip entre ranures de projecte | G-11.E05 |
| [G-11.E07](paquets/G-11.md) | 60 | Carregar sense destruir la sessió bona | G-11.E06 |
| [G-11.E08](paquets/G-11.md) | 90 | Registrar el servei en una biblioteca | G-11.E07 |
| [G-11.E09](paquets/G-11.md) | 90 | Dibuixar les files amb el sistema de menú actual | G-11.E08 |
| [G-11.E10](paquets/G-11.md) | 90 | Connectar pausa i selector | G-11.E09 |
| [G-11.E11](paquets/G-11.md) | 90 | Executar una sola càrrega confirmada | G-11.E10 |
| [G-11.E12](paquets/G-11.md) | 90 | Provar les tres sortides de la pantalla | G-11.E11 |
| [G-01.E01](paquets/G-01.md) | 60 | Identificar els punts de publicació segura | PREP.E10 |
| [G-01.E02](paquets/G-01.md) | 60 | Classificar la petició d’autosave | G-01.E01 |
| [G-01.E03](paquets/G-01.md) | 45 | Seleccionar una ruta automàtica estable | G-01.E02 |
| [G-01.E04](paquets/G-01.md) | 60 | Validar escriptura atòmica existent | G-01.E03 |
| [G-01.E05](paquets/G-01.md) | 60 | Provar interrupció abans de publicar | G-01.E04 |
| [G-01.E06](paquets/G-01.md) | 60 | Triar el guardat més recent que es pot carregar | G-01.E05 |
| [G-01.E07](paquets/G-01.md) | 60 | Importar la ranura ràpida antiga sense pèrdua | G-01.E06 |
| [G-01.E08](paquets/G-01.md) | 90 | Connectar Continuar amb el mateix servei | G-01.E07 |
| [G-01.E09](paquets/G-01.md) | 90 | Connectar els dos punts d’autoguardat | G-01.E08 |
| [G-01.E10](paquets/G-01.md) | 90 | Verificar recuperació completa | G-01.E09 |
| [U1-02.E01](paquets/U1-02.md) | 60 | Fixar la invalidació per creixement | PREP.E10 |
| [U1-02.E02](paquets/U1-02.md) | 60 | Caracteritzar la creació diferida | U1-02.E01 |
| [U1-02.E03](paquets/U1-02.md) | 60 | Caracteritzar la baixa immediata | U1-02.E02 |
| [U1-02.E04](paquets/U1-02.md) | 60 | Provar la identitat després de reutilitzar slot | U1-02.E03 |
| [U1-02.E05](paquets/U1-02.md) | 90 | Fixar l’estratègia d’adaptació | U1-02.E04 |
| [U1-02.E06](paquets/U1-02.md) | 45 | Convertir identitat a la frontera | U1-02.E05, PREP.E11 |
| [U1-02.E07](paquets/U1-02.md) | 90 | Reservar sense publicar entitats | U1-02.E06 |
| [U1-02.E08](paquets/U1-02.md) | 90 | Resoldre només entitats visibles | U1-02.E07 |
| [U1-02.E09](paquets/U1-02.md) | 90 | Aplicar altes i baixes al límit del tick | U1-02.E08 |
| [U1-02.E10](paquets/U1-02.md) | 90 | Recórrer amb mutacions diferides | U1-02.E09 |
| [U1-02.E11](paquets/U1-02.md) | 60 | Consultar l’espai a través del magatzem | U1-02.E10 |
| [U1-02.E12](paquets/U1-02.md) | 45 | Auditar el mirall de posició | U1-02.E11 |
| [U1-02.E13](paquets/U1-02.md) | 90 | Connectar el magatzem a World | U1-02.E12 |
| [U1-02.E14](paquets/U1-02.md) | 90 | Regressió de sessió i desat | U1-02.E13 |
| [U1-03.E01](paquets/U1-03.md) | 60 | Fixar distància, visió i abandonament | PREP.E10 |
| [U1-03.E02](paquets/U1-03.md) | 30 | Calcular distància de caselles | U1-03.E01 |
| [U1-03.E03](paquets/U1-03.md) | 60 | Classificar el camp de visió | U1-03.E01 |
| [U1-03.E04](paquets/U1-03.md) | 60 | Adquirir objectiu | U1-03.E02, U1-03.E03 |
| [U1-03.E05](paquets/U1-03.md) | 60 | Determinar pèrdua d’objectiu | U1-03.E01 |
| [U1-03.E06](paquets/U1-03.md) | 90 | Escollir el següent pas transitable | U1-03.E01 |
| [U1-03.E07](paquets/U1-03.md) | 90 | No travessar actors ni desnivells | U1-03.E01 |
| [U1-03.E08](paquets/U1-03.md) | 60 | Consumir temps sense dependre dels FPS | U1-03.E01 |
| [U1-03.E09](paquets/U1-03.md) | 90 | Canviar patrulla a persecució | U1-03.E04, U1-03.E05, U1-03.E06, U1-03.E07, U1-03.E08 |
| [U1-03.E10](paquets/U1-03.md) | 90 | Retornar a una patrulla vàlida | U1-03.E06, U1-03.E09 |
| [U1-03.E11](paquets/U1-03.md) | 60 | Demanar combat en el contacte | U1-03.E09, U1-03.E10 |
| [U1-03.E12](paquets/U1-03.md) | 90 | Connectar percepció i col·lisió en la sessió | U1-03.E07, U1-03.E11 |
| [U1-03.E13](paquets/U1-03.md) | 60 | Reiniciar IA després de carregar | U1-03.E09 |
| [U1-03.E14](paquets/U1-03.md) | 45 | Decidir el senyal de detecció | U1-03.E09 |
| [U1-03.E15](paquets/U1-03.md) | 90 | Recórrer el bucle jugable de persecució | U1-03.E12, U1-03.E13, U1-03.E14 |
| [U1-04.E01](paquets/U1-04.md) | 45 | Resoldre tres franges reals vs quatre descrites | PREP.E10 |
| [U1-04.E02](paquets/U1-04.md) | 90 | Llegir un horari sense moure el PNJ | U1-04.E01 |
| [U1-04.E03](paquets/U1-04.md) | 60 | Triar el destí d’una franja | U1-04.E02 |
| [U1-04.E04](paquets/U1-04.md) | 60 | Validar ubicacions nocturnes com a proposta | U1-04.E03 |
| [U1-04.E05](paquets/U1-04.md) | 45 | Ajornar un canvi si el PNJ està ocupat | U1-04.E04 |
| [U1-04.E06](paquets/U1-04.md) | 90 | Escollir caminar o trasllat fora de vista | U1-04.E05 |
| [U1-04.E07](paquets/U1-04.md) | 90 | Demanar un pas al navegador existent | U1-04.E06 |
| [U1-04.E08](paquets/U1-04.md) | 90 | Aplicar el canvi només un cop per franja | U1-04.E07 |
| [U1-04.E09](paquets/U1-04.md) | 90 | Preservar identitat en carregar i canviar mapa | U1-04.E08 |
| [U1-04.E10](paquets/U1-04.md) | 60 | Connectar el moment narratiu a la sessió | U1-04.E09 |
| [U1-04.E11](paquets/U1-04.md) | 90 | Comprovar dia, conversa i nit al joc | U1-04.E10 |
| [U1-05.E01](paquets/U1-05.md) | 60 | Localitzar el tint global de món i les capes UI | PREP.E10 |
| [U1-05.E02](paquets/U1-05.md) | 60 | Llegir colors de franja | U1-05.E01 |
| [U1-05.E03](paquets/U1-05.md) | 45 | Triar el color objectiu | U1-05.E02 |
| [U1-05.E04](paquets/U1-05.md) | 60 | Interpolar un tint amb límits | U1-05.E03 |
| [U1-05.E05](paquets/U1-05.md) | 60 | Reorientar una transició en curs | U1-05.E04 |
| [U1-05.E06](paquets/U1-05.md) | 60 | Avançar tint amb dt sense alterar franja | U1-05.E05 |
| [U1-05.E07](paquets/U1-05.md) | 90 | Passar el tint al render de món | U1-05.E06 |
| [U1-05.E08](paquets/U1-05.md) | 60 | Mostrar paleta a Oriol amb el mateix enquadrament | U1-05.E07 |
| [U2-01.E01](paquets/U2-01.md) | 90 | Localitzar propietaris dels serveis | PREP.E10 |
| [U2-01.E02](paquets/U2-01.md) | 90 | Mesurar dependències prohibides | U2-01.E01 |
| [U2-01.E03](paquets/U2-01.md) | 90 | Fixar el contracte entre capes | U2-01.E02 |
| [U2-01.E04](paquets/U2-01.md) | 60 | Caracteritzar l’inici actual de Boundington | U2-01.E03 |
| [U2-01.E05](paquets/U2-01.md) | 90 | Construir una prova sense contingut Onegai | U2-01.E04 |
| [U2-01.E06](paquets/U2-01.md) | 60 | Publicar el mapa dels punts de tall | U2-01.E05 |
| [U2-02.E01](paquets/U2-02.md) | 60 | Auditar ajustos existents i llegat | PREP.E10 |
| [U2-02.E02](paquets/U2-02.md) | 90 | Llegir escena inicial i idioma | U2-02.E01 |
| [U2-02.E03](paquets/U2-02.md) | 90 | Normalitzar rutes del projecte | U2-02.E02 |
| [U2-02.E04](paquets/U2-02.md) | 90 | Validar escena inicial resolta | U2-02.E03 |
| [U2-02.E05](paquets/U2-02.md) | 90 | Resoldre catàlegs declarats | U2-02.E04 |
| [U2-02.E06](paquets/U2-02.md) | 90 | Serialitzar ajustos additius | U2-02.E05 |
| [U2-02.E07](paquets/U2-02.md) | 60 | Propagar idioma i títol | U2-02.E06 |
| [U2-02.E08](paquets/U2-02.md) | 90 | Connectar lector i escriptor del projecte | U2-02.E07 |
| [U2-02.E09](paquets/U2-02.md) | 60 | Verificar dues configuracions diferents | U2-02.E08 |
| [G-08.E01](paquets/G-08.md) | 60 | Auditar CLI i títol ja existents | PREP.E10 |
| [G-08.E02](paquets/G-08.md) | 60 | Interpretar --proyecto | G-08.E01 |
| [G-08.E03](paquets/G-08.md) | 90 | Preparar l’arrencada des del manifest | G-08.E02 |
| [G-08.E04](paquets/G-08.md) | 60 | Escollir l’opció de títol | G-08.E03 |
| [G-08.E05](paquets/G-08.md) | 90 | Publicar el pla d’arrencada | G-08.E04 |
| [G-08.E06](paquets/G-08.md) | 90 | Connectar el CLI a la ruta única | G-08.E05 |
| [G-08.E07](paquets/G-08.md) | 60 | Verificar arrencada fora del checkout | G-08.E06 |
| [U2-03.E01](paquets/U2-03.md) | 90 | Auditar el límit de mòdul sobre el graf real | PREP.E10 |
| [U2-03.E02](paquets/U2-03.md) | 90 | Congelar serveis disponibles per al joc | U2-03.E01 |
| [U2-03.E03](paquets/U2-03.md) | 90 | Definir inicialització del mòdul | U2-03.E02 |
| [U2-03.E04](paquets/U2-03.md) | 90 | Definir tick del mòdul | U2-03.E03 |
| [U2-03.E05](paquets/U2-03.md) | 60 | Definir tancament idempotent | U2-03.E04 |
| [U2-03.E06](paquets/U2-03.md) | 90 | Adaptar regles RPG al port | U2-03.E05 |
| [U2-03.E07](paquets/U2-03.md) | 90 | Construir el mòdul RPG separat | U2-03.E06 |
| [U2-03.E08](paquets/U2-03.md) | 90 | Instanciar un mòdul des de configuració | U2-03.E07 |
| [U2-03.E09](paquets/U2-03.md) | 90 | Desplaçar el primer relleu de composició | U2-03.E08 |
| [U2-03.E10](paquets/U2-03.md) | 90 | Connectar el mòdul a juego | U2-03.E09 |
| [U2-03.E11](paquets/U2-03.md) | 90 | Verificar límit motor/RPG al build | U2-03.E10 |
| [U2-04.E01](paquets/U2-04.md) | 60 | Localitzar la creació real de projectes | PREP.E10 |
| [U2-04.E02](paquets/U2-04.md) | 60 | Validar els paràmetres de plantilla | U2-04.E01 |
| [U2-04.E03](paquets/U2-04.md) | 90 | Expandir una plantilla en memòria | U2-04.E02 |
| [U2-04.E04](paquets/U2-04.md) | 90 | Publicar un projecte complet | U2-04.E03 |
| [U2-04.E05](paquets/U2-04.md) | 45 | Llistar les tres plantilles amb descripció | U2-04.E04 |
| [U2-04.E06](paquets/U2-04.md) | 90 | Connectar elecció al Hub | U2-04.E05 |
| [U2-04.E07](paquets/U2-04.md) | 90 | Verificar crear, reobrir i jugar | U2-04.E06 |
| [E-11.E01](paquets/E-11.md) | 60 | Auditar panell i estat existents | PREP.E10 |
| [E-11.E02](paquets/E-11.md) | 90 | Construir nodes de l’arbre | E-11.E01 |
| [E-11.E03](paquets/E-11.md) | 60 | Filtrar sense perdre selecció | E-11.E02 |
| [E-11.E04](paquets/E-11.md) | 60 | Resoldre acció d’obrir node | E-11.E03 |
| [E-11.E05](paquets/E-11.md) | 90 | Aplicar renombrat amb referències | E-11.E04 |
| [E-11.E06](paquets/E-11.md) | 90 | Dibuixar arbre amb focus accessible | E-11.E05 |
| [E-11.E07](paquets/E-11.md) | 90 | Connectar el node a document real | E-11.E06 |
| [E-11.E08](paquets/E-11.md) | 60 | Verificar recorregut d’autor | E-11.E07 |
| [U2-05.E01](paquets/U2-05.md) | 90 | Inventariar funcionalitats dels dos editors | PREP.E10 |
| [U2-05.E02](paquets/U2-05.md) | 60 | Decidir host únic i compatibilitat de CLI | U2-05.E01 |
| [U2-05.E03](paquets/U2-05.md) | 90 | Extreure el primer servei de document | U2-05.E02 |
| [U2-05.E04](paquets/U2-05.md) | 90 | Compartir encaminament de comandes | U2-05.E03 |
| [U2-05.E05](paquets/U2-05.md) | 90 | Connectar vista de mapa al host únic | U2-05.E04 |
| [U2-05.E06](paquets/U2-05.md) | 90 | Connectar objectes i propietats | U2-05.E05 |
| [U2-05.E07](paquets/U2-05.md) | 90 | Connectar events i validació | U2-05.E06 |
| [U2-05.E08](paquets/U2-05.md) | 60 | Unificar ajuda i controls anunciats | U2-05.E07 |
| [U2-05.E09](paquets/U2-05.md) | 90 | Eliminar el camí duplicat verificat | U2-05.E08 |
| [U2-05.E10](paquets/U2-05.md) | 90 | Fer una sessió d’autor completa | U2-05.E09 |
| [DEC-10.E01](paquets/DEC-10.md) | 90 | Comparar política real i opcions | PREP.E10 |
| [DEC-10.E02](paquets/DEC-10.md) | 60 | Fixar exemples entrada-sortida | DEC-10.E01 |
| [DEC-10.E03](paquets/DEC-10.md) | 30 | Registrar el vist de la política | DEC-10.E02 |
| [U3-01.E01](paquets/U3-01.md) | 90 | Inventariar els dos camins narratius | PREP.E10 |
| [U3-01.E02](paquets/U3-01.md) | 90 | Normalitzar una condició de beat | U3-01.E01 |
| [U3-01.E03](paquets/U3-01.md) | 90 | Adaptar una llista d’efectes | U3-01.E02 |
| [U3-01.E04](paquets/U3-01.md) | 90 | Seleccionar pàgina segons contracte | U3-01.E03 |
| [U3-01.E05](paquets/U3-01.md) | 90 | Traduir un beat al model comú | U3-01.E04 |
| [U3-01.E06](paquets/U3-01.md) | 90 | Executar a través del host existent | U3-01.E05 |
| [U3-01.E07](paquets/U3-01.md) | 90 | Preservar progrés dels guardats | U3-01.E06 |
| [U3-01.E08](paquets/U3-01.md) | 90 | Connectar un recorregut vertical de campanya | U3-01.E07 |
| [U3-01.E09](paquets/U3-01.md) | 90 | Desactivar el camí antic només en paritat | U3-01.E08 |
| [U3-02.E01](paquets/U3-02.md) | 60 | Auditar capa de regions existent | PREP.E10 |
| [U3-02.E02](paquets/U3-02.md) | 60 | Consultar pertinença d’una casella | U3-02.E01 |
| [U3-02.E03](paquets/U3-02.md) | 90 | Calcular entrada i sortida | U3-02.E02 |
| [U3-02.E04](paquets/U3-02.md) | 90 | Produir permanència sense inundar events | U3-02.E03 |
| [U3-02.E05](paquets/U3-02.md) | 60 | Ordenar fets simultanis | U3-02.E04 |
| [U3-02.E06](paquets/U3-02.md) | 90 | Trametre al runner existent | U3-02.E05 |
| [U3-02.E07](paquets/U3-02.md) | 60 | Reiniciar presència en transició de mapa | U3-02.E06 |
| [U3-02.E08](paquets/U3-02.md) | 90 | Connectar i provar tres disparadors | U3-02.E07 |
| [U3-03.E01](paquets/U3-03.md) | 60 | Auditar crides, etiquetes i bucles existents | PREP.E10 |
| [U3-03.E02](paquets/U3-03.md) | 60 | Validar referència a event comú | U3-03.E01 |
| [U3-03.E03](paquets/U3-03.md) | 90 | Apilar una crida i el seu retorn | U3-03.E02 |
| [U3-03.E04](paquets/U3-03.md) | 60 | Retornar d’una crida comuna | U3-03.E03 |
| [U3-03.E05](paquets/U3-03.md) | 60 | Indexar etiquetes sense duplicats | U3-03.E04 |
| [U3-03.E06](paquets/U3-03.md) | 90 | Resoldre salt i trencament de bucle | U3-03.E05 |
| [U3-03.E07](paquets/U3-03.md) | 90 | Limitar treball per tick | U3-03.E06 |
| [U3-03.E08](paquets/U3-03.md) | 90 | Connectar al catàleg i host del joc | U3-03.E07 |
| [U3-07.E01](paquets/U3-07.md) | 60 | Inventariar interpolacions i rellotges reals | PREP.E10 |
| [U3-07.E02](paquets/U3-07.md) | 45 | Normalitzar el progrés temporal | U3-07.E01 |
| [U3-07.E03](paquets/U3-07.md) | 60 | Calcular easing definit en dades | U3-07.E02 |
| [U3-07.E04](paquets/U3-07.md) | 45 | Interpolar escalar | U3-07.E03 |
| [U3-07.E05](paquets/U3-07.md) | 60 | Interpolar vector amb la mateixa política | U3-07.E04 |
| [U3-07.E06](paquets/U3-07.md) | 90 | Avançar i completar una sola vegada | U3-07.E05 |
| [U3-07.E07](paquets/U3-07.md) | 60 | Cancel·lar amb política explícita | U3-07.E06 |
| [U3-07.E08](paquets/U3-07.md) | 90 | Connectar un consumidor de producte | U3-07.E07 |
| [U3-04.E01](paquets/U3-04.md) | 90 | Congelar DTOs i comandes suportades | PREP.E10 |
| [U3-04.E02](paquets/U3-04.md) | 90 | Validar una ordre de moure actor | U3-04.E01 |
| [U3-04.E03](paquets/U3-04.md) | 90 | Avançar moviment d’actor | U3-04.E02 |
| [U3-04.E04](paquets/U3-04.md) | 90 | Validar i llançar una ordre de càmera | U3-04.E03 |
| [U3-04.E05](paquets/U3-04.md) | 60 | Validar i llançar una ordre de so | U3-04.E04 |
| [U3-04.E06](paquets/U3-04.md) | 90 | Preparar opcions d’un diàleg | U3-04.E05 |
| [U3-04.E07](paquets/U3-04.md) | 90 | Recollir resultat d’una acció asíncrona | U3-04.E06 |
| [U3-04.E08](paquets/U3-04.md) | 90 | Connectar accions al EventHost existent | U3-04.E07 |
| [U3-04.E09](paquets/U3-04.md) | 90 | Fer una escena amb les quatre ordres | U3-04.E08 |
| [E-05.E01](paquets/E-05.md) | 60 | Auditar operacions de pàgines ja existents | PREP.E10 |
| [E-05.E02](paquets/E-05.md) | 60 | Crear una pàgina vàlida | E-05.E01 |
| [E-05.E03](paquets/E-05.md) | 90 | Modificar una condició | E-05.E02 |
| [E-05.E04](paquets/E-05.md) | 90 | Reordenar amb prioritat visible | E-05.E03 |
| [E-05.E05](paquets/E-05.md) | 90 | Eliminar pàgina amb desfer | E-05.E04 |
| [E-05.E06](paquets/E-05.md) | 90 | Construir l’explicació de pàgina activa | E-05.E05 |
| [E-05.E07](paquets/E-05.md) | 90 | Dibuixar pestanyes i camps de condició | E-05.E06 |
| [E-05.E08](paquets/E-05.md) | 90 | Aplicar una edició al document | E-05.E07 |
| [E-05.E09](paquets/E-05.md) | 90 | Connectar trigger de regió al formulari | E-05.E08 |
| [E-05.E10](paquets/E-05.md) | 90 | Verificar dues pàgines jugables | E-05.E09 |
| [E-06.E01](paquets/E-06.md) | 60 | Auditar catàleg de comandes i llista existents | PREP.E10 |
| [E-06.E02](paquets/E-06.md) | 90 | Projectar comandes en files | E-06.E01 |
| [E-06.E03](paquets/E-06.md) | 60 | Crear una comanda amb defaults vàlids | E-06.E02 |
| [E-06.E04](paquets/E-06.md) | 90 | Editar un paràmetre amb validació | E-06.E03 |
| [E-06.E05](paquets/E-06.md) | 90 | Inserir comanda en camí vàlid | E-06.E04 |
| [E-06.E06](paquets/E-06.md) | 90 | Moure bloc sense trencar l’arbre | E-06.E05 |
| [E-06.E07](paquets/E-06.md) | 90 | Copiar i enganxar amb nous IDs quan calgui | E-06.E06 |
| [E-06.E08](paquets/E-06.md) | 90 | Dibuixar resum i formulari d’una ordre | E-06.E07 |
| [E-06.E09](paquets/E-06.md) | 90 | Mostrar errors al comandament exacte | E-06.E08 |
| [E-06.E10](paquets/E-06.md) | 90 | Aplicar edició amb una sola operació d’undo | E-06.E09 |
| [E-06.E11](paquets/E-06.md) | 90 | Fer event sense editar JSON | E-06.E10 |
| [E-08.E01](paquets/E-08.md) | 60 | Auditar depurador i trace runtime | PREP.E10 |
| [E-08.E02](paquets/E-08.md) | 60 | Projectar el punt d’execució | E-08.E01 |
| [E-08.E03](paquets/E-08.md) | 60 | Resoldre el comandament clicat a breakpoint | E-08.E02 |
| [E-08.E04](paquets/E-08.md) | 60 | Aplicar pas únic des del panell | E-08.E03 |
| [E-08.E05](paquets/E-08.md) | 90 | Dibuixar trace i valors inspeccionables | E-08.E04 |
| [E-08.E06](paquets/E-08.md) | 90 | Connectar i comprovar un breakpoint real | E-08.E05 |
| [U3-05.E01](paquets/U3-05.md) | 60 | Auditar reproductors i formats existents | PREP.E10 |
| [U3-05.E02](paquets/U3-05.md) | 90 | Llegir un clip en JSON | U3-05.E01 |
| [U3-05.E03](paquets/U3-05.md) | 60 | Indexar events de fotograma | U3-05.E02 |
| [U3-05.E04](paquets/U3-05.md) | 90 | Avançar cursor d’un clip | U3-05.E03 |
| [U3-05.E05](paquets/U3-05.md) | 60 | Canviar clip amb política de reinici | U3-05.E04 |
| [U3-05.E06](paquets/U3-05.md) | 90 | Separar event lògic d’event visual | U3-05.E05 |
| [U3-05.E07](paquets/U3-05.md) | 90 | Connectar actor al reproductor de clip | U3-05.E06 |
| [U3-05.E08](paquets/U3-05.md) | 60 | Verificar loop i clip no cíclic | U3-05.E07 |
| [U3-06.E01](paquets/U3-06.md) | 60 | Auditar qui tria animacions avui | PREP.E10 |
| [U3-06.E02](paquets/U3-06.md) | 90 | Llegir estats i transicions | U3-06.E01 |
| [U3-06.E03](paquets/U3-06.md) | 90 | Seleccionar transició | U3-06.E02 |
| [U3-06.E04](paquets/U3-06.md) | 90 | Aplicar canvi d’estat sense reinici indegut | U3-06.E03 |
| [U3-06.E05](paquets/U3-06.md) | 60 | Construir inputs d’un actor de producte | U3-06.E04 |
| [U3-06.E06](paquets/U3-06.md) | 90 | Connectar la màquina al cursor de clip | U3-06.E05 |
| [U3-06.E07](paquets/U3-06.md) | 60 | Verificar idle/caminar/acció | U3-06.E06 |
| [DEC-06.E01](paquets/DEC-06.md) | 60 | Auditar el playtest actual | PREP.E10 |
| [DEC-06.E02](paquets/DEC-06.md) | 90 | Comparar dues opcions de host | DEC-06.E01 |
| [DEC-06.E03](paquets/DEC-06.md) | 30 | Registrar resolució i límit d’implementació | DEC-06.E02 |
| [F-01.E01](paquets/F-01.md) | 60 | Congelar contracte host i documents bruts | PREP.E10 |
| [F-01.E02](paquets/F-01.md) | 90 | Preparar una petició de prova | F-01.E01 |
| [F-01.E03](paquets/F-01.md) | 90 | Crear el paquet temporal de prova | F-01.E02 |
| [F-01.E04](paquets/F-01.md) | 90 | Llançar amb arguments sense shell | F-01.E03 |
| [F-01.E05](paquets/F-01.md) | 60 | Llegir estat i final del playtest | F-01.E04 |
| [F-01.E06](paquets/F-01.md) | 60 | Aturar només la prova activa | F-01.E05 |
| [F-01.E07](paquets/F-01.md) | 90 | Restaurar la vista d’autor | F-01.E06 |
| [F-01.E08](paquets/F-01.md) | 90 | Connectar Play/Stop a motor_editor | F-01.E07 |
| [F-01.E09](paquets/F-01.md) | 90 | Verificar crear → provar → tornar | F-01.E08 |
| [F-02.E01](paquets/F-02.md) | 45 | Auditar entrada i selecció existents | PREP.E10 |
| [F-02.E02](paquets/F-02.md) | 60 | Validar casella seleccionada | F-02.E01 |
| [F-02.E03](paquets/F-02.md) | 60 | Afegir origen temporal al playtest | F-02.E02 |
| [F-02.E04](paquets/F-02.md) | 90 | Interpretar origen al runtime | F-02.E03 |
| [F-02.E05](paquets/F-02.md) | 60 | Connectar acció Provar des d’aquí | F-02.E04 |
| [F-02.E06](paquets/F-02.md) | 60 | Verificar casella bona i bloquejada | F-02.E05 |
| [F-03.E01](paquets/F-03.md) | 90 | Classificar què es pot recarregar | PREP.E10 |
| [F-03.E02](paquets/F-03.md) | 90 | Detectar canvis estables | F-03.E01 |
| [F-03.E03](paquets/F-03.md) | 90 | Preparar versió nova fora de l’estat viu | F-03.E02 |
| [F-03.E04](paquets/F-03.md) | 90 | Comprovar dependències afectades | F-03.E03 |
| [F-03.E05](paquets/F-03.md) | 90 | Publicar al límit de frame | F-03.E04 |
| [F-03.E06](paquets/F-03.md) | 90 | Migrar o reiniciar estat dependent | F-03.E05 |
| [F-03.E07](paquets/F-03.md) | 60 | Mostrar resultat al creador | F-03.E06 |
| [F-03.E08](paquets/F-03.md) | 90 | Connectar watcher al playtest | F-03.E07 |
| [F-03.E09](paquets/F-03.md) | 90 | Provar canvi vàlid i canvi trencat | F-03.E08 |
| [F-04.E01](paquets/F-04.md) | 60 | Inventariar logs i inspector existents | PREP.E10 |
| [F-04.E02](paquets/F-04.md) | 90 | Capturar un estat de joc llegible | F-04.E01 |
| [F-04.E03](paquets/F-04.md) | 60 | Filtrar registres acotats | F-04.E02 |
| [F-04.E04](paquets/F-04.md) | 60 | Resoldre selecció d’entitat per handle | F-04.E03 |
| [F-04.E05](paquets/F-04.md) | 60 | Projectar error a enllaç d’autor | F-04.E04 |
| [F-04.E06](paquets/F-04.md) | 90 | Dibuixar consola i inspector | F-04.E05 |
| [F-04.E07](paquets/F-04.md) | 90 | Connectar runtime de playtest al panell | F-04.E06 |
| [F-04.E08](paquets/F-04.md) | 60 | Verificar error rastrejable al creador | F-04.E07 |
| [U4-01.E01](paquets/U4-01.md) | 60 | Auditar càmera i escala isomètrica | PREP.E10 |
| [U4-01.E02](paquets/U4-01.md) | 90 | Calcular seguiment amb zona morta | U4-01.E01 |
| [U4-01.E03](paquets/U4-01.md) | 90 | Limitar càmera a mapa real | U4-01.E02 |
| [U4-01.E04](paquets/U4-01.md) | 90 | Calcular sacsejada determinista | U4-01.E03 |
| [U4-01.E05](paquets/U4-01.md) | 60 | Composar base i efectes de càmera | U4-01.E04 |
| [U4-01.E06](paquets/U4-01.md) | 90 | Connectar tween i càmera existent | U4-01.E05 |
| [U4-01.E07](paquets/U4-01.md) | 60 | Verificar cantonada i zona morta | U4-01.E06 |
| [U4-02.E01](paquets/U4-02.md) | 90 | Auditar cinemàtiques actuals abans d’estendre | PREP.E10 |
| [U4-02.E02](paquets/U4-02.md) | 90 | Llegir una línia de temps | U4-02.E01 |
| [U4-02.E03](paquets/U4-02.md) | 90 | Determinar clips actius al temps t | U4-02.E02 |
| [U4-02.E04](paquets/U4-02.md) | 90 | Avançar pistes en un sol rellotge | U4-02.E03 |
| [U4-02.E05](paquets/U4-02.md) | 90 | Arbitrar pistes sobre el mateix canal | U4-02.E04 |
| [U4-02.E06](paquets/U4-02.md) | 90 | Executar accions pels ports existents | U4-02.E05 |
| [U4-02.E07](paquets/U4-02.md) | 90 | Finalitzar i tornar control al jugador | U4-02.E06 |
| [U4-02.E08](paquets/U4-02.md) | 90 | Connectar una escena paral·lela de producte | U4-02.E07 |
| [U4-02.E09](paquets/U4-02.md) | 90 | Verificar independència dels FPS | U4-02.E08 |
| [U4-03.E01](paquets/U4-03.md) | 60 | Auditar document i lloc del panell | PREP.E10 |
| [U4-03.E02](paquets/U4-03.md) | 60 | Projectar temps i píxels | U4-03.E01 |
| [U4-03.E03](paquets/U4-03.md) | 90 | Inserir un clip validat | U4-03.E02 |
| [U4-03.E04](paquets/U4-03.md) | 90 | Moure un clip amb snap | U4-03.E03 |
| [U4-03.E05](paquets/U4-03.md) | 90 | Redimensionar durada | U4-03.E04 |
| [U4-03.E06](paquets/U4-03.md) | 60 | Reordenar pistes sense canviar IDs | U4-03.E05 |
| [U4-03.E07](paquets/U4-03.md) | 90 | Previsualitzar un temps sense efectes irreversibles | U4-03.E06 |
| [U4-03.E08](paquets/U4-03.md) | 90 | Dibuixar pistes amb focus i escala | U4-03.E07 |
| [U4-03.E09](paquets/U4-03.md) | 90 | Aplicar canvi i guardar/reobrir | U4-03.E08 |
| [U4-03.E10](paquets/U4-03.md) | 90 | Connectar amb l’editor de comandes | U4-03.E09 |
| [U4-03.E11](paquets/U4-03.md) | 90 | Verificar escena autorada sencera | U4-03.E10 |
| [U4-04.E01](paquets/U4-04.md) | 60 | Auditar efectes de Noqueo i escenes saltables | PREP.E10 |
| [U4-04.E02](paquets/U4-04.md) | 90 | Construir pla de salt | U4-04.E01 |
| [U4-04.E03](paquets/U4-04.md) | 90 | Aplicar final segur | U4-04.E02 |
| [U4-04.E04](paquets/U4-04.md) | 90 | Convertir EscenaNoqueo a dades | U4-04.E03 |
| [U4-04.E05](paquets/U4-04.md) | 90 | Connectar acció de salt i provar-la | U4-04.E04 |
| [U4-05.E01](paquets/U4-05.md) | 60 | Auditar objectes agrupats i MAP-08 | PREP.E10 |
| [U4-05.E02](paquets/U4-05.md) | 90 | Capturar un grup com a prefab | U4-05.E01 |
| [U4-05.E03](paquets/U4-05.md) | 90 | Validar recursos i espai del prefab | U4-05.E02 |
| [U4-05.E04](paquets/U4-05.md) | 90 | Instanciar amb identitats noves | U4-05.E03 |
| [U4-05.E05](paquets/U4-05.md) | 60 | Serialitzar definició i referències | U4-05.E04 |
| [U4-05.E06](paquets/U4-05.md) | 90 | Aplicar grup amb una operació d’undo | U4-05.E05 |
| [U4-05.E07](paquets/U4-05.md) | 60 | Definir vincle i overrides | U4-05.E06 |
| [U4-05.E08](paquets/U4-05.md) | 90 | Connectar biblioteca de grups al panell | U4-05.E07 |
| [U4-05.E09](paquets/U4-05.md) | 90 | Verificar dues còpies i desfer | U4-05.E08 |
| [U4-06.E01](paquets/U4-06.md) | 60 | Auditar formes i overlays existents | PREP.E10 |
| [U4-06.E02](paquets/U4-06.md) | 60 | Projectar col·lisions sense recomputar regles | U4-06.E01 |
| [U4-06.E03](paquets/U4-06.md) | 60 | Projectar regions i IDs | U4-06.E02 |
| [U4-06.E04](paquets/U4-06.md) | 60 | Projectar rutes i casella bloquejada | U4-06.E03 |
| [U4-06.E05](paquets/U4-06.md) | 90 | Connectar controls i render debug | U4-06.E04 |
| [U4-06.E06](paquets/U4-06.md) | 60 | Cotejar dibuix i comportament | U4-06.E05 |
| [U4-07.E01](paquets/U4-07.md) | 60 | Auditar partícules i helpers ja existents | PREP.E10 |
| [U4-07.E02](paquets/U4-07.md) | 90 | Llegir paràmetres de l’emissor | U4-07.E01 |
| [U4-07.E03](paquets/U4-07.md) | 60 | Calcular emissions amb residu | U4-07.E02 |
| [U4-07.E04](paquets/U4-07.md) | 60 | Crear partícula amb llavor explícita | U4-07.E03 |
| [U4-07.E05](paquets/U4-07.md) | 90 | Avançar estat i caducar partícules | U4-07.E04 |
| [U4-07.E06](paquets/U4-07.md) | 60 | Projectar estat a instàncies visuals | U4-07.E05 |
| [U4-07.E07](paquets/U4-07.md) | 90 | Connectar emissor a una escena real | U4-07.E06 |
| [U4-07.E08](paquets/U4-07.md) | 60 | Mesurar límit i repetibilitat | U4-07.E07 |
