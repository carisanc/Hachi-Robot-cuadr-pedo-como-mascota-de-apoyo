import { useState } from "react";
import Landing from "./pages/Landing";
import Welcome from "./pages/Welcome";
import BlindMode from "./pages/BlindMode";
import DepressionMode from "./pages/DepressionMode";
import "./App.css";

const API_BASE_URL = "http://192.168.100.9:3000/api";

export default function App() {
  const [page, setPage] = useState("landing");
  const [userName, setUserName] = useState("");
  const [userGender, setUserGender] = useState("F");
  const [mode, setMode] = useState("");

  const goToWelcome = () => setPage("welcome");

  const goToDashboard = (name, selectedMode, gender) => {
    setUserName(name);
    setUserGender(gender);
    setMode(selectedMode);
    setPage(selectedMode === "blind" ? "blind" : "depression");
  };

  const handleBackToWelcome = async () => {
    try {
      await fetch(`${API_BASE_URL}/modo`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ modo: 0 }),
      });
      console.log("⏪ Regresando a Modo 0 (Menú) en la ESP32");
    } catch (err) {
      console.error("Error enviando comando de regreso a la ESP32:", err);
    }

    setPage("welcome");
  };

  return (
    <div className="app-root">
      {page === "landing" && <Landing onEnter={goToWelcome} />}
      {page === "welcome" && <Welcome onStart={goToDashboard} />}
      {page === "blind" && (
        <BlindMode userName={userName} onBack={handleBackToWelcome} />
      )}
      {page === "depression" && (
        <DepressionMode 
          userName={userName} 
          userGender={userGender} 
          onBack={handleBackToWelcome} 
        />
      )}
    </div>
  );
}