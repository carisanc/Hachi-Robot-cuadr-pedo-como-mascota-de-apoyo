# Hachi: Robot Perro Cuadrúpedo Asistencial 🐾

Hachi es un proyecto robótico asistencial basado en una plataforma cuadrúpeda controlada por un microcontrolador **ESP32**. Diseñado para interactuar con los usuarios y proporcionar apoyo mediante dos modos de funcionamiento adaptativos:
* **Modo Asistente:** Sistema autónomo de evasión de obstáculos con detección de distancias, avisos sonoros y alertas en tiempo real.
* **Modo Compañero:** Rutinas de bailes, animaciones faciales emocionales en pantalla OLED, frases motivacionales personalizadas por género y reproducidas con sonido e interacciones hápticas.

El sistema se conecta mediante **WebSockets** a una aplicación web completa desarrollada en **React (Frontend)** y un servidor en **Node.js/Express (Backend)** que persiste los eventos, métricas y telemetría en **Firebase Cloud Firestore**.

---

## 📸 Galería del Proyecto

| Robot Físico Hachi | Aplicación Web (Dashboard) | Base de Datos (Cloud Firestore) |
| :---: | :---: | :---: |
| <img width="1600" height="1200" alt="IMG-20260728-WA0006" src="https://github.com/user-attachments/assets/8e67967b-bc86-44d5-aa41-c7a887563757" />
 | _![Dashboard Web](https://via.placeholder.com/400x300?text=Captura+de+la+Pagina+Web)_ | <img width="2169" height="1246" alt="Screenshot_20260811-191431" src="https://github.com/user-attachments/assets/7eaf1e34-e424-4692-be5a-70eeac740c92" />
 |

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
| Interface Web    | REST| Backend    | API | Almacenamiento nube    |
+------------------+     +------------+     +------------------------+
