# 15/10/2026 — Codex, AV16-08: preparación de los sitios del sur

Oriol pide «CONTINUA PORFA». Se vuelve a comprobar el prerrequisito: HEAD sigue en 11ff327 del 08/10. Se prepara trabajo independiente y se pide autorización expresa para guardar los paquetes ajenos en una base aislada; esa pregunta sigue pendiente. No hay ramas nuevas ni escrituras git.

## Inventario y propuesta

Los 17 lugares tienen 22 puertas en las ciudades declaradas por el manifiesto de Boundington. Hay cinco parejas con el mismo destino: banco, baños, mercado, posada y sastrería. Ciudad_este y ciudad_oeste no añaden duplicados hacia esos locales. PO-48 propone cinco nombres, emblemas y dependientes propios, con copias de interiores; espera la respuesta. No se han cambiado nombres ni niveles.

Validador preparado en un paquete de revisión: `MotorGraphico-main/MotorGraphico/out/codex_cola_15-10/AV16-08-preparacion.tgz`. Lee las ciudades del manifiesto, detecta nombres normalizados repetidos, rutas que salen del proyecto, niveles ausentes, puertas sin ficha y varios accesos al mismo local.

## Verificación

`python3 tools/test_validar_lugares.py -v`: 14/14. El intento inicial con unittest discover importaba también interiors_kit y pedía NumPy; se ejecutó directamente el módulo solicitado. Sobre los datos actuales: 22 puertas y cinco errores esperados de local repetido. No se ha escondido ese resultado ni se ha integrado una guarda roja en validar_todo.

Se extrajo HEAD en `/private/tmp/codex-cola-15-10-preparacion` y se superpusieron los 377 cambios seleccionados del motor. Copia de preparación, no una rama ni un commit. CMake real vive en MotorGraphico/, no en la raíz del repositorio. La configuración inicial no pudo descargar glm por la red; se reutilizaron las fuentes de glm y glfw ya disponibles en build-release/_deps mediante FETCHCONTENT_SOURCE_DIR_GLM y FETCHCONTENT_SOURCE_DIR_GLFW. La compilación Release completa pasa.

CTest GL-free inicial: 227/230. Dos fallos eran falta de NumPy en Python 3.14; con el Python 3.12 del runtime y NumPy, validar_todo da 0 errores/22 avisos y los dos tests de contenido pasan al repetirlos. Queda linea_base_core, con 44 deudas detectadas en la base candidata. No se retoca la línea base. Los logs se conservan en out/codex_cola_15-10/.

## Pendiente

Autorización de la base aislada; nombres de PO-48; interiores y catálogo; enlaces del carril de Qwen. No hay captura de producto cambiado porque aún no hay cambios visuales aplicados. No se marca la tarea Hecho y no se commitea trabajo ajeno sin autorización.
