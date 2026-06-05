import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend } from 'recharts';
import { Droplets, CloudRain, Mountain, AlertTriangle, CheckCircle, WifiOff, Thermometer, Activity } from 'lucide-react';

function App() {
  const [currentData, setCurrentData] = useState({ 
    chuva: 0, umidade: 0, inclinacao: 0, 
    temperatura: 0, accelX: 0, risco: 'Carregando...' 
  });
  const [history, setHistory] = useState([]);
  const [error, setError] = useState(false);

  const fetchData = async () => {
    try {
      const response = await axios.get('http://localhost:5000/api/data');
      const newData = response.data;
      setCurrentData(newData);
      setError(false);

      setHistory(prevHistory => {
        const time = new Date().toLocaleTimeString();
        return [...prevHistory, { time, ...newData }].slice(-15);
      });
    } catch (err) {
      setError(true);
    }
  };

  useEffect(() => {
    fetchData();
    const interval = setInterval(fetchData, 2000);
    return () => clearInterval(interval);
  }, []);

  const isCritical = currentData.risco && currentData.risco.includes("CRITICO");

  // Função para aplicar o tema frio nos cards
  const getCardStyle = (color) => ({
    flex: '1', minWidth: '220px',
    backgroundColor: '#1e293b', // Slate escuro
    border: `1px solid ${color}`,
    padding: '20px', borderRadius: '16px',
    boxShadow: `0 4px 12px ${color}33`, // Sombra suave da cor do tema
    color: '#f1f5f9', // Texto quase branco
    display: 'flex', flexDirection: 'column', justifyContent: 'center'
  });

  return (
    <div style={{ backgroundColor: '#0f172a', minHeight: '100vh', padding: '20px', fontFamily: '"Segoe UI", sans-serif', color: '#f8fafc' }}>
      
      <header style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center', marginBottom: '30px', maxWidth: '1200px', margin: '0 auto 30px' }}>
        <h1 style={{ color: '#38bdf8' }}>TerraGuard <span style={{fontWeight: 200}}>System</span> ⛈️</h1>
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: error ? '#ef4444' : '#10b981' }}>
          {error ? <WifiOff /> : <div style={{width: 10, height: 10, borderRadius: '50%', backgroundColor: '#10b981'}}></div>}
          <b>{error ? "Offline" : "Online"}</b>
        </div>
      </header>

      {/* Cards */}
      <div style={{ display: 'flex', gap: '20px', marginBottom: '40px', flexWrap: 'wrap', maxWidth: '1200px', margin: '0 auto 40px' }}>
        
        <div style={getCardStyle('#3b82f6')}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#3b82f6' }}><CloudRain size={24}/> <h3>Chuva</h3></div>
          <p style={{ fontSize: '28px', margin: '5px 0' }}>{currentData.chuva}<span style={{fontSize: '14px', opacity: 0.6}}>%</span></p>
        </div>

        <div style={getCardStyle('#06b6d4')}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#06b6d4' }}><Droplets size={24}/> <h3>Umidade</h3></div>
          <p style={{ fontSize: '28px', margin: '5px 0' }}>{currentData.umidade}%</p>
        </div>

        <div style={getCardStyle('#f59e0b')}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: '#f59e0b' }}><Thermometer size={24}/> <h3>Temp.</h3></div>
          <p style={{ fontSize: '28px', margin: '5px 0' }}>{currentData.temperatura}°C</p>
        </div>

        <div style={getCardStyle(isCritical ? '#ef4444' : '#10b981')}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px', color: isCritical ? '#ef4444' : '#10b981' }}>
            {isCritical ? <AlertTriangle size={24}/> : <CheckCircle size={24}/>} <h3>Risco</h3>
          </div>
          <p style={{ fontSize: '24px', margin: '5px 0' }}>{currentData.risco}</p>
        </div>
      </div>

      {/* Gráfico */}
      <div style={{ backgroundColor: '#1e293b', padding: '20px', borderRadius: '16px', maxWidth: '1200px', margin: '0 auto', border: '1px solid #334155' }}>
        <h2 style={{ color: '#94a3b8' }}>Monitoramento em Tempo Real</h2>
        <div style={{ width: '100%', height: 350, marginTop: '20px' }}>
          <ResponsiveContainer>
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" stroke="#94a3b8" />
              <YAxis stroke="#94a3b8" />
              <Tooltip contentStyle={{ backgroundColor: '#0f172a', border: 'none', borderRadius: '8px' }} />
              <Legend />
              <Line type="monotone" dataKey="umidade" stroke="#06b6d4" strokeWidth={3} dot={false} />
              <Line type="monotone" dataKey="chuva" stroke="#3b82f6" strokeWidth={3} dot={false} />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>
      {/* SEÇÃO DA DISCIPLINA DE ANÁLISE DE ALGORITMOS */}
      <div style={{ backgroundColor: '#1e293b', padding: '20px', borderRadius: '16px', maxWidth: '1200px', margin: '20px auto', border: '1px solid #f43f5e' }}>
        <div style={{display: 'flex', justifyContent: 'space-between', alignItems: 'center'}}>
          <h2 style={{ color: '#f43f5e' }}>Análise de Desempenho e Memória (O(n) vs O(1))</h2>
          <div style={{ color: '#94a3b8' }}>Heap Livre: <b style={{color: '#f8fafc'}}>{currentData.heap_livre} bytes</b></div>
        </div>
        
        <div style={{ width: '100%', height: 300, marginTop: '20px' }}>
          <ResponsiveContainer>
            <LineChart data={history}>
              <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
              <XAxis dataKey="time" stroke="#94a3b8" />
              <YAxis stroke="#94a3b8" label={{ value: 'Latência (µs)', angle: -90, position: 'insideLeft', fill: '#94a3b8' }} />
              <Tooltip contentStyle={{ backgroundColor: '#0f172a', border: '1px solid #334155', borderRadius: '8px' }} />
              <Legend />
              <Line type="monotone" dataKey="latencia_on" stroke="#f43f5e" name="Array Shift O(n)" strokeWidth={3} dot={false} />
              <Line type="monotone" dataKey="latencia_o1" stroke="#10b981" name="Ring Buffer O(1)" strokeWidth={3} dot={false} />
            </LineChart>
          </ResponsiveContainer>
        </div>
      </div>
    </div>
  );
}

export default App;