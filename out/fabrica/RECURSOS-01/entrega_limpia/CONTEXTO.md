# Entrega limpia RECURSOS-01

Commit c9b6b81. Clon compartido creado con git-safe, archivos versionados limpios
antes/después de verificar. Se recompilaron editor y cinco objetivos puros desde
las fuentes del commit. Solo se reutilizaron las fuentes de glm 1.0.1 y GLFW 3.4;
no se copiaron objetos/binarios. Build: 41,4 s. Siete objetivos CTest pasan,
incluidas dos pruebas reales del modal de referencias a escalas 1/2 (5,3 s).
Diez regresiones de paridad pasan. Fábrica VERDE para fabrica_recursos.json.
Se usó entregar --sin-clon porque el clon ya se había preparado con git-safe;
la prueba no se hizo sobre el árbol de trabajo sucio. El alcance acotado no
sustituye la comparación general: cinco checks y dieciséis tests heredados,
ninguno nuevo. Revisión visual y capturas: ../visual/.

Prueba para Oriol: Proyecto → abrir mapa → Referencias → consultar/cerrar;
Quitar → revisar/cancelar; en copia desechable cambiar una ruta y Actualizar
revisión. No se ha atribuido aceptación manual ni ejecución Windows.
