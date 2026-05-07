import { useEffect, useState } from 'react'
import axios from 'axios'

function App() {
  const [data, setData] = useState({
    chuva: 0,
    umidade: 0,
    inclinacao: 0,
    risco: 'Seguro'
  })

  useEffect(() => {
    const interval = setInterval(() => {
      axios
        .get('http://localhost:5000/api/data')
        .then((response) => {
          setData(response.data)
        })
    }, 1000)

    return () => clearInterval(interval)
  }, [])

  return (
    <div className="container">
      <h1>TerraGuard</h1>

      <div className="cards">
        <div className="card">
          <h2>Chuva</h2>
          <p>{data.chuva}</p>
        </div>

        <div className="card">
          <h2>Umidade</h2>
          <p>{data.umidade}</p>
        </div>

        <div className="card">
          <h2>Inclinação</h2>
          <p>{data.inclinacao}</p>
        </div>

        <div className="card risco">
          <h2>Risco</h2>
          <p>{data.risco}</p>
        </div>
      </div>
    </div>
  )
}

export default App