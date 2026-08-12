import { useState } from "react";
import HachiDog from "../components/HachiDog";
import "./Welcome.css";

const API_BASE_URL = "http://192.168.100.9:3000/api";

const MODES = [
  {
    id: "blind",
    icon: "🦮",
    title: "Asistente para personas con discapacidad visual",
    desc: "Hachi caminará contigo, detectará obstáculos y te avisará con sonido y vibración.",
    color: "var(--pastel-green)",
    border: "#8fd494",
  },
  {
    id: "depression",
    icon: "💙",
    title: "Asistente para personas con depresión",
    desc: "Hachi te hará compañía con movimientos suaves, frases de apoyo en su pantalla y sonidos.",
    color: "var(--pastel-blue)",
    border: "#8fc8e8",
  },
];

export default function Welcome({ onStart }) {
  const [name, setName] = useState("");
  const [gender, setGender] = useState("F"); // "F" = Femenino, "M" = Masculino
  const [selected, setSelected] = useState("");
  const [error, setError] = useState("");
  const [loading, setLoading] = useState(false);

  const handleStart = async () => {
    if (!name.trim()) { setError("¡Dinos tu nombre para continuar! 🐾"); return; }
    if (!selected)     { setError("Elige el modo en que Hachi te acompañará 🐾"); return; }

    setError("");
    setLoading(true);

    const numModo = selected === "blind" ? 1 : 2;

    try {
      // Enviar el modo y género seleccionado al backend para enviarlo a la ESP32 por WebSocket
      await fetch(`${API_BASE_URL}/modo`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ modo: numModo, genero: gender }),
      });
      console.log(`✅ Comando de modo ${numModo} (Género: ${gender}) enviado a Hachi.`);
    } catch (err) {
      console.error("⚠️ No se pudo enviar el modo al servidor:", err);
    } finally {
      setLoading(false);
      onStart(name.trim(), selected, gender);
    }
  };

  return (
    <div className="welcome-page">
      <div className="blob blob-1" />
      <div className="blob blob-2" />

      <div className="welcome-inner">
        <div className="welcome-dog">
          <HachiDog size={130} mode={selected || "happy"} />
        </div>

        <h2 className="welcome-heading">¡Hola! Soy Hachi 🐾</h2>
        <p className="welcome-sub">Cuéntame un poco antes de empezar</p>

        {/* Campo Nombre */}
        <div className="welcome-field">
          <label className="field-label">¿Cómo te llamas?</label>
          <input
            className="welcome-input"
            type="text"
            placeholder="Tu nombre…"
            value={name}
            onChange={(e) => { setName(e.target.value); setError(""); }}
            maxLength={30}
            disabled={loading}
          />
        </div>

        {/* Campo Género */}
        <div className="welcome-field">
          <label className="field-label">¿Cómo prefieres que Hachi se dirija a ti?</label>
          <div style={{ display: "flex", gap: "12px", marginTop: "6px" }}>
            <button
              type="button"
              className={`btn-ghost ${gender === "F" ? "active-gender" : ""}`}
              style={{
                flex: 1,
                padding: "10px",
                borderRadius: "12px",
                border: gender === "F" ? "2px solid #8fc8e8" : "1px solid #ccc",
                backgroundColor: gender === "F" ? "#eef7fc" : "transparent",
                fontWeight: gender === "F" ? "bold" : "normal",
                cursor: "pointer"
              }}
              onClick={() => setGender("F")}
            >
              👑 Femenino (Reina)
            </button>
            <button
              type="button"
              className={`btn-ghost ${gender === "M" ? "active-gender" : ""}`}
              style={{
                flex: 1,
                padding: "10px",
                borderRadius: "12px",
                border: gender === "M" ? "2px solid #8fc8e8" : "1px solid #ccc",
                backgroundColor: gender === "M" ? "#eef7fc" : "transparent",
                fontWeight: gender === "M" ? "bold" : "normal",
                cursor: "pointer"
              }}
              onClick={() => setGender("M")}
            >
              👑 Masculino (Rey)
            </button>
          </div>
        </div>

        {/* Campo Selector de Modo */}
        <div className="welcome-field">
          <label className="field-label">¿En qué modo quieres que Hachi te acompañe?</label>
          <div className="mode-cards">
            {MODES.map((m) => (
              <div
                key={m.id}
                className={`mode-card ${selected === m.id ? "selected" : ""}`}
                style={{
                  "--card-color": m.color,
                  "--card-border": m.border,
                }}
                onClick={() => { if (!loading) { setSelected(m.id); setError(""); } }}
              >
                <span className="mode-icon">{m.icon}</span>
                <div>
                  <p className="mode-title">{m.title}</p>
                  <p className="mode-desc">{m.desc}</p>
                </div>
                <span className="mode-check">{selected === m.id ? "✓" : ""}</span>
              </div>
            ))}
          </div>
        </div>

        {error && <p className="welcome-error">{error}</p>}

        <button 
          className="btn-primary welcome-btn" 
          onClick={handleStart}
          disabled={loading}
        >
          {loading ? "Conectando con Hachi..." : "¡Vamos, Hachi! 🐾"}
        </button>
      </div>
    </div>
  );
}