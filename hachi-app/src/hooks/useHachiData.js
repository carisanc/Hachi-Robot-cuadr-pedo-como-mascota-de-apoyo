/**
 * useHachiData — Hook para consultar la telemetría real del robot
 */
import { useState, useEffect } from "react";

// Puerto 3000 donde corre nuestro servidor Express + WebSocket
const API = "http://192.168.100.9:3000/api";

const ESTADO_DESCONECTADO = {
  conectado: false,
  ultimaActualizacion: null,
  modo: 0,
  emocion: "NORMAL",
  distancia_cm: null,
  bateria_pct: 0,
  voltaje_v: 0,
  vibracion_activa: false,
  frase: ""
};

export function useHachiEstado() {
  const [estado, setEstado] = useState(ESTADO_DESCONECTADO);
  const [backendOk, setBackendOk] = useState(null); // null=cargando, true/false

  useEffect(() => {
    const fetchEstado = async () => {
      try {
        const res = await fetch(`${API}/estado`, { signal: AbortSignal.timeout(3000) });
        if (!res.ok) throw new Error("HTTP " + res.status);
        const data = await res.json();
        setEstado(data);
        setBackendOk(true);
      } catch (err) {
        setBackendOk(false);
        // Sin backend ni datos inventados: forzar estado desconectado
        setEstado(ESTADO_DESCONECTADO);
      }
    };

    fetchEstado();
    const interval = setInterval(fetchEstado, 1000);
    return () => clearInterval(interval);
  }, []);

  return { estado, backendOk };
}

export function useHachiUsoSemanal() {
  const [uso, setUso] = useState([]);

  useEffect(() => {
    const fetch7dias = async () => {
      try {
        const res = await fetch(`${API}/uso-semanal`);
        if (!res.ok) throw new Error("HTTP " + res.status);
        const data = await res.json();
        setUso(data);
      } catch {
        // En lugar de inventar horas, se llena estrictamente con 0h para los últimos 7 días
        const dias = [];
        const hoy = new Date();
        for (let i = 6; i >= 0; i--) {
          const d = new Date(hoy);
          d.setDate(d.getDate() - i);
          dias.push({
            fecha: d.toISOString().slice(0, 10),
            horas: 0
          });
        }
        setUso(dias);
      }
    };

    fetch7dias();
    const interval = setInterval(fetch7dias, 10000);
    return () => clearInterval(interval);
  }, []);

  return uso;
}

export function useHachiAlertas() {
  const [alertas, setAlertas] = useState([]);

  useEffect(() => {
    const fetchAlertas = async () => {
      try {
        const res = await fetch(`${API}/alertas`);
        if (!res.ok) throw new Error("HTTP " + res.status);
        const data = await res.json();
        setAlertas(data);
      } catch {
        setAlertas([]);
      }
    };

    fetchAlertas();
    const interval = setInterval(fetchAlertas, 5000);
    return () => clearInterval(interval);
  }, []);

  return alertas;
}