import "./UsageDashboard.css";

const DIAS_CORTOS = ["Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"];

/**
 * UsageDashboard
 * Props:
 *   uso  {Array<{fecha: string, horas: number}>} — del hook useHachiUsoSemanal (donde horas representa minutos acumulados)
 */
export default function UsageDashboard({ uso = [] }) {
  // Si no hay datos reales, mostrar estructura vacía con 0
  const datos = uso.length > 0 ? uso : Array(7).fill(null).map((_, i) => ({
    fecha: new Date(Date.now() - (6 - i) * 86400000).toISOString().slice(0, 10),
    horas: 0,
  }));

  // Interpretamos la propiedad como minutos acumulados
  const maxMinutos = Math.max(...datos.map(d => d.horas), 1);
  const totalMinutos = datos.reduce((a, b) => a + (Number(b.horas) || 0), 0);
  const avgMinutos = (totalMinutos / 7).toFixed(1);

  // Encontrar el índice del día con mayor uso
  const mejorIdx = datos.indexOf(datos.reduce((a, b) => (a.horas > b.horas ? a : b)));

  const getDiaCorto = (fecha) => {
    const d = new Date(fecha + "T12:00:00");
    return DIAS_CORTOS[d.getDay()];
  };

  // Formateador dinámico (Minutos vs Horas)
  const formatearTiempo = (minutos, incluirUnidad = true) => {
    const minNum = Number(minutos) || 0;
    if (minNum >= 60) {
      const hrs = (minNum / 60).toFixed(1);
      return incluirUnidad ? `${hrs}h` : hrs;
    }
    const mins = minNum.toFixed(0);
    return incluirUnidad ? `${mins}m` : mins;
  };

  return (
    <div className="usage-dashboard">
      <div className="usage-header">
        <span className="usage-title">⏱ Uso semanal</span>
        <span className="usage-total">
          {totalMinutos > 0 ? `${formatearTiempo(totalMinutos)} esta semana` : "0m esta semana"}
        </span>
      </div>

      <div className="usage-bars">
        {datos.map((d, i) => {
          const min = Number(d.horas) || 0;
          return (
            <div key={d.fecha || i} className="usage-bar-col">
              <span className="usage-val">
                {min > 0 ? formatearTiempo(min) : "-"}
              </span>
              <div className="usage-bar-track">
                <div
                  className="usage-bar-fill"
                  style={{
                    height: `${(min / maxMinutos) * 100}%`,
                    animationDelay: `${i * 0.07}s`,
                    background: i === mejorIdx && min > 0
                      ? "linear-gradient(180deg, #bde0f5, #8fc8e8)"
                      : "linear-gradient(180deg, var(--pastel-pink), var(--pastel-purple))",
                  }}
                />
              </div>
              <span className="usage-day">{getDiaCorto(d.fecha)}</span>
            </div>
          );
        })}
      </div>

      <div className="usage-footer">
        <span>Promedio: <strong>{formatearTiempo(avgMinutos)}/día</strong></span>
        {datos[mejorIdx]?.horas > 0 && (
          <span>Mejor día: <strong>{getDiaCorto(datos[mejorIdx].fecha)}</strong> 🐾</span>
        )}
      </div>
    </div>
  );
}