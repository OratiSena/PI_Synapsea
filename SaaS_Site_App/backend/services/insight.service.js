function numberOrNull(value) {
  const number = Number(value);
  return Number.isFinite(number) ? number : null;
}

function analyzeVitals(vitals) {
  if (!vitals) {
    return {
      level: "unknown",
      title: "Aguardando dados",
      message: "Ainda nao ha sinais vitais suficientes para uma analise educacional.",
      recommendation: "Conecte o circuito e valide o posicionamento dos sensores."
    };
  }

  const bpm = numberOrNull(vitals.bpm);
  const spo2 = numberOrNull(vitals.spo2);
  const temperature = numberOrNull(vitals.temperature);
  const stressIndex = numberOrNull(vitals.stressIndex ?? vitals.stress_index);

  if ((spo2 !== null && spo2 < 90) || (bpm !== null && bpm < 50)) {
    return {
      level: "critical",
      title: "Indicadores exigem nova verificacao",
      message: "Uma ou mais leituras estimadas ficaram distantes da faixa educacional esperada.",
      recommendation: "Repita a leitura e valide os dados com equipamento certificado quando aplicavel."
    };
  }
  if (
    (bpm !== null && bpm > 100)
    || (spo2 !== null && spo2 < 95)
    || (temperature !== null && temperature > 37.8)
    || (stressIndex !== null && stressIndex > 70)
  ) {
    return {
      level: "warning",
      title: "Indicadores em faixa de atencao",
      message: "A analise por regras encontrou valores que merecem observacao educacional.",
      recommendation: "Continue a observacao e confirme a qualidade da leitura."
    };
  }
  return {
    level: "normal",
    title: "Sinais dentro da faixa esperada",
    message: "Os indicadores recebidos nao apresentaram alteracoes relevantes no momento.",
    recommendation: "Continuar observacao educacional."
  };
}

function analyzeTemperature(temperature) {
  const maxTemp = numberOrNull(temperature?.maxTemp ?? temperature?.max_temp);
  if (maxTemp === null) return null;
  if (maxTemp > 39) return { level: "critical", title: "Temperatura estimada elevada" };
  if (maxTemp > 37.8) return { level: "warning", title: "Temperatura estimada em atencao" };
  return { level: "normal", title: "Temperatura dentro da faixa esperada" };
}

// Esta camada baseada em regras pode futuramente ser substituida por IA local
// ou machine learning, preservando o mesmo contrato educacional.
function generateEducationalInsights({ vitals, temperature, alerts = [] }) {
  const vitalInsight = analyzeVitals(vitals);
  const temperatureInsight = analyzeTemperature(temperature);
  const levels = [vitalInsight?.level, temperatureInsight?.level];
  const level = levels.includes("critical")
    ? "critical"
    : levels.includes("warning") ? "warning" : vitalInsight.level;

  return {
    ...vitalInsight,
    level,
    temperature: temperatureInsight,
    alertsCount: alerts.length,
    disclaimer: "Analise educacional. Nao substitui avaliacao medica profissional."
  };
}

function buildVitalAlerts(vitals) {
  const alerts = [];
  const add = (type, title, message) => alerts.push({
    type,
    title,
    description: `${message} Sugestao: continuar observacao e validar a leitura. Este alerta nao substitui avaliacao medica.`
  });
  const bpm = numberOrNull(vitals.bpm);
  const spo2 = numberOrNull(vitals.spo2);
  const temperature = numberOrNull(vitals.temperature);
  const stress = numberOrNull(vitals.stressIndex);

  if (bpm !== null && bpm > 100) add("warning", "Frequencia cardiaca elevada", "Indicador educacional acima da faixa esperada.");
  if (bpm !== null && bpm < 50) add(bpm < 40 ? "critical" : "warning", "Frequencia cardiaca baixa", "Indicador educacional abaixo da faixa esperada.");
  if (spo2 !== null && spo2 < 95) add(spo2 < 90 ? "critical" : "warning", "Saturacao abaixo do recomendado", "A estimativa de SpO2 merece nova verificacao.");
  if (temperature !== null && temperature > 37.8) add("warning", "Temperatura estimada elevada", "A leitura de temperatura merece observacao.");
  if (stress !== null && stress > 70) add("warning", "Indice de estresse elevado", "O indicador estimado ficou acima da faixa educacional.");
  return alerts;
}

function buildTemperatureAlerts(temperature) {
  const maxTemp = numberOrNull(temperature.maxTemp);
  if (maxTemp === null || maxTemp <= 37.8) return [];
  return [{
    type: maxTemp > 39 ? "critical" : "warning",
    title: "Temperatura maxima estimada elevada",
    description: "Indicador educacional acima da faixa esperada. Sugestao: validar a leitura. Este alerta nao substitui avaliacao medica."
  }];
}

module.exports = {
  analyzeVitals,
  analyzeTemperature,
  generateEducationalInsights,
  buildVitalAlerts,
  buildTemperatureAlerts
};
