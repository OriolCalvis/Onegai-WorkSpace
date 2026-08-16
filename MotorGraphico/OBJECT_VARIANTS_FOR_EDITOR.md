# Objetos, variantes y propiedades editables

## Objetivo

El catálogo define el arquetipo reutilizable y el nivel guarda la variante
colocada. Una poción base puede convertirse en curativa, de maná, veneno o
misión sin crear cuatro objetos casi idénticos.

## Propiedades por instancia

| Propiedad | Ejemplo | Uso |
| --- | --- | --- |
| displayName | Poción de Ben Kafka | Nombre narrativo local. |
| scale | 0.75, 1.0, 2.0 | Tamaño de props, criaturas o decoración. |
| variant | curativa, mana, veneno | Configuración del arquetipo. |
| effectOverride | curar 12 PV | Ajuste de efecto sin duplicar la ficha base. |
| properties | rareza, facción, misión | Datos ampliables para narrativa. |

El catálogo conserva los valores por defecto. La instancia solo guarda lo
que cambia, de modo que un ajuste de balance afecta a todos los objetos que
no tengan un override.

## Disponible ahora

El formato de nivel y `EditorState` ya conservan `displayName`, `variant`,
`scale`, `effectOverride` y `properties` por instancia. Copiar, pegar,
deshacer y guardar no descartan esos campos. En el editor visual, `H` sobre
un objeto alterna una escala de 50%, 75%, 100%, 150% y 200%; es un override
local y no modifica el arquetipo del catálogo.

Los demás campos ya pueden venir de niveles creados por herramientas o
scripts. El siguiente paso de UI es exponer sus controles de texto y listas
por categoría en el inspector.

## Ficha visual siguiente

Al seleccionar un objeto colocado, el inspector debe mostrar:

1. Icono y nombre humano.
2. Tamaño pequeño, normal, grande o valor numérico.
3. Variantes válidas del arquetipo.
4. Solo las propiedades pertinentes:
   - Pociones: tipo, potencia y duración.
   - Enemigos/animales: vida, agresividad, patrulla y botín.
   - Equipo: daño o defensa, rareza y ranura.
   - Props: sólido, interactuable y conexión de nivel.
5. Acción para restaurar los valores del arquetipo.
