# Los 63 kits del cofre de El Vacío

UI-BG3-01 · 05/10/2026. Salen de una regla con parámetros (`assets/rules/kit_basico.json`, código en `Game/Personajes/KitBasico`), no de una lista a mano: el arma de la clase y piezas sin requisitos de tier 1 y rareza hasta *uncommon* que suben al menos +1 su atributo principal y el secundario, en 3 piezas como mucho. Para cambiar un kit se cambia la regla (o el equipo inicial de la clase en `classes.json`) y esta tabla se regenera con `KIT_LISTADO=1 ./test_kit_basico`.

| Clase | Principal / secundario | Kit |
|---|---|---|
| Acechador de Tejados | DES / INT | Arco Cazador de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Grebas Rúnicas Menores (piernas, +1 INT) |
| Ancla del Vacío | INT / CON | Cetro de Bronce de Milicia (arma, +1 CAR) · Tunica del Conjurador (torso, +1 INT) · Casco de Abordaje (cabeza, +1 CON) |
| Astrónomo Errante | INT / CON | Orbe de Mano de Milicia (arma, +1 INT) · Casco de Abordaje (cabeza, +1 CON) |
| Bardo de la Cuerda Rota | CAR / DES | Espada Corta (arma, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Bardo de la Voz Divina | CAR / DES | Espada Corta (arma, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Bardo de las Cuatro Estaciones | CAR / DES | Sable Ligero de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Caballero Templario | CAR / DES | Espada Corta (arma, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Cambiaformas | DES / CON | Lanza Corta de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Casco de Abordaje (cabeza, +1 CON) |
| Cantor de la Tierra | CON / INT | Bastón Ferrado de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Túnicas Ligeras (torso, +1 INT) |
| Carcelero de Hierro | CON / INT | Látigo de Cuero de Milicia (arma) · Zuecos del Alquimista (pies, +1 CON, +1 INT) |
| Cazarrecompensas de Bastrea | DES / INT | Ballesta Ligera de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Grebas Rúnicas Menores (piernas, +1 INT) |
| Centinela del Alba | CON / DES | Lanza Corta de Viaje (arma) · Perneras del Vadeador (piernas, +1 CON, +1 DES) |
| Arcanista | INT / DES | Foco Arcano (arma, +1 INT) · Armadura de Cuero (torso, +1 DES) |
| Voz del Alba | CAR / CON | Símbolo Sagrado (arma, +1 CAR) · Botas Claveteadas (pies, +1 CON) |
| Guardián de Hierro | CON / CAR | Espada Corta (arma, +1 DES) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Sombra del Camino | DES / INT | Dagas Gemelas (arma, +1 DES) · Túnicas Ligeras (torso, +1 INT) |
| Sable Errante | DES / CON | Espada Larga (arma) · Perneras del Vadeador (piernas, +1 CON, +1 DES) |
| Cocinero de Campaña | CON / INT | Maza de Viaje (arma) · Botas Claveteadas (pies, +1 CON) · Túnicas Ligeras (torso, +1 INT) |
| Coloso de Ceniza | CON / CAR | Martillo de Guerra de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Cronista de Batallas | INT / CAR | Daga de Viaje (arma) · Amuleto de Foco (cabeza, +1 CAR) · Túnicas Ligeras (torso, +1 INT) |
| Danzarín del Filo | DES / CAR | Sable Ligero de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Devoto del Bosque | INT / CAR | Martillo de Guerra de Milicia (arma) · Túnicas Ligeras (torso, +1 INT) · Amuleto de Foco (cabeza, +1 CAR) |
| El Apóstata | CAR / CON | Sable Ligero de Milicia (arma) · Amuleto de Foco (cabeza, +1 CAR) · Botas Claveteadas (pies, +1 CON) |
| Forjacantos | CON / INT | Martillo de Guerra de Viaje (arma) · Zuecos del Alquimista (pies, +1 CON, +1 INT) |
| Francotirador | INT / DES | Arco Cazador de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Grebas Rúnicas Menores (piernas, +1 INT) |
| Furia de los Creadores | CAR / CON | Maza de Viaje (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Furia Salvaje | CON / CAR | Gran Hacha de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Guardaespaldas | CON / DES | Espada Corta (arma, +1 DES) · Botas Claveteadas (pies, +1 CON) |
| Guardián de la Presa | CON / INT | Lanza Corta de Milicia (arma) · Zuecos del Alquimista (pies, +1 CON, +1 INT) |
| Guerrero del Puño Cortante | CON / DES | Sable Ligero de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Armadura de Cuero (torso, +1 DES) |
| Hermano de Sangre | CAR / CON | Escudo de Madera de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Hoja del Monzón | DES / CON | Daga de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Casco de Abordaje (cabeza, +1 CON) |
| Hoja Venenosa | DES / CON | Dagas Gemelas (arma, +1 DES) · Casco de Abordaje (cabeza, +1 CON) |
| Inventor | INT / DES | Arco Cazador de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Grebas Rúnicas Menores (piernas, +1 INT) |
| Jinete de Tormentas | DES / CAR | Lanza Corta de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Ladrón de Rostros | CAR / DES | Estilete de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Lancero del Vendaval | DES / CON | Pica de Guardia de Viaje (arma) · Grebas de Cuero (piernas, +1 DES) · Botas Claveteadas (pies, +1 CON) |
| Maestro de Ceremonias | CAR / INT | Cetro de Bronce de Milicia (arma, +1 CAR) · Tunica del Conjurador (torso, +1 INT) |
| Mago Chasqueador | INT / DES | Antifaz de la Vigilia (cabeza, +1 DES, +1 INT) |
| Mago del Paso Ligero | INT / DES | Estilete de Milicia (arma) · Faldar de Explorador (piernas, +1 DES) · Túnicas Ligeras (torso, +1 INT) |
| Médium de la Tomba | CAR / INT | Cetro de Bronce de Viaje (arma, +1 CAR) · Tunica del Conjurador (torso, +1 INT) |
| Monje del Despertar | DES / CON | Perneras del Vadeador (piernas, +1 CON, +1 DES) |
| Muralla de Udrax | CON / CAR | Maza de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Nacido del Trueno | CAR / DES | Jabalinas de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Peregrino de los Doce Meses | CAR / CON | Bastón Ferrado de Viaje (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Portador de Plagas | INT / CON | Daga de Viaje (arma) · Zuecos del Alquimista (pies, +1 CON, +1 INT) |
| Portador del Farol | CAR / CON | Maza de Milicia (arma) · Amuleto de Foco (cabeza, +1 CAR) · Botas Claveteadas (pies, +1 CON) |
| Portaestandarte | CAR / CON | Jabalinas de Milicia (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
| Quebraguardia | DES / CON | Hacha de Una Mano de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Botas Claveteadas (pies, +1 CON) |
| Quemador de Reliquias | INT / CAR | Vara de Aprendiz de Viaje (arma, +1 INT) · Amuleto de Foco (cabeza, +1 CAR) |
| Rastreador de Ecos | INT / DES | Arco Cazador de Milicia (arma) · Armadura de Cuero (torso, +1 DES) · Grebas Rúnicas Menores (piernas, +1 INT) |
| Redimido | CAR / DES | Daga de Milicia (arma) · Máscara del Duelista (cabeza, +1 CAR, +1 DES) |
| Renacido | CAR / CON | Cetro de Bronce de Milicia (arma, +1 CAR) · Botas Claveteadas (pies, +1 CON) |
| Rompemareas | CON / DES | Hacha de Mano de Viaje (arma) · Perneras del Vadeador (piernas, +1 CON, +1 DES) |
| Saltador de Planos | DES / CAR | Estilete de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Amuleto de Foco (cabeza, +1 CAR) |
| Sanadora de Esporas | INT / CON | Bastón Ferrado de Viaje (arma) · Zuecos del Alquimista (pies, +1 CON, +1 INT) |
| Señor de las Armas | INT / CON | Orbe de Mano de Viaje (arma, +1 INT) · Casco de Abordaje (cabeza, +1 CON) |
| Señor de los Astros | INT / CAR | Vara de Aprendiz de Viaje (arma, +1 INT) · Amuleto de Foco (cabeza, +1 CAR) |
| Sombra de la Duna | DES / CON | Cuchillo de Cazador de Viaje (arma) · Armadura de Cuero (torso, +1 DES) · Casco de Abordaje (cabeza, +1 CON) |
| Tejedora de Mareas | INT / DES | Vara de Aprendiz de Milicia (arma, +1 INT) · Botas de Caminante (pies, +1 DES) |
| Titiritero Maldito | CAR / CON | Látigo de Cuero de Milicia (arma) · Amuleto de Foco (cabeza, +1 CAR) · Gambesón del Veterano (torso, +1 CON) |
| Usurero de Almas | CAR / CON | Látigo de Cuero de Viaje (arma) · Amuleto de Foco (cabeza, +1 CAR) · Gambesón del Veterano (torso, +1 CON) |
| Verdugo del Silencio | CON / CAR | Gran Hacha de Viaje (arma) · Botas Claveteadas (pies, +1 CON) · Amuleto de Foco (cabeza, +1 CAR) |
