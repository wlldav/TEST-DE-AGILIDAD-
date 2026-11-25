# Prueba de Agilidad — Proyecto final ALSE

Aplicación de escritorio en **C++17 / Qt 5** con base de datos embebida **SQLite3**
para aplicar una prueba de agilidad mental (propuesta 3.6, *Juego de habilidad mental*).
Proyecto final de *Algoritmos en Sistemas Electrónicos* — Escuela Colombiana de
Ingeniería Julio Garavito.

## Qué hace

1. **Autenticación** de usuarios (usuario y contraseña) y formulario de **creación/edición de usuarios**
   (nombre, apellido, documento, fecha de nacimiento).
2. **Edad** del usuario calculada a partir de la fecha actual y su fecha de nacimiento.
3. **Formulario sociodemográfico del paciente**: nombre, apellido, documento, fecha de nacimiento,
   dirección, raza, género e ingresos familiares.
4. **Prueba de agilidad**: 11 botones circulares; se enciende **uno solo a la vez, al azar, durante 1 s**,
   y el paciente intenta pulsarlo con el ratón.
5. **Registro** de cada acción con **minuto:segundo** desde el inicio, el botón encendido y si hubo acierto o fallo.
6. **Duración configurable** por el usuario (no por el paciente) antes de iniciar.
7. **Estadísticas**: aciertos, fallos, precisión, aciertos/min, gráfica por botón y exportación a CSV.

## Compilar y ejecutar (Linux)

```bash
sudo apt install qtbase5-dev qt5-qmake libqt5sql5-sqlite doxygen
mkdir build && cd build
qmake ../AgilityTest.pro CONFIG+=release
make -j4
./agility_test
```

La base de datos se crea en `~/.local/share/AgilityTest/agility.db`
(o en la ruta de la variable de entorno `AGILITY_DB`).
En el primer arranque, la aplicación pide crear el primer usuario.

## Pruebas automáticas

```bash
# Núcleo: modelos, SQLite, motor de la prueba, estadísticas
mkdir build-tests && cd build-tests && qmake ../tests/tests.pro && make && ./tst_core

# Interfaz (sin pantalla) + capturas opcionales
mkdir build-gui && cd build-gui && qmake ../tests/gui.pro && make
QT_QPA_PLATFORM=offscreen SCREENSHOT_DIR=../docs/img ./tst_gui
```

## Documentación (Doxygen)

```bash
doxygen Doxyfile      # genera docs/html/index.html
```

## Estructura

```
AgilityTest.pro        proyecto qmake de la aplicación
Doxyfile               configuración de Doxygen
src/
  models/              Person, User, Patient, TestEvent, TestSession
  database/            DatabaseManager (único lugar con SQL)
  logic/               AgilityEngine (motor sin GUI), Statistics
  ui/                  LoginDialog, UserFormDialog, PatientFormDialog,
                       MainWindow, AgilityWindow, StatsDialog, CircleButton, BarChartWidget
tests/                 pruebas con QtTest (núcleo y GUI)
docs/                  diagramas, capturas y material del informe
```

## Diseño en una frase

La **lógica** (`AgilityEngine`, `Statistics`) y los **datos** (`DatabaseManager`, modelos) no dependen de
la interfaz; la **GUI** solo conecta señales y pantallas. Esto permite probar el núcleo sin ventanas.

## Modelo de datos

`users` · `patients` · `test_sessions (patient_id, user_id)` · `test_events (session_id, minuto, segundo, botón, acierto)`
con claves foráneas y restricciones `UNIQUE`/`CHECK`. Ver `docs/modelo.md`.

## Seguridad

Las contraseñas se guardan como `SHA-256(sal + contraseña)` con sal aleatoria por usuario.
(Para un producto real se recomendaría PBKDF2/Argon2.)


