# Récords

El emulador guarda la mejor puntuación de cada juego que la muestra en su
pantalla, en cada modo de juego. La enseña en la lista de juegos, en el menú de
pausa y, si quieres, encima del propio juego.

## Lo que ve el jugador

**Ajustes > Lista de juegos**

| Opción | Valores | Por defecto | Qué hace |
|---|---|---|---|
| Récords | Sí / No | Sí | Interruptor general. Con No desaparece todo lo de abajo y deja de apuntar puntuaciones: el emulador queda exactamente como antes. |
| Récord en pantalla | No / Al batirlo / Siempre | Al batirlo | *Al batirlo*: un aviso "¡NUEVO RÉCORD!" durante 3 s cuando superas un récord que ya tenías. *Siempre*: además, el récord del modo al que juegas en un recuadro arriba a la derecha durante toda la partida. |
| Mostrar récords en la lista | Sí / No | Sí | El recuadro de récords junto a la carcasa (vista de uno en uno) y una línea "Récords: ..." en la lista de juegos de cada fabricante. |
| Borrar todos los récords | acción | — | Borra los récords de todos los juegos (pulsa A dos veces). |

**Menú de pausa > Partida**

- Fila **Récords**, solo en los juegos que tienen. Los muestra como
  `A 0420 · B 0185` (o `Puntuación 0156` en los juegos de un solo modo).
  Pulsando A dos veces se borran los de ese juego.

Los juegos de McDonald's / Burger Bard conservan su récord "Sin fin" y su
marcador de siempre (`render/lcdonald_hud.cpp`); ese récord ahora también sale
en la lista y en el menú de pausa.

No se apunta nada mientras hay dos aparatos conectados
(`core/link_session.h`) ni durante el avance rápido.

## Cómo funciona

Los juegos no informan de su puntuación: se **lee de la pantalla LCD**,
segmento a segmento.

- **Juegos tipo Game & Watch (chips Sharp SM5xx):** enseñan la puntuación en
  las mismas cuatro cifras que la hora. Cada juego se calibró por separado:
  se pusieron horas conocidas en el reloj (`SM5XX::set_time()`) y, por los
  segmentos que se encendían, se supo qué segmento es cada parte de cada
  cifra. Lo que solo enciende el reloj (AM, PM, los dos puntos) indica "se
  está viendo la hora", no una puntuación.
- **Máquinas de pantalla de colores VFD (HMCS40 / MELPS4):** no tienen reloj.
  Sus cifras de 7 segmentos se encontraron por la forma (tres barras
  horizontales una encima de otra y cuatro verticales) y se agruparon en un
  número por su posición.

Durante la partida (`core/score_tracker.cpp`, una vez por fotograma):

1. Si alguna marca del reloj se ha encendido en el último segundo, el juego
   está mostrando la hora y no se lee nada. Los dos puntos parpadean, de ahí
   ese segundo de margen.
2. Si no, se descifran las cuatro cifras. Un fotograma en el que alguna cifra
   no es un número válido (un gráfico que la cruza, un redibujado) se ignora.
3. Un valor que se mantiene igual durante 6 fotogramas (~0,1 s) se pasa a
   `high_score_note()` para el modo actual. El modo lo marca el botón Juego A /
   Juego B que pulsó el jugador (`input/yokoi_input_mapping.cpp`); los juegos
   VFD tienen un solo modo.
4. Los récords se guardan en `yokoi_config/yokoi_highscores.txt` (formato
   `REF.modo=puntuación`, modos `a`, `b`, `endless`) cuando cambian: como mucho
   cada 10 s durante la partida, al terminarla y al salir del juego.

## Juegos con récord (69)

**Game & Watch de Nintendo (37):** Chef, Donkey Kong, Donkey Kong II, Donkey
Kong Jr. (New Wide Screen, Panorama), Donkey Kong Circus, Egg, Fire (Wide
Screen), Fire Attack, Green House, Helmet (CN-07, CN-17), Life Boat, Lion,
Manhole (Gold, New Wide Screen), Mario Bros., Mario's Bombs Away, Mario's
Cement Factory (New Wide Screen, CM-72, CM-72A), Mario The Juggler, Mickey &
Donald, Mickey Mouse (Wide Screen, Panorama), Octopus, Oil Panic, Parachute,
Pinball, Popeye (Wide Screen, Panorama), Rain Shower, Safe Buster, Snoopy,
Snoopy Tennis, Spitball Sparky, Turtle Bridge.

**Elektronika (17):** Nu, pogodi!, Vesyolyy povar, Ataka asteroidov,
Biathlon, Circus, Hockey, Kosmicheskiy polyot, Kosmicheskiy most,
Kot-rybolov, Kvaka-zadavaka, Morskaja ataka, Nochnye vorishki, Okhota,
Razvedchiki kosmosa, Tayny okeana, Vesyolye futbolisty, Vinni-Pukh.

**Tronica (6):** Clever Chicken, Diver's Adventure, Space Mission, Spider,
Thunder Ball, Space Rescue.

**Nelsonic (2):** Star Fox, Super Mario Bros. 3.

**Máquinas VFD, una sola puntuación (7):** Galaxian 2 (Entex), Pac Man 2 cian
y rojo (Entex), Pac-Man rev. 1 y rev. 2 (Coleco), Kingman (Tomy), Frogger
(Coleco).

Todos se comprobaron jugando al azar en una versión del núcleo sin pantalla,
leyendo la puntuación durante la partida: empieza en 0, sube y los números
coinciden con la pantalla. También se incluyen los juegos que en esas pruebas
no llegaron a puntuar, siempre que sus cifras calibraran limpiamente con el
reloj.

## Los que quedan fuera (de momento) y por qué

De los 60 Game & Watch de Nintendo del pack, 37 tienen récord; los otros 23
son los de las cuatro primeras filas de esta tabla (temporizador, Zelda, dos jugadores
o calibración fallida). Pocket Pikachu también figura como Nintendo, pero no
tiene puntuación.

| Juegos | Motivo |
|---|---|
| Bomb Sweeper, Gold Cliff, Crab Grab, Super Mario Bros., Squish | Las cifras también muestran un temporizador o una cuenta atrás durante la partida. |
| Zelda | No tiene puntuación: sus cifras son los golpes que le quedan al enemigo (40 = hay que darle 40). La partida se gana o se pierde, sin marcador. |
| Boxing, Donkey Kong Hockey, Donkey Kong 3 | Juegos de dos jugadores: los dos marcadores comparten las cifras. |
| Ball, Flagman, Vermin, Climber, Black Jack, Judge, Fire (Silver), Balloon Fight, Tropical Fish, Bassmate, Altair, Super Mario World (Nelsonic) | La calibración con el reloj falló o las cifras no se leen de forma fiable. |
| Tiger y Konami (~80) | No tienen reloj con el que calibrar, y sus cifras muestran tiempo, energía, vidas o fase tan a menudo como la puntuación. Hay que revisarlos juego a juego. |
| Dig Dug, Zackman, Donkey Kong (Coleco), Crazy Kong, Ms. Pac-Man | Lecturas dudosas: un gráfico encima de una cifra o valores que bajan. |
| Invaders, Stargate, Q*Bert, Tron, Burger Time, Pengo, Zaxxon, Turtles y otros VFD | Se encuentran cifras sueltas pero no se agrupan en un número. |
| Mascotas virtuales, relojes, calculadoras | No tienen puntuación. |

Un juego sin datos de cifras no tiene récords y no muestra nada.

## Archivos

| Archivo | Contenido |
|---|---|
| `switch/source/core/score_tracker.h/.cpp` | La lectura de la puntuación, el aviso en pantalla y los récords para los menús. |
| `switch/source/core/score_layouts.inc` | Datos generados por juego: cuatro cifras × siete segmentos (como ids de segmento `{col, line, word}`), la tabla de patrones de cada cifra, las marcas del reloj y los segmentos extra que invalidan un fotograma. |
| `switch/source/core/high_scores.h/.cpp` | Récords guardados (`high_score_get/note/flush`, más `high_score_clear` / `high_score_clear_all`). |
| `switch/source/core/switch_settings.h/.cpp` | `g_records_enabled` (`records`), `g_record_hud` (`recordhud`), `g_records_in_list` (`recordslist`). |
| `switch/source/core/switch_menu.cpp` | Las filas de Ajustes, la fila del menú de pausa, lo que se ve en la lista y la confirmación "pulsa A otra vez". |
| `switch/source/input/yokoi_input_mapping.cpp` | Botón Juego A / Juego B → `score_tracker_set_mode()`. |
| `switch/source/main.cpp` | `score_tracker_update()` tras los pasos de cada fotograma; `score_tracker_render()` encima del juego. |

### Formato de los datos

```cpp
struct ScoreLayout {
    const char* ref;
    SegId digit[4][7];  // millares .. unidades, 7 segmentos cada una (255 = ninguno)
    uint8_t table[10];  // bit i = digit[..][i] encendido, para 0..9
    SegId clock[4];     // AM / PM / dos puntos: encendido = se ve la hora
    SegId extra[4];     // encendido = este fotograma no es un número
    uint8_t alt[3];     // formas alternativas del 6 / 7 / 9 (cifras halladas por forma)
    uint8_t modes;      // 2 = Juego A + Juego B, 1 = una sola puntuación
};
```

En los juegos calibrados con el reloj, los siete segmentos van en el orden de
la lista de segmentos de la cifra de las unidades, y `table` guarda los
patrones que se vieron de verdad en el reloj. En las máquinas VFD, cuyas
cifras se hallaron por la forma, van en el orden estándar `a..g`, con la tabla
estándar de 7 segmentos más `alt`.
