# C-11 — integración aplicada en el workspace, pendiente de versionar la base

Este parche conserva **solo los cambios de esta sesión** sobre los trece archivos indicados. Ya están aplicados en el workspace del 03/09/2026; no volver a aplicarlos allí.

`src/Project/*`, `include/Project/*`, el serializador y el esquema existían como trabajo ajeno sin versionar. La regla 2 de la BITACORA impide incluir íntegra esa base en nuestro commit. Migrar los cinco manifiestos en Git sin incorporar también su lector dejaría HEAD incompatible, por lo que los cambios coordinados de consumidores, contrato y datos permanecen juntos en este parche y en el workspace. Las herramientas nuevas, las pruebas, su módulo CMake y el seguimiento sí se versionan directamente. El include del módulo queda en el parche para conservar también la preparación simultánea de C-22 en el índice compartido.

Cuando se versione la arquitectura previa, revisar el parche contra esa base. Si los cambios siguen aplicados, incorporar los archivos normales; no duplicar la aplicación. Sobre la base anterior exacta, desde la raíz del repositorio:

```bash
../herramientas/git-safe.sh apply --check MotorGraphico/planificacion/seguimiento/C-11-integracion.patch
../herramientas/git-safe.sh apply MotorGraphico/planificacion/seguimiento/C-11-integracion.patch
```

Antes de cerrar C-11, compilar `suite_gl_free`, ejecutar CTest y pasar CI. Si la base evolucionó, integrar los hunks manualmente; no forzar el parche. Retirar este archivo y el parche una vez incorporada la integración normal en Git. Las huellas permiten identificar exactamente el antes y el después sin atribuirse trabajo previo.

| Archivo | SHA-256 base | SHA-256 con C-11 |
|---|---|---|
| `src/Project/ProjectIndex.cpp` | `6ebd0946137b54007f6322bdb9a2462eff1a45334935c9bfcecc15e05a300003` | `3cb5842f58e0682bc9fcc099b3cbb3410e58ac1d1085485255bd5942a4b195d0` |
| `src/Project/ProjectSerializer.cpp` | `209e8b72c6da3b6ba58c9ed243b90859a25dd62e039c3caa1a94b246f8fd3536` | `db35aed11b68f2cfd114d43cf7c3d1e8dffc8dec8f9b9df1c4148d065f5e1af9` |
| `include/Project/ProjectIndex.h` | `d578d4f82921db2f6ee73a46721c195c29d996e58eb44e920aee03328f34ae9e` | `f08d9795dad819110425621662f7251e2e1784e357ca82d5ced0db9cd71d05fb` |
| `schemas/project-v1.schema.json` | `8bdf04267ef8ac0cfe550d6d574b7c8a39fe1c33c18df6cfc168ce2d54e4041e` | `a799766fcb88957c237b1664422c2673fe604d99b509ccb2d783ea730fed9ac6` |
| `FORMATO_PROYECTOS.md` | `b772f37c3de9e0184d9ec83099f69f1402b988d482b501684079e59c275aed6d` | `135f2778259e1f8411ae0b8f4b06fcbf18287e30084358997ad27326d7124e44` |
| `tools/build_proyecto.py` | `d3894ad5ed1f54ec19bcea3c3b5060c065653df2c9d210205dcd8645c75669b3` | `1aa5288a90211f0bfc78341a3880b4445f5c8d94729de1408227f635330ae6d9` |
| `tools/validar_todo.py` | `22f2f4021af570570a57874d67efc17f3b7d23ef6cdc37a5fbd0f1f22c3bd93a` | `a0874ea17b82d9fa12fe4c849aaeecaee298d80d8a38b186082e2031361b972d` |
| `assets/proyectos/boundington.json` | `c88e26898452c0f81719979756ef973813ddc1714633f2f1126c4249d6e31830` | `a54fc7d1dfcfff95910b0050c5a55d23c35f29dd987fd344faca20222eb756b6` |
| `assets/proyectos/este_norte.json` | `4409926c3ade1527d81446d70e26753ac98a27e62405395e5dcf1709a4c8fe3b` | `f9caa8a7694deb519cb96a88ef4a9fd107c2358b3f2eb1f4d033248acb403c77` |
| `assets/proyectos/este_sur.json` | `b0426450e478c44afb088d858b974279ca81075790857fe5ed52f6d99c62bc46` | `a685a03bdad50ff7293ff5fbc2a6930020243bd72ebc0aac9e0516f978e771ce` |
| `assets/proyectos/mundo.json` | `450902c9ee141e753a6d1239f6e60db4215eb5aac9ac1a9857922198656ee5c4` | `b94f9651c76f45e5e474e7fa34a5c7ec6cd91e2e63ddc15648fd7b7228aacde0` |
| `assets/proyectos/oeste_norte.json` | `c3f957788167f0eafb79d91ae941f6c0027ac41b1d45ee0150fc8c8362bf79cb` | `74d450ca8c8a3a477876c9a7e449d54e02249faab62684a2e0ccd127892e2d07` |
| `CMakeLists.txt` (solo el include C-11; base con C-22 concurrente) | `6bffab3bf96aab804093fd87b34ee1bbc0ef6c228df49a086447af74a4d5d678` | `9c758f5f56e377c4c77cd6ce1456bc467bbdf518d634cc509d6e1846338fb567` |
