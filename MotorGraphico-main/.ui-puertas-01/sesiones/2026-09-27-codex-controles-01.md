# CONTROLES-01 — 26/27 de septiembre de 2026

Commit MotorGraphico: 0e2b2b4. Encargo: mejorar game feel y controles.

WASD mantenido con prioridad reciente, retardo inicial 240 ms y cadencia 140 ms; interpolación visual conservando grid autoritativo. Bloqueo tras modal/foco/carga hasta soltar. TAB dejaba de abrir por doble alternancia y Cancel; corregido. Escape contextual. SessionCommandHost integrado en motor_level; test comparte librería.

Validado: 50 pruebas Player; 4/4 baterías CTest (Player, populate, inventario, host); juego compila y renderiza ciudad_oeste tres frames con salida 0. Guardia rápida y diff-check verdes. Guardia completa bloqueada por deuda ajena Application.h 107/104. Compilación global no afirmada (E09Bridge y PlayerPathLog ajenos). Mapa G-12 actualizado. Índice aislado preservó WIP concurrente.
