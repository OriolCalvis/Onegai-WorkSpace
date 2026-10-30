# 2026-10-15 — claude integra (ronda 15/10)

Integré y probé pnj_datos01, pnj_motor01, mapa01, ciudad02 y oficios_datos01, en ese orden.
La VM se colgó: probé en el contenedor (Linux, Release). No hay compilación en el Mac.
Arreglos (con *.orig al lado): mapa01 ediciones.json (ancla tras pnj_motor) y mapa01.tgz (flecha sobre
retales apilados + test); ciudad02.tgz/postimagen.md5 (3 PNJ a elfos del bosque por validar_pnj);
aplicar.sh de pnj_datos01 y oficios_datos01 (exclusiones en validar_catalogos_rpg.py).
ctest: gl-free 228/230, GL 9/11; los fallos son del entorno o esperados.
Guardas OK. Capturas e informe en out/claude_integra01/. Script: out/aplicar_ronda_15-10.sh.
Pendiente: Release en el Mac, git add de nuevos, carriles misiones y puertas.
