import { useState, useEffect } from "react";
import "./BatteryWidget.css";

/**
 * BatteryWidget
 * Props:
 *   level    {number}  — porcentaje real desde el hook useHachiEstado (0-100)
 *   voltaje  {number}  — voltaje real desde el ESP32
 */
export default function BatteryWidget({ level = 0, voltaje = null }) {
  const [showAlert, setShowAlert] = useState(false);

  useEffect(() => {
    setShowAlert(level > 0 && level <= 15);
  }, [level]);

  const getColor = () => {
    if (level > 50) return "#c3eac6";
    if (level > 20) return "#fdeea3";
    return "#f9c8d4";
  };
  const getIcon = () => {
    if (level > 50) return "🔋";
    if (level > 20) return "🪫";
    return "⚠️";
  };

  return (
    <div className="battery-widget">
      {showAlert && (
        <div className="battery-alert">
          🌙 ¡Es hora de que tu amigo descanse!
          <br />
          <span>Conecta a Hachi para recargarlo</span>
        </div>
      )}
      <div className="battery-label">
        <span>{getIcon()} Batería</span>
        <span className="battery-pct">{Math.round(level)}%</span>
      </div>
      <div className="battery-bar-track">
        <div
          className="battery-bar-fill"
          style={{
            width: `${level}%`,
            background: getColor(),
            transition: "width 1s ease, background 1s ease",
          }}
        />
      </div>
      <div className="battery-status">
        {voltaje ? `${voltaje}V · ` : ""}
        {level > 50 ? "¡Listo para jugar!" : level > 20 ? "Cargando pronto…" : level > 0 ? "Batería baja" : "Sin datos"}
      </div>
    </div>
  );
}
