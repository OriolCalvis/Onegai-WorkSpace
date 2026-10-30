# Codex 21/10 — ESCENAS-04 sobre la integración

Base 2bdbdfe, rama agente/codex/ESCENAS-04. Sermón con Edul/Verina forzados, captores presentes y gancho de cripta; campanas y ejército localizados; ocho variantes de combate exigen su mapa y cinemática. Rutas por zonas físicas; norte deja de saltar al exterior.

Base Militar provisional: no existe base_militar en la rama recibida; se conserva interior_cuartel, pendiente de respuesta de Oriol y de adaptación visual al patio/puertas cuando exista mapa. No se declara esa parte visual cerrada. No se cambia la geometría ni la salida única del Oeste.

Contenido 0 errores, 48 cinemáticas sin errores, 323 PNJ/464 aspectos; 300 partidas semilla7 con 0 atascos. Norte/quedarse se cubren dirigidamente, pues el simulador elige la primera zona. Pruebas dentro/fuera y con/sin señal de ocho variantes, cripta, negativo del validador e idempotencia OK. El validador ahora reconoce resultados de combate como productores; analisis_trama mantiene solo dos excepciones previas. Sin C++/Gorran/push/merge.

Trabajo ajeno preservado en árbol principal: ShaderManager.cpp, .obsidian/.qwen, bandeja, guía, tablero y coordinación previos. No se cambia ni incorpora.
