# 🐾 Hachi — Backend + Integración Completa

Este documento explica cómo montar **el sistema completo**: backend Node.js + frontend React + ESP32.

---

## 📁 Estructura general

```
hachi-app/          ← Frontend React (ya tienes esto)
hachi-backend/      ← Backend Express (carpeta nueva)
  ├── server.js         ← Servidor principal
  ├── package.json
  ├── datos_hachi.csv   ← Se crea automáticamente al recibir datos
  ├── alertas_hachi.csv ← Se crea automáticamente con alertas
  └── esp32_envio_datos.ino  ← Fragmento para agregar al ESP32
```

---

## 🚀 PASO A PASO COMPLETO

### PARTE 1 — Instalar y correr el backend

**Paso 1.** Abre una terminal nueva en VS Code (`Ctrl + `` ` ``).

**Paso 2.** Navega a la carpeta del backend:
```bash
cd ~/Desktop/hachi-backend
```

**Paso 3.** Activa Node 14 (como siempre):
```bash
nvm use 14
```

**Paso 4.** Instala dependencias:
```bash
npm install
npm install ws express cors
```

**Paso 5.** Inicia el backend:
```bash
npm start
```


---

### PARTE 2 — Correr el frontend

**Paso 6.** Abre **otra terminal** (sin cerrar la del backend).

**Paso 7.** Navega al frontend y activa Node 14:
```bash
cd ~/Desktop/hachi-app
nvm use 14
npm run dev
```

**Paso 8.** Abre en el navegador: **http://localhost:5173**

Verás el punto de estado en la esquina superior derecha:
- 🟢 Verde = backend conectado y recibiendo datos del ESP32
- 🔴 Rojo  = backend no disponible 

---

### PARTE 3 — Conectar el ESP32

**Paso 9.** 
```bash
ipconfig getifaddr en0
```
Anota el resultado (ejemplo: `192.168.1.45`).

**Paso 10.** Abre el archivo `esp32_envio_datos.ino` y cambia esta línea:
```cpp
const char* BACKEND_URL = "http://192.168.1.X:3001/api/datos";
//                                          ↑ Pon tu IP aquí
```

**Paso 11.** Agrega los 4 fragmentos al código principal del ESP32:

1. Al inicio (imports):
```cpp
#include <HTTPClient.h>
```

2. En variables globales:
```cpp
const char* BACKEND_URL = "http://TU_IP:3001/api/datos";
unsigned long lastEnvioBackend = 0;
const int INTERVALO_ENVIO = 2000;
```

3. Pega la función `enviarDatosBackend()` completa (está en el archivo .ino).

4. En el `loop()`, primera línea después de `loopAP()`:
```cpp
enviarDatosBackend();
```

**Paso 12.** Sube el código al ESP32. Si todo está bien, verás en el Serial Monitor:
```
📤 Datos enviados al backend OK
```

Y en la terminal del backend:
```
[23:15:02] 📡 Dato recibido | Modo:2 Bat:72% Dist:N/A cm Emoción:HAPPY
```

---

## 📊 Los CSVs — tu "base de datos"

Se crean automáticamente en la carpeta `hachi-backend/`:

### `datos_hachi.csv`
```
timestamp,fecha,hora,modo,emocion,distancia_cm,bateria_pct,voltaje_v,vibracion_activa
2026-06-06T23:15:02.000Z,2026-06-06,23:15:02,2,HAPPY,,72,7.4,0
2026-06-06T23:15:04.000Z,2026-06-06,23:15:04,2,HAPPY,,71,7.39,0
```

### `alertas_hachi.csv`
```
timestamp,fecha,hora,tipo_alerta,distancia_cm,modo
2026-06-06T23:20:10.000Z,2026-06-06,23:20:10,PELIGRO,15.3,1
2026-06-06T23:21:05.000Z,2026-06-06,23:21:05,PRECAUCION,28.7,1
```

Puedes descargarlos desde el navegador: **http://localhost:3001/api/csv**

---

## 🔌 Qué datos fluyen de dónde a dónde

```
ESP32                    Backend (Node.js)           Frontend (React)
  │                           │                           │
  │── POST /api/datos ────────►│                           │
  │   {modo, emocion,          │── filtra y valida         │
  │    distancia_cm,           │── guarda en CSV           │
  │    bateria_pct,            │── actualiza estado        │
  │    voltaje_v,              │                           │
  │    vibracion_activa}       │◄── GET /api/estado ───────│ (cada 2s)
  │                            │◄── GET /api/uso-semanal ──│ (cada 30s)
  │                            │◄── GET /api/alertas ──────│ (cada 5s)
  │                            │                           │
  │                            │── responde JSON ──────────►│
  │                            │                           │── actualiza UI
```

---

## 🛠 Filtros que aplica el backend

Antes de guardar en CSV, el backend descarta datos inválidos:

| Dato | Condición válida | Si es inválido |
|------|-----------------|----------------|
| `distancia_cm` | entre 2 y 400 cm | se guarda como vacío |
| `bateria_pct` | entre 0 y 100 | se hace clamp |
| `voltaje_v` | entre 5.0 y 9.0 V | se guarda como vacío |

Adicionalmente, si `distancia_cm < 20` → se registra como **PELIGRO** en `alertas_hachi.csv`. Si está entre 20 y 30 → **PRECAUCION**.

---

## ❓ Solución a problemas comunes

**"Cannot find module 'express'"**
```bash
cd ~/Desktop/hachi-backend
nvm use 14
npm install
```

**El frontend muestra el banner amarillo de "sin backend"**
- Verifica que el backend esté corriendo (`npm start` en `hachi-backend/`)
- Ambos deben estar activos al mismo tiempo en terminales separadas

**El ESP32 no envía datos (error HTTP -1)**
- Verifica que el ESP32 y tu Mac estén en la misma red WiFi
- Confirma la IP de tu Mac con `ipconfig getifaddr en0`
- Actualiza `BACKEND_URL` en el código del ESP32

**Los CSVs están vacíos**
- Solo se llenan cuando el ESP32 envía datos reales
- Para probar sin ESP32, usa curl desde la terminal:
```bash
curl -X POST http://localhost:3001/api/datos \
  -H "Content-Type: application/json" \
  -d '{"modo":2,"emocion":"HAPPY","distancia_cm":null,"bateria_pct":75,"voltaje_v":7.5,"vibracion_activa":false}'
```
