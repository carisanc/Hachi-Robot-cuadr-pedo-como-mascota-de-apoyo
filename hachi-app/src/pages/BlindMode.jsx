import { useState, useEffect } from "react";
import HachiDog from "../components/HachiDog";
import BatteryWidget from "../components/BatteryWidget";
import UsageDashboard from "../components/UsageDashboard";
import { useHachiEstado, useHachiUsoSemanal, useHachiAlertas } from "../hooks/useHachiData";
import "./ModePages.css";

const TIPS = [
  "Hachi te avisará cuando haya un obstáculo a menos de 30 cm.",
  "La intensidad del sonido aumenta cuanto más cerca esté el obstáculo.",
  "La vibración te da retroalimentación táctil en todo momento.",
  "↩️ Cuando detecta peligro, Hachi se detiene, retrocede y cambia de ruta.",
];

const EMOCION_LABEL = {
  HAPPY:     "🚶 Caminando contigo",
  SAD:       "⚠️ Reduciendo velocidad",
  SURPRISED: "🛑 Detenido — girando",
  SLEEPY:    "💤 En espera",
  NORMAL:    "🚶 Activo",
};

export default function BlindMode({ userName, onBack }) {
  const { estado, backendOk } = useHachiEstado();
  const uso     = useHachiUsoSemanal();
  const alertas = useHachiAlertas();

  const [sessionMin, setSessionMin] = useState(0);
  const [tipIdx, setTipIdx]         = useState(0);

  useEffect(() => {
    const t = setInterval(() => setSessionMin((m) => m + 1), 60000);
    return () => clearInterval(t);
  }, []);

  useEffect(() => {
    const t = setInterval(() => setTipIdx((i) => (i + 1) % TIPS.length), 8000);
    return () => clearInterval(t);
  }, []);

  const estaConectado = Boolean(backendOk && estado.conectado);

  const dist = estado.distancia_cm;
  const bateriaPct = estado.bateria_pct ?? 0;
  const voltajeV = estado.voltaje_v ?? 0;

  const status =
    !estaConectado || dist === null || dist === 999 ? "sin-señal"
    : dist < 20   ? "danger"
    : dist < 30   ? "near"
    : "clear";

  const alertColor =
    status === "danger"    ? "#f9c8d4"
    : status === "near"    ? "#fdeea3"
    : status === "clear"   ? "#c3eac6"
    : "#e8d9c4";

  const alertText =
    status === "danger"    ? "⚠️ ¡Obstáculo muy cercano! Hachi se detuvo"
    : status === "near"    ? "🔔 Obstáculo detectado — precaución"
    : status === "clear"   ? "✅ Camino libre"
    : "📡 Esperando señal del sensor…";

  const obtenerFechaHoyLocal = () => {
    const d = new Date();
    const year = d.getFullYear();
    const month = String(d.getMonth() + 1).padStart(2, '0');
    const day = String(d.getDate()).padStart(2, '0');
    return `${year}-${month}-${day}`;
  };

  const hoyStr = obtenerFechaHoyLocal();
  const alertasHoy = alertas.filter(a => String(a.fecha) === hoyStr);

  return (
    <div className="mode-page blind-page">
      <div className="blob blob-1" />
      <div className="blob blob-2" />

      {status === "danger" && <div className="danger-flash" />}

      <header className="mode-header">
        <button className="btn-ghost back-btn" onClick={onBack}>← Volver</button>
        <div className="mode-header-title">
          <span className="mode-pill blind-pill">🦮 Modo Guía</span>
          <h2>¡Hola, {userName}! 🌿</h2>
        </div>
        <div style={{ display:"flex", gap:"8px", alignItems:"center" }}>
          <div className={`conexion-dot ${estaConectado ? "online" : "offline"}`} title={estaConectado ? "ESP32 Conectado" : "ESP32 Desconectado"} />
          <div className="session-badge">⏱ {sessionMin}min</div>
        </div>
      </header>

      <div className="mode-grid">
        <div className="mode-col-main">

          {!estaConectado && (
            <div className="banner-mock">
              ⚠️ ESP32 desconectado o sin señal. Enciende el robot para comenzar a recibir datos reales.
            </div>
          )}

          {/* Radar de obstáculo */}
          <div className="card obstacle-card" style={{ background: alertColor }}>
            <div className="obstacle-radar">
              <div className="radar-ring ring-3" />
              <div className="radar-ring ring-2" />
              <div className="radar-ring ring-1" />
              <div className={`radar-dot ${status}`} />
            </div>
            <div className="obstacle-info">
              <p className="obstacle-status">{alertText}</p>
              <p className="obstacle-distance">
                Distancia: <strong>{dist !== null && dist !== 999 ? `${Number(dist).toFixed(1)} cm` : "—"}</strong>
              </p>
              <div className="obstacle-bar-wrap">
                <div
                  className="obstacle-bar"
                  style={{
                    width: dist !== null && dist !== 999 ? `${Math.min(100, Math.max(0, ((150 - dist) / 150) * 100))}%` : "0%",
                    background:
                      status === "danger" ? "#f4a1b5"
                      : status === "near" ? "#f9d84a"
                      : "#8fd494",
                  }}
                />
              </div>
            </div>
          </div>

          {/* Hachi card */}
          <div className="card hachi-card">
            <div className="hachi-card-dog">
              <HachiDog size={150} mode="blind" />
            </div>
            <div className="hachi-card-info">
              <h3 className="hachi-name">Hachi 🦮</h3>
              <span className="mood-tag">
                {estaConectado ? (EMOCION_LABEL[estado.emocion] || "🚶 Activo") : "💤 Desconectado"}
              </span>
              <p className="hachi-info-text">
                Hachi camina de forma autónoma detectando obstáculos con su sensor ultrasónico. Las alertas sonoras y táctiles te avisan en tiempo real.
              </p>
              <div className="indicator-row">
                <div className="indicator">
                  <span className={`ind-dot ${status !== "clear" && status !== "sin-señal" ? "on" : ""}`} />
                  <span>Sonido</span>
                </div>
                <div className="indicator">
                  <span className={`ind-dot ${estaConectado && estado.vibracion_activa ? "on" : ""}`} />
                  <span>Vibración</span>
                </div>
                <div className="indicator">
                  <span className={`ind-dot ${estaConectado ? "on" : ""}`} />
                  <span>ESP32 online</span>
                </div>
              </div>
            </div>
          </div>

          {/* Estadísticas de Alertas y Peligros de la sesión/día */}
          <div className="stats-row">
            <div className="stat-card">
              <span className="stat-icon">⚠️</span>
              <p className="stat-val">{alertasHoy.length}</p>
              <p className="stat-label">Alertas hoy</p>
            </div>
            <div className="stat-card">
              <span className="stat-icon">🚨</span>
              <p className="stat-val">{alertasHoy.filter(a => String(a.tipo_alerta) === "PELIGRO").length}</p>
              <p className="stat-label">Peligros hoy</p>
            </div>
            <div className="stat-card">
              <span className="stat-icon">⏱</span>
              <p className="stat-val">{sessionMin}m</p>
              <p className="stat-label">Sesión actual</p>
            </div>
          </div>

          {/* Historial de alertas */}
          {alertasHoy.length > 0 && (
            <div className="card alertas-card">
              <p className="actions-title">🚨 Últimas alertas de la sesión</p>
              <div className="alertas-list">
                {alertasHoy.slice(-5).reverse().map((a, i) => (
                  <div key={i} className={`alerta-item ${a.tipo_alerta === "PELIGRO" ? "peligro" : "precaucion"}`}>
                    <span>{a.tipo_alerta === "PELIGRO" ? "⚠️" : "🔔"}</span>
                    <span>{a.hora}</span>
                    <span>{a.distancia_cm} cm</span>
                    <span className="alerta-tag">{a.tipo_alerta}</span>
                  </div>
                ))}
              </div>
            </div>
          )}

          <div className="card tip-card">
            <p className="tip-text">{TIPS[tipIdx]}</p>
          </div>
        </div>

        <div className="mode-col-side">
          <div className="card">
            <BatteryWidget level={bateriaPct} voltaje={voltajeV} />
          </div>
          <div className="card">
            <UsageDashboard uso={uso} />
          </div>

          {/* OLED simulada */}
          <div className="card oled-card">
            <p className="oled-label">📟 Pantalla OLED de Hachi</p>
            <div className="oled-screen">
              <p className="oled-name">{userName}</p>
              <p className="oled-msg">
                {estaConectado && dist !== null && dist !== 999 ? `${Number(dist).toFixed(0)}cm · ` : ""}
                {status === "clear" ? "Libre" : status === "danger" ? "¡Peligro!" : status === "near" ? "Cuidado" : "Desconectado"}
              </p>
              <p className="oled-sub">Modo Guía · {estaConectado ? estado.emocion : "OFF"}</p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}