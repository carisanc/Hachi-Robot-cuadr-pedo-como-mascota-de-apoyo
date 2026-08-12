# 🐾 Hachi: Robot Perro Cuadrúpedo Asistencial

Hachi es un proyecto robótico asistencial basado en una plataforma cuadrúpeda controlada por un microcontrolador **ESP32**. Está diseñado para interactuar con los usuarios y brindarles apoyo mediante dos modos de funcionamiento adaptativos:

- 🛡️ **Modo Asistente** — Sistema autónomo de evasión de obstáculos con detección de distancias, avisos sonoros y alertas en tiempo real.
- 🎉 **Modo Compañero** — Rutinas de baile, animaciones faciales emocionales en pantalla OLED y frases motivacionales personalizadas por género, reproducidas con sonido e interacciones hápticas.

El sistema se conecta mediante **WebSockets** a una aplicación web completa desarrollada en **React** (frontend) y un servidor en **Node.js/Express** (backend), que persiste eventos, métricas y telemetría en **Firebase Cloud Firestore**.

---

## 📸 Galería del Proyecto

| Robot Físico Hachi | Aplicación Web (Dashboard) | Base de Datos (Cloud Firestore) |
| :---: | :---: | :---: |
| <img width="1600" height="1200" alt="IMG-20260728-WA0006" src="https://github.com/user-attachments/assets/2e1f4089-1c03-4ddf-be5b-f759b2057f71" /> | <img width="1280" height="554" alt="Captura de pantalla 2026-08-11 a la(s) 20 29 14" src="https://github.com/user-attachments/assets/aaed0b33-f8ae-4214-a7cd-23c4ec17ff68" /> <img width="1279" height="552" alt="Captura de pantalla 2026-08-11 a la(s) 20 29 41" src="https://github.com/user-attachments/assets/2f7780ed-cf5d-44f8-9b22-5ab234230d1d" /> | <img width="2169" height="1246" alt="Screenshot_20260811-191431" src="https://github.com/user-attachments/assets/74a73b4b-30db-4624-b443-b12724f8a7a1" /> |

---

## 🛠️ Arquitectura del Sistema

```text
                   +------------------------+
                   |  ESP32 (Robot Hachi)   |
                   | Servos + OLED + Audio  |
                   +-----------+------------+
                               |
                   WebSocket (Telemetría / Modo)
                               v
+------------------+     +------------+     +------------------------+
| React App (Vite) | <-> | Node.js    | --> | Google Cloud Firestore |
| Interfaz Web      |REST| Backend    | API | Almacenamiento en nube |
+------------------+     +------------+     +------------------------+
```

| Componente | Descripción |
| --- | --- |
| **Microcontrolador (ESP32)** | Gestiona la marcha cuadrúpeda (servos vía PCA9685), el sensor de ultrasonido HC-SR04, la pantalla OLED SSD1306, el módulo de audio DFPlayer Mini y el motor de vibración. |
| **Backend (Node.js + Express)** | Servidor HTTP y WebSocket. Administra una cola de mensajes en memoria (buffer) para evitar latencias en la ESP32 y realiza escrituras en lote (*batch writes*) a la base de datos. |
| **Base de datos (Firebase Cloud Firestore)** | Almacena el historial de telemetría, el registro de uso semanal y el registro de alertas de seguridad. |
| **Frontend (React + Vite)** | Panel interactivo con estadísticas en tiempo real, cambio de modos, personalización de género y monitoreo de batería. |

---

## 🗄️ Persistencia de Datos (Firebase Cloud Firestore)

El proyecto utiliza Cloud Firestore para garantizar una estructura de datos escalable en tiempo real. La información se organiza en dos colecciones principales:

### 1. Colección `telemetria_hachi`

Almacena muestras periódicas de rendimiento y estado del robot:

| Campo | Descripción |
| --- | --- |
| `timestamp` | Estampa de tiempo en formato ISO. |
| `fecha` / `hora` | Cadenas legibles para reportes locales. |
| `modo` | ID del modo activo (`1` = Asistente, `2` = Compañero). |
| `genero` | Preferencia de tratamiento del usuario (`"F"` o `"M"`). |
| `emocion` | Estado emocional activo (`"HAPPY"`, `"SAD"`, `"SURPRISED"`, etc.). |
| `distancia_cm` | Lectura del sensor ultrasónico. |
| `bateria_pct` / `voltaje_v` | Nivel de batería filtrado. |
| `vibracion_activa` | Booleano del estado del motor háptico. |

### 2. Colección `alertas_hachi`

Registra eventos únicos de detección de obstáculos bajo un esquema estricto:

| Nivel | Condición |
| --- | --- |
| `PELIGRO` | Distancia ≤ 20 cm |
| `PRECAUCION` | Distancia entre 21 cm y 50 cm |

---

## 🚀 Estructura del Repositorio

```text
.
├── hachi-app/               # Aplicación Frontend en React + Vite
│   ├── src/                 # Componentes, hooks y páginas de la app
│   └── package.json
├── hachi-backend/           # Servidor Backend en Node.js + Express
│   ├── firebase-key.json    # Credenciales de servicio Firebase (ignorado en Git)
│   ├── server.js            # Servidor WebSocket y API REST
│   └── package.json
└── hachi/                    # Código C++/Arduino para ESP32 (PlatformIO)
    ├── src/
    │   └── main.cpp          # Firmware del robot Hachi
    └── platformio.ini
```

---

## 🧰 Requisitos e Instalación

### Requisitos previos

- Node.js v18 o superior.
- [PlatformIO](https://platformio.org/) (extensión para VS Code) o Arduino IDE para compilar el firmware de la ESP32.
- Un proyecto configurado en la consola de Firebase con **Firestore Database** habilitado.

### 1. Configuración del Backend (`hachi-backend`)

```bash
cd hachi-backend
npm install
```

1. Descarga la clave privada de tu cuenta de servicio desde la consola de Firebase:
   `Configuración del Proyecto ➔ Cuentas de Servicio ➔ Generar nueva clave privada`.
2. Renombra el archivo descargado como `firebase-key.json` y colócalo dentro de la carpeta `hachi-backend/`.
3. Inicia el servidor:

   ```bash
   npm start
   ```

### 2. Configuración del Frontend (`hachi-app`)

```bash
cd hachi-app
npm install
```

1. Ajusta la dirección IP de tu servidor backend en `src/hooks/useHachiData.js` y `src/pages/Welcome.jsx` si es necesario.
2. Ejecuta el entorno de desarrollo:

   ```bash
   npm run dev
   ```

### 3. Carga del Firmware en la ESP32 (`cero`)

1. Abre el proyecto ubicado en la carpeta `cero` usando PlatformIO.
2. Verifica la dirección IP del servidor Node.js en la variable `serverWS_IP` dentro de `src/main.cpp`.
3. Conecta tu placa ESP32 por USB y compila/sube el código.

---

## ⚙️ Uso de la Aplicación

1. Enciende el robot Hachi. La ESP32 se conectará a la red Wi-Fi configurada y establecerá el canal WebSocket con el servidor backend.
2. Abre la aplicación React en tu navegador.
3. Ingresa tu nombre, selecciona tu preferencia de género y elige el modo de operación deseado (Asistente o Compañero).
4. Monitorea la telemetría en vivo, el nivel de batería, el estado del sensor y las gráficas de uso semanal directamente en el panel principal.

---

## Autores

Carolina Sánchez Cevallos. Ingeniería en Mecatrónica.
Jhony Choez López. Ingeniería en Mecatrónica.
