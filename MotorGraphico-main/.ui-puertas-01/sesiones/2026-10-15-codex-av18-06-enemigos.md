# Codex — AV18-06: cifras y repertorios (15/10/2026)

Rama agente/codex/AV18-06 sobre f0a91b7. Fila reclamada antes de crear ficheros. No se toca el C++ de combate ni Application.

Se añaden 900 cifras nativas en 432 monstruos: 466 positivas y 434 ceros de control. La cifra llega a magnitudeHint del cargador. DEUDA §7 no trae una fórmula de daño, dice que no existe: se registra la discrepancia y se ofrece referencia editorial 2*tier + rango (0/2/4). No se finge que sea la fórmula de vida ni que ya se aplique al daño. Se revisaron las excepciones de la matriarca de la sal, el portero y la matriarca del pozo para no dar daño a controles ni quitarlo al golpe Coral rompehuesos. El texto y los bonos condicionales quedan intactos. El daño del runtime sigue DANO-02.

31 cartas en formato nativo: dos por repertorio, tres para Gorran/Sandes/Edul. Nueve actores salen realmente en las referencias de Boundington (objetos con fallback, ninguno corresponde a las 432 fichas). Sus combat.skills aprenden los repertorios. Cinco plantillas más (dos lobos, rata, naga y acólito) quedan preparadas en skillIds; no se inventan colocaciones y la naga conserva tier 3. Solo estados existentes; los cuatro nuevos quedan marcados pendientes.

Comprobado contra HEAD: mismas 432 vidas/atributos/defensas y mismo texto de todos los ataques; mismos nueve maxHealth/maxMana/stats de campaña. Generador idempotente por huella de cuatro ficheros. Diez pruebas Python de fórmulas/cobertura y tres C++ de cargador, puente de creación y ganchos de las 31 cartas. Validador propio integrado en validar_todo.

Bloqueo funcional: CartasEnCombate::resolverEnemigo solo consulta habilidadesLegado, que Application rellena con tres habilidades antiguas. Las cartas nativas no se ejecutan por añadir skillIds. Además se elige siempre la primera pagable, por lo que una conversión sin selección tampoco habilitaría las tres. Se documenta el punto preciso y la aceptación que falta en FORMATO_ACCIONES_MONSTRUOS.md. Corresponde a Claude. No se declara AV18-06 hecha ni se fabrica una captura de acciones que el juego aún no usa.

Verificación final: Release completa y gl-free 235/237; fallan únicamente las dos guardias heredadas, con 49 instantáneos y 44 deudas previas. La copia limpia del árbol preparado en /private/tmp/codex-av1806-limpio compila y pasa los dos objetivos propios (tres C++ + diez Python); validar_todo 0 errores/22 avisos. Los ficheros de cruce quedan en 499/497/494 líneas, sin nueva cabecera de más de 15 miembros.

Revisión final de autoría de botín: una única garantizada fuera de su tabla no llevaría la marca unico del generador. Se añade rechazo al validador y una regresión (botín 15/15 también en copia limpia); el contrato exige que la garantía única figure en su propia tabla. No se cambia el generador de producto.

Trabajo ajeno observado y preservado en el árbol principal: ajustes de creación/ropa/Espectro de Claude, CapaDeDibujo/render, cambios de ficha de Qwen, .obsidian/workspace.json y .qwen/settings.json. No se incluyen en ninguna rama ni commit de Codex. No se hace push ni merge.
