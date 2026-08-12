import { useState, useEffect } from "react";
import HachiDog from "../components/HachiDog";
import BatteryWidget from "../components/BatteryWidget";
import UsageDashboard from "../components/UsageDashboard";
import { useHachiEstado, useHachiUsoSemanal } from "../hooks/useHachiData";
import "./ModePages.css";

const EMOCION_LABEL = {
  HAPPY:     "🐾 Alegre y compañero",
  SURPRISED: "✨ Emocionado",
  SLEEPY:    "💤 Descansando",
  SAD:       "🤗 Dándote apoyo",
  NORMAL:    "🐾 Contigo",
};

export default function DepressionMode({ userName, userGender, onBack }) {
  const { estado, backendOk } = useHachiEstado();
  const uso = useHachiUsoSemanal();

  const [sessionMin, setSessionMin] = useState(0);

  useEffect(() => {
    const t = setInterval(() => setSessionMin((m) => m + 1), 60000);
    return () => clearInterval(t);
  }, []);

  const estaConectado = Boolean(backendOk && estado.conectado);
  
  // Frase enviada por la ESP32 o la por defecto según género
  const frasePorDefecto = userGender === "F" ? "TU PUEDES REINA" : "TU PUEDES REY";
  const fraseRealEsp = estaConectado ? (estado.frase || frasePorDefecto) : "—";
  
  const bateriaPct = estado.bateria_pct ?? 0;
  const voltajeV = estado.voltaje_v ?? 0;
  const vibracionActiva = estaConectado ? estado.vibracion_activa : false;
  const emocionActual = estaConectado ? estado.emocion : "NORMAL";

  return (
    <div className="mode-page depression-page">
      <div className="blob blob-1" />
      <div className="blob blob-2" />
      <div className="blob blob-3" />

      <header className="mode-header">
        <button className="btn-ghost back-btn" onClick={onBack}>← Volver</button>
        <div className="mode-header-title">
          <span className="mode-pill depression-pill">💙 Modo Compañero</span>
          <h2>Hola, {userName} 🌸</h2>
        </div>
        <div style={{ display:"flex", gap:"8px", alignItems:"center" }}>
          <div 
            className={`conexion-dot ${estaConectado ? "online" : "offline"}`} 
            title={estaConectado ? "ESP32 Conectado" : "ESP32 Desconectado"} 
          />
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

          <div className="card hachi-card depression-hachi">
            <div className="hachi-card-dog">
              <HachiDog size={160} mode="calm" />
            </div>
            <div className="hachi-card-info">
              <h3 className="hachi-name">Hachi 🐾</h3>
              <span className="mood-tag">
                {estaConectado ? (EMOCION_LABEL[emocionActual] || "🐾 Contigo") : "💤 Desconectado"}
              </span>
              <p className="hachi-info-text">
                Hachi camina suavemente a tu lado, emite pequeñas vibraciones como caricias y muestra frases de apoyo en su pantalla.
              </p>
            </div>
          </div>

          <div className="card phrase-card visible">
            <span className="phrase-emoji">🌸</span>
            <p className="phrase-text">
              {estaConectado ? `"${fraseRealEsp}"` : "📡 Esperando conexión con Hachi…"}
            </p>
            <p className="phrase-note">💬 Mensaje transmitido desde la pantalla OLED de Hachi</p>
          </div>

          <div className="card sensor-row-card" style={{ display: "grid", gridTemplateColumns: "1fr 1fr", gap: "16px" }}>
            <div className="sensor-item">
              <div className="sensor-icon-wrap blue"><span>📳</span></div>
              <div>
                <p className="sensor-label">Vibración</p>
                <p className={`sensor-val ${vibracionActiva ? "active-val" : ""}`}>
                  {estaConectado ? (vibracionActiva ? "Activa · suave" : "En espera") : "Inactiva"}
                </p>
              </div>
            </div>
            <div className="sensor-item">
              <div className="sensor-icon-wrap pink"><span>🎭</span></div>
              <div>
                <p className="sensor-label">Emoción actual</p>
                <p className="sensor-val active-val">{estaConectado ? emocionActual : "—"}</p>
              </div>
            </div>
          </div>
        </div>

        <div className="mode-col-side">
          <div className="card">
            <BatteryWidget level={bateriaPct} voltaje={voltajeV} />
          </div>
          <div className="card">
            <UsageDashboard uso={uso} />
          </div>

          <div className="card oled-card">
            <p className="oled-label">📟 Pantalla OLED de Hachi</p>
            <div className="oled-screen">
              <p className="oled-name">Hola, {userName}!</p>
              <p className="oled-msg">{estaConectado ? fraseRealEsp : "Desconectado"}</p>
              <p className="oled-sub">
                {estaConectado ? (EMOCION_LABEL[emocionActual] || "Modo Compañero") : "OFF"}
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}