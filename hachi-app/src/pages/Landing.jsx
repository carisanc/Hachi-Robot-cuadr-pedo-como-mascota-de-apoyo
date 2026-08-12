import HachiDog from "../components/HachiDog";
import "./Landing.css";

export default function Landing({ onEnter }) {
  return (
    <div className="landing">
      {/* Decorative blobs */}
      <div className="blob blob-1" />
      <div className="blob blob-2" />
      <div className="blob blob-3" />

      {/* Paw prints scattered */}
      {["🐾","🐾","🐾","🐾","🐾"].map((p,i) => (
        <span key={i} className={`paw paw-${i+1}`}>{p}</span>
      ))}

      <div className="landing-content">
        <div className="landing-dog-wrap">
          <div className="pulse-ring" />
          <HachiDog size={240} mode="happy" />
        </div>

        <div className="landing-text">
          <h1 className="landing-title">
            <span className="title-h">H</span>
            <span className="title-a">a</span>
            <span className="title-c">c</span>
            <span className="title-hi">hi</span>
          </h1>
          <p className="landing-sub">
            Tu compañero robot con corazón 🐶✨
          </p>
          <p className="landing-desc">
            Un amigo siempre presente, diseñado para acompañarte,
            guiarte y recordarte que no estás solo.
          </p>
          <button className="btn-primary landing-btn" onClick={onEnter}>
            ¡Conoce a Hachi! 🐾
          </button>
        </div>

        <div className="landing-badges">
          <span className="badge">🦮 Apoyo emocional</span>
          <span className="badge">🔊 Guía sonora</span>
          <span className="badge">📡 Control WiFi</span>
          <span className="badge">💡 ESP32</span>
        </div>
      </div>
    </div>
  );
}
