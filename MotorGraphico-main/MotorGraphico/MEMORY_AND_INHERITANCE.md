# Memoria, pools y herencia

**Estado:** aceptado. Aplicable a todo código nuevo y a cada refactor que
toque propiedad, entidades, render o contenido dinámico.

## Decisión

El motor usa RAII y propiedad explícita. `std::vector<T>` es la opción por
defecto para datos compactos; `std::unique_ptr<T>` se usa cuando hay
propiedad exclusiva y estabilidad de dirección; `HandlePool<T>` se usa
para objetos dinámicos que se crean y destruyen con frecuencia. La
herencia sólo expresa contratos o implementación verdaderamente común;
el comportamiento variable se resuelve por composición.

No se añade un allocator global ni un pool por clase sin medir: aumentan
la complejidad, fragmentan la propiedad y hacen más difíciles los tests.

## Reglas obligatorias

1. Todo recurso tiene un único propietario. Usa valor si el objeto cabe
   naturalmente en su dueño, `unique_ptr` si su vida es opcional o su
   dirección debe ser estable. `shared_ptr` exige justificar en el
   comentario qué propietarios comparten realmente el ciclo de vida.
2. No se usa `new`/`delete` en lógica de juego. Sólo se permiten dentro
   de wrappers RAII de APIs C, C++ o OpenGL, y el wrapper debe ser no
   copiable o tener una semántica de copia explícita.
3. Un puntero o referencia no posee. Su contrato debe indicar quién vive
   más tiempo. Nunca se guarda como identidad de una entidad dinámica.
4. `vector<T>` sirve para recorrido lineal y datos estables. Se hace
   `reserve()` cuando el máximo esperado es conocido.
5. `HandlePool<T>` se usa para proyectiles, partículas, encuentros,
   comandos o entidades transitorias. El handle generacional es la única
   identidad persistente: no índices, no punteros. Un `get()` puede
   devolver `nullptr`; debe comprobarse.
6. Los pools no se emplean para recursos OpenGL, catálogos inmutables ni
   objetos únicos de la aplicación. Esos ya tienen un ciclo de vida más
   claro con wrappers RAII, mapas o `unique_ptr`.
7. El pool se dimensiona con `reserve()` al cargar el nivel. Si el perfil
   muestra realocaciones o presión de memoria, se mide antes de cambiar
   de estructura.
8. Las interfaces tienen destructor virtual por defecto y sólo métodos
   de contrato. Una clase tiene como máximo una base concreta; la
   herencia múltiple es sólo de interfaces `I...`.
9. Las bases concretas comparten estado y comportamiento real. Si sólo
   se quiere reutilizar una capacidad (IA, inventario, animación,
   estadísticas), se compone un objeto y se delega.
10. Toda jerarquía nueva requiere una prueba de destrucción polimórfica,
    y todo pool una prueba de reutilización e invalidación de handles.

## Tabla de decisión

| Necesidad | Elección |
|---|---|
| Valor pequeño sin identidad | `T` / `vector<T>` |
| Recurso único o objeto polimórfico estable | `unique_ptr<T>` |
| Objeto dinámico, alta rotación, identidad segura | `HandlePool<T>::Handle` |
| Catálogo por identificador textual | `unordered_map<string, T>` o `unique_ptr<T>` |
| Propiedad compartida real | `shared_ptr<T>` con justificación |

## Checklist de revisión

- ¿Quién destruye el objeto y en qué momento?
- ¿Puede quedar un puntero, referencia o índice colgando?
- ¿La frecuencia de altas/bajas justifica un pool?
- ¿Se recorren los datos de forma lineal y compacta?
- ¿La herencia expresa un contrato existente o debería ser composición?
- ¿El test cubre borrado, reutilización e intento de usar un handle viejo?

## Aplicación actual

El motor ya usa RAII para texturas, shaders y FBOs; `unique_ptr` para la
propiedad de UI, entidades y sesiones; y contratos `I...` para render,
actualización, combate, HUD y catálogos. `HandlePool<T>` queda disponible
para los subsistemas de alta rotación que se incorporen a partir de ahora.
