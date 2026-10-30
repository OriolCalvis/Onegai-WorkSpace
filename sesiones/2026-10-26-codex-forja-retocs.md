# Recepción de retocos de Forja — 26/10/2026

Oriol entrega Downloads/forja_onegai_202610261359.json con cambios estéticos. Base de trabajo: codex/revision-reciente-20261026, 28c511d. Sin cambios ajenos ni modificaciones del juego.

Original preservado íntegro en MotorGraphico-main/MotorGraphico/tools/forja_importaciones/, con informe y SHA-256. Validados 12 entradas, 171 fotogramas y 15527 píxeles RGBA; índices de píxel y frame correctos.

Armas, ropa, fichas y animaciones coinciden exactamente con los ajustes actuales. Lo nuevo reside en diagonales y creador, bloques que importar_forja.py ignora. Incluye capas cuerpo, cola, pelo, alas, cuernos, complementos, hombre y torso; __partes mueve cola o tronco. El taller incluye retoques de prendas/tocados y dos definiciones de armas nuevas. Se conserva todo, sin reinterpretar los datos de estadísticas como un encargo de modificar reglamento.

Falta identificar la base de coordenadas de la Forja. exportar_forja_piezas.py (nuevo e2224ed, inspeccionado sin fusionarlo) documenta que los cuerpos de la Forja y el generador tienen orígenes distintos; además hemos escalado y separado la familia minotauro. Aplicación directa de los píxeles incorrecta. Se pregunta a Oriol por la versión de la Forja; para transformar con fidelidad se necesita su diagonales.json base o el artefacto que lo contiene. No se ha aplicado ni anunciado una integración inexistente.

Fila FORJA-IMPORT-26 en TABLERO: pendiente de la base visual. Las pruebas de esta recepción son validación de JSON y comparación de ajustes, no pruebas de una importación al motor.
