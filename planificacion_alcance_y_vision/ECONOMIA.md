# Economía de los tres días de Boundington — AV18-01

Codex / ChatGPT, 15/10/2026. Diseño para AV18-02 a AV18-05. **No cambia precios ni declara implantadas las reglas que aún faltan.** Los perfiles son objetivos calculados: AV18-04 debe contrastarlos con tablas, niveles y límites reales.

## 1. Precios de partida comprobados

La moneda es el oro del grupo. `SessionData::m_gold` nace en 0, pero la partida jugable empieza con **150 de oro**: `Application.cpp` los entrega al iniciar y `ApplicationPantallas.cpp` al crear una partida nueva. Esa bolsa se cuenta una sola vez. El kit del prólogo es una dotación inicial garantizada, fuera del presupuesto de botín; no se concede otra vez en cada mapa.

`GameSession::basePriceOf` consulta primero el equipo RPG (`equipment.json`), incluidos sus objetos puente. Si no es equipo usa el precio del objeto de nivel. Pan, cerveza, daga y espada corta están en `assets/objects/ciudad_objetos.json`. `consumables.json` no trae precios: no se debe valorar todo consumible a cero por leer solo ese fichero.

| Objeto o servicio existente | Precio base (oro) | Fuente |
|---|---:|---|
| Pan | 8 | ciudad_objetos, pan; cura 5 PV |
| Cerveza | 6 | ciudad_objetos, cerveza; cura 3 PV |
| Poción | 30 | ciudad_objetos, pocion |
| Éter | 45 | ciudad_objetos, eter |
| Vendas | 18 | ciudad_objetos, vendas |
| Daga | 60 | ciudad_objetos, daga |
| Espada corta | 120 | ciudad_objetos, espada_corta |
| Escudo de madera | 90 | ciudad_objetos, escudo_madera |
| Capa de viajero | 70 | ciudad_objetos, capa_viajero |
| Botas de cuero | 55 | ciudad_objetos, botas_cuero |
| Anillo de plata | 200 | ciudad_objetos, anillo_plata |
| Gema azul | 350 | ciudad_objetos, gema_azul |
| Entrada de ópera | 40 | ciudad_objetos, entrada_opera |
| Estudiar en la biblioteca | 3 | oficios, estudiar_biblioteca |
| Bañarse en las termas | 6 | oficios, banarse_termas |
| Servir un turno con éxito | **+8** | oficios, servir_bar; roce +4, crítico +12, pifia −2 |

Los oficios dan oro, experiencia y algo de reputación según D0017. Trabajar consume una franja del día, como declara el catálogo; estudiar o bañarse pueden costar dinero y dar beneficios. No son fuentes gratuitas equivalentes a abrir un cofre.

El precio explícito de `shop.items` manda sobre el base al comprar. La facción del tendero aplica después `clamp(int(-0.75 * reputacion), -25, 50)` por ciento; `precioConAjuste` redondea al entero más próximo. La compra queda entre el 75 % y el 150 % del precio de la oferta. Un tendero sin facción no aplica ajuste. No se sustituyen estos límites por los multiplicadores del ejemplo anterior de PENDIENTE_DE_REVISAR.

## 2. Rareza, objetos y negocios

| Grupo | Precios existentes o propuestos | Uso en Boundington |
|---|---:|---|
| Mundano nuevo | 1–12 (propuesta para objetos nuevos) | platos, trapos, materiales pobres; venta 1–3 |
| Equipo común, common | 3–150 (129 fichas) | kit y mejoras de 60–120 |
| Poco común, uncommon | 175–250 (62 fichas) | mejora opcional, algunas únicas tácticas |
| Raro, rare | 550 (484 fichas) | premio excepcional; nunca en un barril |
| Épico, epic | 2.025 (150 fichas) | fuera de las tablas aleatorias de esta campaña |
| Mítico, mythic | 40.300 (35 fichas) | fuera del presupuesto de estos tres días |

AV18-03 añade doce únicas comunes de 40–60: su precio es nuevo, sin bonos de atributo y con una carta activa por combate. «Única» significa una sola copia, no rareza alta. Se baja el mínimo orientativo de escondite a 20 para que los secretos de ciudad quepan en su partida de 20. No cambian los presupuestos por día ni los márgenes.

No se cambia ningún precio existente. Una única se valora a su precio real y cuenta en el presupuesto; no se rebaja su precio para que quepa. La rareza no autoriza saltarse el Tier 1. Las llaves y pruebas de misión sin precio no se venden; se documenta su función, sin inventar oro que no producen.

**Venta de objetos: 25 % del precio base**, con división entera hacia abajo y mínimo 1 para un objeto vendible. La mala reputación puede reducir ese porcentaje según `recompraConAjuste`; la buena no lo sube. Comprar al mejor descuento (75 %) y vender al 25 % pierde dinero. **Hoy siguen existiendo recompras de 40, 50 y 70 % y un defecto de 50 %**: Claude debe aplicar la regla encargada y sus pruebas en la zona de comercio. El documento y el simulador no deben darla por implementada.

**Reventa de negocios: 50 %**, conforme a PO-42 y D0018. No se les aplica el 25 %. Los precios actuales de los negocios no cambian ni se garantiza comprarlos siguiendo solo la historia.

## 3. Unidad y contabilidad

`V = oro directo + suma(precio base × cantidad de cada objeto)`. Es valor de adquisición, no oro de venta. Una daga de 60 y diez monedas gastan V=70; vender la daga deja 25 monedas en total.

Los premios ya escritos en aventuras o encuentros se descuentan de la fila de misiones o combates: no se añade todo el presupuesto encima. Las ramas excluyentes se cuentan una vez por recorrido. Hoy el día 1 tiene 13 efectos grantGold (dos positivos de +10); el día 2 tiene seis (positivos +40 y +30); el Ocaso no tiene. No es correcto sumar todas las ramas del fichero como ingresos de un jugador. Las menciones de oro en diálogo no son pagos.

Ciudad agrega todos sus barrios. Interiores agrega todos sus locales: Qwen repartirá ese importe entre instancias concretas, sin repetir el presupuesto por cada edificio. El día 3 incluye el Ocaso, que no es un cuarto día. Las compras se hacen antes del cierre de tiendas que imponga la historia.

## 4. Presupuesto por día, mapa y origen (V)

| Día | Región | Cofres visibles | Combates | Misiones | Entorno | Tesoros ocultos | Total V |
|---|---|---:|---:|---:|---:|---:|---:|
| 1 | ciudad | 60 | 20 | 120 | 20 | 20 | 240 |
| 1 | interiores | 60 | 0 | 80 | 20 | 40 | 200 |
| 1 | cripta | 40 | 80 | 40 | 0 | 40 | 200 |
| 1 | afueras | 40 | 40 | 40 | 20 | 20 | 160 |
| 1 | cueva | 0 | 0 | 0 | 0 | 0 | 0 |
| 2 | ciudad | 40 | 20 | 80 | 0 | 20 | 160 |
| 2 | interiores | 80 | 0 | 100 | 20 | 40 | 240 |
| 2 | cripta | 80 | 140 | 100 | 0 | 80 | 400 |
| 2 | afueras | 60 | 60 | 60 | 20 | 40 | 240 |
| 2 | cueva | 120 | 180 | 140 | 20 | 100 | 560 |
| 3 | ciudad | 40 | 60 | 100 | 20 | 20 | 240 |
| 3 | interiores | 60 | 0 | 100 | 20 | 60 | 240 |
| 3 | cripta | 40 | 40 | 60 | 0 | 20 | 160 |
| 3 | afueras | 40 | 80 | 60 | 20 | 40 | 240 |
| 3 | cueva | 60 | 120 | 80 | 0 | 60 | 320 |
| **1** | **Total del día** | 200 | 140 | 280 | 60 | 120 | **800** |
| **2** | **Total del día** | 380 | 400 | 480 | 60 | 280 | **1600** |
| **3** | **Total del día** | 240 | 300 | 400 | 60 | 200 | **1200** |

**Total V=3.600**: día 1, 800; día 2, 1.600; día 3 más Ocaso, 1.200. El día 1 no presupone acceso a la cueva. Si una región no se alcanza ese día, se redistribuye su partida y se simula de nuevo; no se duplica. Una recompensa de otro día necesita otra identidad: cambiar de día no rellena un cofre ya saqueado.

## 5. Recompensas y reglas

| Fuente | Valor orientativo V | Condición |
|---|---:|---|
| Barril o saco | 0–12 | a menudo vacío; materiales o alimento |
| Cadáver | 0–20 | se descuenta del combate; no paga dos veces |
| Armario | 8–40 | ropa o materiales del lugar |
| Cofre visible | 15–60 | en la ruta normal; sin coste obligatorio de llave |
| Cofre protegido | 60–120 | llave previsible, ganzúa o fuerza |
| Escondite | 20–250 | Percepción y exploración; nada esencial |
| Encuentro normal | 10–30 | por encuentro y grupo, no por acompañante |
| Encuentro difícil | 40–80 | oro y objetos suman ese valor |
| Misión secundaria | 20–60 | alternativa excluyente; puede dar una pista o acceso |
| Hito de historia | 60–120 | recurso previsible en la ruta principal |
| Única táctica | su precio real | garantizada tras un reto; consume esa partida |

El saldo disponible de la partida manda sobre estos rangos. Una espada de 120 no cabe en un barril de 12. Se descuentan primero los garantizados y se rechaza la definición si los garantizados ya superan el presupuesto.

- Cada contenedor se genera una vez con semilla de partida e identidad de nivel e instancia. Salir, cambiar de día y recargar conservan lo cogido y lo que queda.
- Las tablas solo eligen candidatos que caben. No dan un premio gratuito de reserva cuando una tirada no cabe. Las únicas usan un registro común a toda la partida.
- Lo mundano vale poco: recoger platos no es el mejor negocio de la ciudad.
- Lo valioso exige una ruta, un requisito o un combate. La llave imprescindible, curación básica y recurso necesario para avanzar están en la ruta normal.
- Los secretos amplían opciones: empujar, marcar, una receta, una pista o un atajo. Nunca encierran la única solución de una misión principal.
- Una compra y una venta no son nuevas recompensas. Los 40–60 reutilizables de AV18-03 se reutilizan en tablas regionales; las 10–15 únicas se colocan una sola vez. Su suma debe caber, aunque eso obligue a reducir otros premios.

## 6. Perfiles y saldos esperados

La referencia usa precios neutrales. La cobertura es porcentaje del **valor** de cada fuente, no una probabilidad de apertura independiente. AV18-04 la contrastará con los niveles; los objetos conservados dan opciones, pero no oro.

| Perfil | Cobertura: cofres / combates / misiones / entorno / secretos | Objetos vendidos | Oficios de referencia |
|---|---|---:|---|
| Historia | 60 / 65 / 75 / 0 / 0 % | 70 % de su valor | ninguno necesario |
| Explora todo | 100 / 100 / 100 / 100 / 100 % | 45 % | ninguno necesario |
| Optimiza | 100 / 100 / 100 / 100 / 100 % | 100 % | un turno exitoso de servir por día, +8 |

Fracción de V que es oro directo: cofres 25 %, combates 50 %, misiones 75 %, entorno y secretos 0 %. El resto son objetos. Una única garantizada no produce además su valor en monedas.

| Perfil | Día | Cesta con precios actuales | Gasto |
|---|---:|---|---:|
| Historia | 1 | daga, 2 pociones, 2 panes; reserva narrativa 25 | 161 |
| Historia | 2 | 3 pociones, 2 panes, vendas, termas; reserva narrativa 25 | 155 |
| Historia | 3 | escudo, 3 pociones, 2 panes | 196 |
| Explora todo | 1 | espada, 2 pociones, 2 panes, vendas, termas; reserva narrativa 25 | 245 |
| Explora todo | 2 | escudo, capa, botas, 3 pociones, 2 panes, biblioteca y termas; reserva narrativa 25 | 355 |
| Explora todo | 3 | anillo, gema, 3 pociones, 2 panes, ópera y termas | 702 |
| Optimiza | 1 | daga, poción, 2 panes, cerveza | 112 |
| Optimiza | 2 | espada, poción, 2 panes; reserva narrativa 25 | 191 |
| Optimiza | 3 | anillo, 2 pociones, 2 panes | 276 |

AV18-04 corrigió una suma: la cesta del explorador del día 2 cuesta 355, no 455. Su saldo teórico de los días 2 y 3 sube 100; sigue dentro de los mismos márgenes. No se cambian precios ni presupuestos para compensarlo.

Son compras de referencia; si el kit trae arma puede elegirse otra opción. La reserva narrativa se sustituye por los costes de los beats elegidos. No se paga una compra con el oro de un cofre futuro: el simulador debe comprobar saldo y disponibilidad en cada compra, y explicar las compras imposibles.

| Perfil | Saldo día 1 | Día 2 acumulado | Día 3 acumulado | Márgenes por día |
|---|---:|---:|---:|---|
| historia | 112 | 369 | 378 | 90–130; 310–430; 320–440 |
| explora_todo | 113 | 194 | 160 | 90–130; 160–230; 130–190 |
| optimiza | 273 | 578 | 659 | 230–320; 490–670; 560–760 |

**Calibrado el 16/10 (AV18-04b, §9; recalibrado la noche del 16/10 con la cueva de Gorran, AV19-04):** estos saldos son los que da `tools/simular_economia.py` con el reparto nativo, las rutas sintetizadas y el canal de combates modelado; los márgenes son ±15 % de ese saldo. Los objetivos del 15/10 (historia 255/625/837, explora 288/694/580, optimiza 494/1202/1616) quedan en el bloque §8 como `margenes_objetivo_15_10`: no se alcanzan con el contenido actual (ver §9). Explorar todo deja menos efectivo que la historia porque compra más y conserva más equipo. El optimizador termina con aproximadamente dos veces el efectivo de historia, vendiendo sus opciones tácticas. No se exige igualar las bolsas finales.

Los márgenes se refieren a la ruta neutral. Se mostrarán aparte compras con descuento −25 % y recargo +50 %. Ante precios hostiles se aplaza equipo opcional antes de quitar curación básica.

## 7. Cierre que aún falta en el runtime

Hoy `collectRent` cobra por ciclos de 40 pasos y `Actuacion` vuelve a pagar tras 120 pasos. Caminar en círculo permite repetir ingresos. Los oficios declaran duración de una franja, pero su ejecución sigue en el carril de Claude. No se puede declarar cerrada la economía del juego suponiendo que el jugador nunca repite.

**PO-49 aprobada el 15/10 a las 20:46:** rentas una vez por día de historia; actuación una vez por sitio y día; oficios consumen su franja según D0017. Guardar esos cobros evita reiniciarlos al cargar. No cambia precios ni la reventa de negocios. Hasta su integración, el simulador debe mostrar el ingreso repetible como **sin cota** en el escenario de caminar en bucle. Los perfiles anteriores solo representan su recorrido finito.

Montajes pendientes: recompra al 25 % en comercio (Claude); generación y persistencia de contenedores y tablas (AV18-02/03); colocación alcanzable por día (Qwen, AV18-05); recompensas y alternativas de escenas (Z.AI). La bandeja §0 recoge la aprobación de PO-49; su implementación corresponde a Claude (AV18-07). El documento no toca estas zonas.

## 8. Datos para validadores y simulador

El bloque siguiente contiene la misma tabla: es la fuente del presupuesto para las herramientas; no se duplicarán los números en constantes Python. `valor` y `recoge_pct` siguen el orden de `canales`. Se rechazan cantidades negativas, porcentajes fuera de 0–100, días/regiones desconocidos y sumas distintas de 800/1.600/1.200. Las instancias de AV18-05 se descuentan de esas partidas.

```json economia
{
  "version": 1,
  "moneda": "oro",
  "saldo_inicial": 150,
  "venta_objetos_pct": 25,
  "reventa_negocios_pct": 50,
  "canales": [
    "cofres_visibles",
    "combates",
    "misiones",
    "entorno",
    "tesoros_ocultos"
  ],
  "oro_por_canal_pct": [
    25,
    50,
    75,
    0,
    0
  ],
  "presupuestos": [
    {
      "dia": 1,
      "region": "ciudad",
      "valor": [
        60,
        20,
        120,
        20,
        20
      ]
    },
    {
      "dia": 1,
      "region": "interiores",
      "valor": [
        60,
        0,
        80,
        20,
        40
      ]
    },
    {
      "dia": 1,
      "region": "cripta",
      "valor": [
        40,
        80,
        40,
        0,
        40
      ]
    },
    {
      "dia": 1,
      "region": "afueras",
      "valor": [
        40,
        40,
        40,
        20,
        20
      ]
    },
    {
      "dia": 1,
      "region": "cueva",
      "valor": [
        0,
        0,
        0,
        0,
        0
      ]
    },
    {
      "dia": 2,
      "region": "ciudad",
      "valor": [
        40,
        20,
        80,
        0,
        20
      ]
    },
    {
      "dia": 2,
      "region": "interiores",
      "valor": [
        80,
        0,
        100,
        20,
        40
      ]
    },
    {
      "dia": 2,
      "region": "cripta",
      "valor": [
        80,
        140,
        100,
        0,
        80
      ]
    },
    {
      "dia": 2,
      "region": "afueras",
      "valor": [
        60,
        60,
        60,
        20,
        40
      ]
    },
    {
      "dia": 2,
      "region": "cueva",
      "valor": [
        120,
        180,
        140,
        20,
        100
      ]
    },
    {
      "dia": 3,
      "region": "ciudad",
      "valor": [
        40,
        60,
        100,
        20,
        20
      ]
    },
    {
      "dia": 3,
      "region": "interiores",
      "valor": [
        60,
        0,
        100,
        20,
        60
      ]
    },
    {
      "dia": 3,
      "region": "cripta",
      "valor": [
        40,
        40,
        60,
        0,
        20
      ]
    },
    {
      "dia": 3,
      "region": "afueras",
      "valor": [
        40,
        80,
        60,
        20,
        40
      ]
    },
    {
      "dia": 3,
      "region": "cueva",
      "valor": [
        60,
        120,
        80,
        0,
        60
      ]
    }
  ],
  "totales_por_dia": [
    800,
    1600,
    1200
  ],
  "perfiles": {
    "historia": {
      "recoge_pct": [
        60,
        65,
        75,
        0,
        0
      ],
      "vende_objetos_pct": 70,
      "gastos": [
        161,
        155,
        196
      ],
      "oficios_exito": [
        0,
        0,
        0
      ],
      "margenes": [
        [
          90,
          130
        ],
        [
          310,
          430
        ],
        [
          320,
          440
        ]
      ],
      "saldo_esperado": [
        254.9,
        625.32,
        836.91
      ],
      "compras": [
        [
          {
            "id": "daga",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "reserva_historia",
            "cantidad": 25,
            "orden": 95
          }
        ],
        [
          {
            "id": "pocion",
            "cantidad": 3,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "vendas",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "oficio:banarse_termas",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "reserva_historia",
            "cantidad": 25,
            "orden": 95
          }
        ],
        [
          {
            "id": "escudo_madera",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 3,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          }
        ]
      ],
      "margenes_objetivo_15_10": [
        [
          230,
          290
        ],
        [
          550,
          700
        ],
        [
          750,
          950
        ]
      ]
    },
    "explora_todo": {
      "recoge_pct": [
        100,
        100,
        100,
        100,
        100
      ],
      "vende_objetos_pct": 45,
      "gastos": [
        245,
        355,
        702
      ],
      "oficios_exito": [
        0,
        0,
        0
      ],
      "margenes": [
        [
          90,
          130
        ],
        [
          160,
          230
        ],
        [
          130,
          190
        ]
      ],
      "saldo_esperado": [
        287.88,
        694.19,
        579.81
      ],
      "compras": [
        [
          {
            "id": "espada_corta",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "vendas",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "oficio:banarse_termas",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "reserva_historia",
            "cantidad": 25,
            "orden": 95
          }
        ],
        [
          {
            "id": "escudo_madera",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "capa_viajero",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "botas_cuero",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 3,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "oficio:estudiar_biblioteca",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "oficio:banarse_termas",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "reserva_historia",
            "cantidad": 25,
            "orden": 95
          }
        ],
        [
          {
            "id": "anillo_plata",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "gema_azul",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 3,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "entrada_opera",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "oficio:banarse_termas",
            "cantidad": 1,
            "orden": 90
          }
        ]
      ],
      "margenes_objetivo_15_10": [
        [
          250,
          330
        ],
        [
          500,
          700
        ],
        [
          400,
          600
        ]
      ]
    },
    "optimiza": {
      "recoge_pct": [
        100,
        100,
        100,
        100,
        100
      ],
      "vende_objetos_pct": 100,
      "gastos": [
        112,
        191,
        276
      ],
      "oficios_exito": [
        1,
        1,
        1
      ],
      "margenes": [
        [
          230,
          320
        ],
        [
          490,
          670
        ],
        [
          560,
          760
        ]
      ],
      "saldo_esperado": [
        493.5,
        1201.75,
        1616.25
      ],
      "compras": [
        [
          {
            "id": "daga",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "cerveza",
            "cantidad": 1,
            "orden": 90
          }
        ],
        [
          {
            "id": "espada_corta",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "reserva_historia",
            "cantidad": 25,
            "orden": 95
          }
        ],
        [
          {
            "id": "anillo_plata",
            "cantidad": 1,
            "orden": 90
          },
          {
            "id": "pocion",
            "cantidad": 2,
            "orden": 90
          },
          {
            "id": "pan",
            "cantidad": 2,
            "orden": 90
          }
        ]
      ],
      "margenes_objetivo_15_10": [
        [
          450,
          550
        ],
        [
          1100,
          1300
        ],
        [
          1500,
          1750
        ]
      ]
    }
  },
  "oficio_id": "servir_bar",
  "precio_oficio_exito": 8,
  "limites_contenedor": {
    "barril": [
      0,
      12
    ],
    "saco": [
      0,
      12
    ],
    "cadaver": [
      0,
      20
    ],
    "armario": [
      8,
      40
    ],
    "cofre": [
      15,
      120
    ],
    "escondite": [
      20,
      250
    ]
  }
}
```

## 9. Calibración AV18-04b (Claude, 16/10)

Qué se hizo: el reparto pasa al contrato nativo (`tools/repartir_botin.py`: prefabs `botin_<region>_<tipo>`, `eventId` único por instancia, `botin{dia,region,canal,orden}`, tres días, `disponibleDia` para el botín del día 2 y 3 en mapas del día 1); las rutas del simulador salen de las aventuras (`tools/sintetizar_rutas.py`: cada premio con su cadena de beats, sobre el esqueleto de Codex); el canal de combates se modela por victoria (presupuesto del día entre los combates posibles, 50 % en oro); la venta es el **25 %** en el motor y en todos los tenderos (`ObjectCatalog`, `validar_botin.py` lo exige); PO-49 ya estaba en el motor para rentas y actuación. `simular_economia.py` sale a **0** con los nueve márgenes dentro.

Lo que la simulación enseña y el documento del 15/10 no sabía:

| Día | Canal | Presupuesto §4 | Lo que paga el contenido en un recorrido completo (explora_todo) | Diferencia |
|---|---|---:|---:|---:|
| 1 | misiones (escenas) | 280 | ≈ 60 oro + objetos (las ramas son excluyentes; sumarlas todas da 335 + 304, pero un jugador solo recorre una) | ≈ −200 |
| 2 | misiones | 480 | ≈ 100 | ≈ −380 |
| 3 | misiones | 400 | ≈ 230 | ≈ −170 |
| 1–3 | cofres/entorno/escondites | 1.240 (las cinco regiones) | 1.240 colocados: 58 contenedores (afueras y cueva ya dentro, 16/10) | 0 |
| 1–3 | combates | 840 | modelado: lo cobra el motor al ganar | 0 (modelo) |

Es decir: los márgenes del 15/10 suponían que un recorrido cobra el 65–100 % del presupuesto de misiones, y las aventuras pagan entre un cuarto y la mitad. No es un fallo del reparto ni del simulador: es **contenido**. Dos salidas, para Oriol (PO-53): (a) aceptar los saldos reales (es lo que hay escrito ahora: la bolsa final de historia ronda 330, no 840; las cestas de §6 se compran igual porque el saldo nunca baja de 0 en ningún perfil, pero explora_todo deja sin comprar el anillo y la gema); o (b) pedir a Z.AI que las aventuras paguen más en la ruta principal (premios de 60–120 en los hitos: «hito de historia 60–120» de §5), y entonces los márgenes vuelven a subir solos al reejecutar.

**Recalibración con la cueva (Fable, noche del 16/10, AV19-04):** con `cueva_gorran` en el manifiesto, `repartir_botin.py` coloca 11 contenedores en la cueva (día 2: 120/20/100; día 3: 60/0/60, al 100 %) y los saldos suben donde se esperaba: explora_todo 113/194/160 (antes 97/127/56) y optimiza 273/578/659 (antes 246/473/509); historia apenas se mueve (112/369/378). Márgenes a ±15 % de esos saldos; `simular_economia.py` sale a 0. Es la segunda y última recalibración prevista (la primera dejó la cueva fuera a sabiendas).

Pendiente que no cierra esto: oficios de fabricar en el simulador (cuando entren en el motor, AV18-09), y `requisitos`/`disponibleDia` en la hoja del editor.
