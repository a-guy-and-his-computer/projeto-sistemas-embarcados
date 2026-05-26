import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { Droplets, CloudRain, Mountain, AlertTriangle, CheckCircle, WifiOff } from 'lucide-react';
import "./styles/app.css"; // Certifique-se de que este arquivo existe (pode ser o padrão do Vite)

function App() {
  const [currentData, setCurrentData] = useState({ chuva: 0, umidade: 0, inclinacao: 0, risco: 'Carregando...' });
  const [history, setHistory] = useState([]); // Guarda o histórico para o gráfico
  const [error, setError] = useState(false);

  const fetchData = async () => {
    try {
      const response = await axios.get('http://localhost:5000/api/data');
      const newData = response.data;
      
      setCurrentData(newData);
      setError(false);

      // Atualiza o histórico para o gráfico (mantém no máximo os últimos 15 registros)
      setHistory(prevHistory => {
        const time = new Date().toLocaleTimeString();
        const newRecord = { time, ...newData };
        const updatedHistory = [...prevHistory, newRecord];
        return updatedHistory.slice(-15); // Corta para manter só os 15 mais recentes
      });

    } catch (err) {
      console.error("Erro ao buscar dados da API:", err);
      setError(true);
    }
  };

  // Busca os dados a cada 2 segundos
  useEffect(() => {
    fetchData(); // Busca imediatamente na primeira vez
    const interval = setInterval(fetchData, 2000);
    return () => clearInterval(interval);
  }, []);

  // Estilo condicional para o Alerta de Risco
  const isCritical = currentData.risco && currentData.risco.includes("Crítico");

  return (
    <div style={{ padding: '20px', fontFamily: 'Arial, sans-serif', maxWidth: '1200px', margin: '0 auto' }}>
      
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '30px' }}>
        <h1>TerraGuard Dashboard ⛰️</h1>
        {error ? (
          <div style={{ color: 'red', display: 'flex', alignItems: 'center', gap: '8px' }}>
            <WifiOff size={24} /> <b>Desconectado do Backend</b>
          </div>
        ) : (
          <div style={{ color: 'green', display: 'flex', alignItems: 'center', gap: '8px' }}>
            <div style={{ width: '10px', height: '10px', backgroundColor: 'green', borderRadius: '50%' }}></div>
            <b>Sistema Online</b>
          </div>
        )}
      </header>

      {/* Cards Superiores */}
      <div style={{ display: 'flex', gap: '20px', marginBottom: '40px', flexWrap: 'wrap' }}>
        
        {/* Card Chuva */}
        <div style={cardStyle}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#3b82f6' }}>
            <CloudRain size={28} /> <h2>Chuva</h2>
          </div>
          <p style={{ fontSize: '32px', margin: '10px 0' }}>{currentData.chuva}</p>
          <small style={{ color: '#666' }}>Leitura Analógica</small>
        </div>

        {/* Card Umidade do Solo */}
        <div style={cardStyle}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#10b981' }}>
            <Droplets size={28} /> <h2>Umidade</h2>
          </div>
          <p style={{ fontSize: '32px', margin: '10px 0' }}>{currentData.umidade}%</p>
          <small style={{ color: '#666' }}>Umidade do Solo</small>
        </div>

        {/* Card Inclinação */}
        <div style={cardStyle}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#8b5cf6' }}>
            <Mountain size={28} /> <h2>Inclinação</h2>
          </div>
          <p style={{ fontSize: '32px', margin: '10px 0' }}>{currentData.inclinacao}</p>
          <small style={{ color: '#666' }}>Deslocamento X/Y</small>
        </div>

        {/* Card Alerta Principal */}
        <div style={{ ...cardStyle, border: `2px solid ${isCritical ? '#ef4444' : '#10b981'}`, backgroundColor: isCritical ? '#fef2f2' : '#ecfdf5' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: isCritical ? '#ef4444' : '#10b981' }}>
            {isCritical ? <AlertTriangle size={28} /> : <CheckCircle size={28} />} 
            <h2>Status de Risco</h2>
          </div>
          <p style={{ fontSize: '32px', margin: '10px 0', color: isCritical ? '#ef4444' : '#10b981' }}>
            {currentData.risco}
          </p>
        </div>

      </div>

      {/* Gráfico */}
      <div style={{ backgroundColor: '#fff', padding: '20px', borderRadius: '12px', boxShadow: '0 4px 6px rgba(0,0,0,0.1)' }}>
        <h2>Histórico Recente de Sensores</h2>
        <div style={{ width: '100%', height: 400, marginTop: '20px' }}>
          <ResponsiveContainer>
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" />
              <XAxis dataKey="time" />
              <YAxis yAxisId="left" />
              <YAxis yAxisId="right" orientation="right" />
              <Tooltip />
              <Line yAxisId="left" type="monotone" dataKey="umidade" stroke="#10b981" name="Umidade (%)" strokeWidth={3} />
              <Line yAxisId="right" type="monotone" dataKey="chuva" stroke="#3b82f6" name="Chuva (bruto)" strokeWidth={3} />
              <Line yAxisId="right" type="monotone" dataKey="inclinacao" stroke="#8b5cf6" name="Inclinação" strokeWidth={3} />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>

    </div>
  );
}

// Estilo reaproveitável para os cards
const cardStyle = {
  flex: '1',
  minWidth: '200px',
  backgroundColor: '#fff',
  padding: '20px',
  borderRadius: '12px',
  boxShadow: '0 4px 6px rgba(0,0,0,0.1)',
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center'
};

export default App;