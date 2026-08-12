/**
 * HACHI BACKEND - Servidor Express + WebSocket + Firebase Firestore
 * Optimizado para cambio de modo instantáneo sin lagueo y con IDs legibles
 */

const express   = require("express");
const cors      = require("cors");
const http      = require("http");
const WebSocket = require("ws");
const admin     = require("firebase-admin");
const path      = require("path");

// ============ INICIALIZACIÓN DE FIREBASE ============
const serviceAccount = require(path.join(__dirname, "firebase-key.json"));

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount)
});

const db = admin.firestore();

const app  = express();
const PORT = 3000;

app.use(cors());
app.use(express.json());

let estadoActual = {
  conectado: false,
  ultimaActualizacion: null,
  modo: 0,
  emocion: "NORMAL",
  distancia_cm: null,
  bateria_pct: 0,
  voltaje_v: 0,
  vibracion_activa: false,
  frase: "",
  genero: "F"
};

// ============ COLA EN MEMORIA Y TIMERS ============
let colaTelemetria = [];
let ultimoTiempoAlerta = 0;
const TIEMPO_BLOQUEO_ALERTA_MS = 12000; 

function fechaHoy(d = new Date()) { 
  const year = d.getFullYear();
  const month = String(d.getMonth() + 1).padStart(2, '0');
  const day = String(d.getDate()).padStart(2, '0');
  return `${year}-${month}-${day}`; 
}

function horaActual(d = new Date()) { return d.toTimeString().slice(0, 8); }

const server = http.createServer(app);
const wss    = new WebSocket.Server({ server });

let esp32Socket = null;

wss.on("connection", (ws, req) => {
  const userAgent = req.headers["user-agent"] || "";
  if (userAgent.includes("ESP32")) {
    esp32Socket = ws;
    console.log("🤖 [WS] ESP32 Conectada");
  } else {
    ws.send(JSON.stringify({ type: "init", data: estadoActual }));
  }

  // EVENTO DE RECEPCIÓN ULTRA LIGERO (0 MS DE ESPERA)
  ws.on("message", (message) => {
    try {
      const payload = JSON.parse(message);

      if (payload.source === "ESP32" && payload.data) {
        const d = payload.data;
        const ahora = new Date();
        const ts    = ahora.toISOString();
        const ahoraMs = Date.now();

        let vFloat = 0;
        if (typeof d.voltaje === "string") {
          vFloat = parseFloat(d.voltaje.replace("V", "")) || 0;
        } else if (typeof d.voltaje === "number") {
          vFloat = d.voltaje;
        }

        let distValida = (d.distancia !== 999 && d.distancia >= 2 && d.distancia <= 400) ? d.distancia : null;

        // 1. Actualización en memoria RAM
        estadoActual = {
          ...estadoActual,
          conectado: true,
          ultimaActualizacion: ts,
          modo: d.modoActual ?? 0,
          emocion: d.emocion ?? "HAPPY",
          distancia_cm: distValida,
          bateria_pct: d.bateria ?? 0,
          voltaje_v: vFloat,
          vibracion_activa: (distValida !== null && distValida <= 50),
          frase: d.frase ?? ""
        };

        // 2. Acumular en cola rápida con ID personalizado (ej: 2026-08-10_22-07-03)
        if (estadoActual.modo > 0) {
          const idPersonalizado = `${fechaHoy(ahora)}_${horaActual(ahora).replace(/:/g, "-")}`;
          
          colaTelemetria.push({
            idDoc: idPersonalizado,
            datos: {
              timestamp: ts,
              fecha: fechaHoy(ahora),
              hora: horaActual(ahora),
              modo: estadoActual.modo,
              genero: estadoActual.genero,
              emocion: estadoActual.emocion,
              distancia_cm: estadoActual.distancia_cm ?? null,
              bateria_pct: estadoActual.bateria_pct,
              voltaje_v: estadoActual.voltaje_v,
              vibracion_activa: estadoActual.vibracion_activa
            }
          });
        }

        // 3. Procesar alertas únicas con ID personalizado (ej: PELIGRO_2026-08-10_22-07-03)
        if (estadoActual.modo === 1 && estadoActual.distancia_cm !== null) {
          let tipoAlerta = null;
          if (estadoActual.distancia_cm <= 20) {
            tipoAlerta = "PELIGRO";
          } else if (estadoActual.distancia_cm > 20 && estadoActual.distancia_cm <= 50) {
            tipoAlerta = "PRECAUCION";
          }

          if (tipoAlerta && (ahoraMs - ultimoTiempoAlerta > TIEMPO_BLOQUEO_ALERTA_MS)) {
            ultimoTiempoAlerta = ahoraMs;
            const idAlerta = `${tipoAlerta}_${fechaHoy(ahora)}_${horaActual(ahora).replace(/:/g, "-")}`;

            setImmediate(() => {
              db.collection("alertas_hachi").doc(idAlerta).set({
                timestamp: ts,
                fecha: fechaHoy(ahora),
                hora: horaActual(ahora),
                tipo_alerta: tipoAlerta,
                genero: estadoActual.genero,
                distancia_cm: estadoActual.distancia_cm,
                modo: estadoActual.modo
              }).then(() => {
                console.log(`🚨 [Alerta Guardada] ${idAlerta}`);
              }).catch(err => console.error("Error guardando alerta:", err.message));
            });
          }
        }

        // 4. Retransmisión inmediata al cliente React
        wss.clients.forEach((client) => {
          if (client !== esp32Socket && client.readyState === WebSocket.OPEN) {
            client.send(JSON.stringify({ type: "update", data: estadoActual }));
          }
        });
      }
    } catch (err) {
      console.error("Error procesando mensaje WS:", err.message);
    }
  });

  ws.on("close", () => {
    if (ws === esp32Socket) {
      esp32Socket = null;
      estadoActual = {
        conectado: false,
        ultimaActualizacion: null,
        modo: 0,
        emocion: "NORMAL",
        distancia_cm: null,
        bateria_pct: 0,
        voltaje_v: 0,
        vibracion_activa: false,
        frase: "",
        genero: "F"
      };
    }
  });
});

// 🔥 SUBIDA BATCH CON ID NOMBRE LEGIBLE
setInterval(async () => {
  if (colaTelemetria.length === 0) return;

  const datosAProcesar = [...colaTelemetria];
  colaTelemetria = [];

  try {
    const batch = db.batch();
    const muestrasLote = datosAProcesar.slice(-5);

    muestrasLote.forEach((item) => {
      // Guarda usando la fecha y hora como nombre del documento
      const docRef = db.collection("telemetria_hachi").doc(item.idDoc);
      batch.set(docRef, item.datos);
    });

    await batch.commit();
    console.log(`☁️ [Firebase] ${muestrasLote.length} muestras guardadas con IDs de tiempo.`);
  } catch (err) {
    console.error("Error al sincronizar cola con Firebase:", err.message);
  }
}, 5000);

// ENDPOINT DE CONTROL DE MODO Y GÉNERO
app.post("/api/modo", (req, res) => {
  const { modo, genero } = req.body;
  if (typeof modo === "number") {
    estadoActual.modo = modo;

    if (genero) {
      estadoActual.genero = genero;
    }

    if (modo === 0) {
      ultimoTiempoAlerta = 0;
      colaTelemetria = [];
    }

    // 1. Notificar inmediatamente a la ESP32 por WebSocket
    if (esp32Socket && esp32Socket.readyState === WebSocket.OPEN) {
      esp32Socket.send(JSON.stringify({ command: "SET_MODO", modo, genero: estadoActual.genero }));
    }

    // 2. Responder a la web de inmediato sin esperar a Firebase
    res.json({ ok: true, modo, genero: estadoActual.genero });

    // 3. Ejecutar limpieza de alertas en segundo plano
    if (modo === 0) {
      setImmediate(() => {
        db.collection("alertas_hachi").get().then(snapshot => {
          const batch = db.batch();
          snapshot.docs.forEach((doc) => batch.delete(doc.ref));
          return batch.commit();
        }).catch(err => console.error("Error limpiando alertas en segundo plano:", err.message));
      });
    }
    return;
  }
  res.status(400).json({ error: "Modo inválido" });
});

app.get("/api/estado", (req, res) => {
  if (estadoActual.ultimaActualizacion) {
    const diff = Date.now() - new Date(estadoActual.ultimaActualizacion).getTime();
    if (diff > 8000) {
      estadoActual.conectado = false;
      estadoActual.distancia_cm = null;
      estadoActual.bateria_pct = 0;
      estadoActual.voltaje_v = 0;
    }
  }
  res.json(estadoActual);
});

app.get("/api/uso-semanal", async (req, res) => {
  try {
    const snapshot = await db.collection("telemetria_hachi")
      .orderBy("timestamp", "desc")
      .limit(3000)
      .get();

    const hoy = new Date();
    const semana = {};
    for (let i = 6; i >= 0; i--) {
      const d = new Date(hoy);
      d.setDate(d.getDate() - i);
      semana[fechaHoy(d)] = 0;
    }

    snapshot.docs.forEach((doc) => {
      const data = doc.data();
      if (semana.hasOwnProperty(data.fecha) && data.modo > 0) {
        semana[data.fecha] += 2 / 60;
      }
    });

    const resultado = Object.entries(semana).map(([fecha, minutos]) => ({
      fecha,
      horas: parseFloat(minutos.toFixed(1)),
    }));

    res.json(resultado);
  } catch (err) {
    console.error("Error obteniendo uso semanal desde Firebase:", err.message);
    res.status(500).json({ error: "Error consultando uso semanal" });
  }
});

app.get("/api/alertas", async (req, res) => {
  try {
    const snapshot = await db.collection("alertas_hachi")
      .orderBy("timestamp", "asc")
      .limit(500)
      .get();

    const alertas = snapshot.docs.map(doc => doc.data());
    res.json(alertas);
  } catch (err) {
    console.error("Error obteniendo alertas desde Firebase:", err.message);
    res.status(500).json({ error: "Error consultando alertas" });
  }
});

server.listen(PORT, "0.0.0.0", () => {
  console.log(`Servidor Hachi activo en http://localhost:${PORT}`);
});