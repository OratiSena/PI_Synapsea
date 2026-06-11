#include "telemetry.h"

struct VitalSnapshot {
  int bpm;
  int spo2;
  int respiration;
  int hrv;
  int signalQuality;
  float pi;
  bool fingerDetected;
};

static float rawSnapshot[AMG_ROWS * AMG_COLS];
static float interpolatedSnapshot[INTERPOLATED_ROWS * INTERPOLATED_COLS];
static float rawForUpload[AMG_ROWS * AMG_COLS];
static float interpolatedForUpload[INTERPOLATED_ROWS * INTERPOLATED_COLS];
static VitalSnapshot vitalSnapshot;
static VitalSnapshot vitalForUpload;
static volatile bool temperatureUploadPending = false;
static volatile bool vitalsUploadPending = false;
static unsigned long lastTemperatureScheduled = 0;
static unsigned long lastVitalsScheduled = 0;
static portMUX_TYPE snapshotMux = portMUX_INITIALIZER_UNLOCKED;

static void appendMatrix(
  String &json,
  const float *values,
  int rows,
  int columns
) {
  json += '[';
  for (int y = 0; y < rows; y++) {
    if (y > 0) json += ',';
    json += '[';
    for (int x = 0; x < columns; x++) {
      if (x > 0) json += ',';
      json += String(values[y * columns + x], 2);
    }
    json += ']';
  }
  json += ']';
}

static void calculateRawStats(
  const float *values,
  float &minimum,
  float &maximum,
  float &average,
  int &hotspotX,
  int &hotspotY
) {
  minimum = values[0];
  maximum = values[0];
  float sum = 0.0f;
  int hotspotIndex = 0;

  for (int index = 0; index < AMG_ROWS * AMG_COLS; index++) {
    const float value = values[index];
    sum += value;
    if (value < minimum) minimum = value;
    if (value > maximum) {
      maximum = value;
      hotspotIndex = index;
    }
  }

  average = sum / (AMG_ROWS * AMG_COLS);
  hotspotX = hotspotIndex % AMG_COLS;
  hotspotY = hotspotIndex / AMG_COLS;
}

static int calculateStressIndex(const VitalSnapshot &vital) {
  if (!vital.fingerDetected || vital.bpm <= 0 || vital.spo2 <= 0) return -1;

  int score = 0;
  if (vital.hrv == 0) score += 25;
  else if (vital.hrv > 60) score += 0;
  else if (vital.hrv > 40) score += 10;
  else if (vital.hrv > 25) score += 20;
  else if (vital.hrv > 10) score += 30;
  else score += 40;

  if (vital.bpm < 65) score += 0;
  else if (vital.bpm < 80) score += 8;
  else if (vital.bpm < 95) score += 16;
  else score += 25;

  if (vital.pi <= 0.0f) score += 10;
  else if (vital.pi > 5.0f) score += 0;
  else if (vital.pi > 2.0f) score += 7;
  else if (vital.pi > 0.5f) score += 14;
  else score += 20;

  if (vital.respiration == 0) score += 7;
  else if (vital.respiration >= 12 && vital.respiration <= 20) score += 0;
  else if (vital.respiration <= 26) score += 8;
  else score += 15;
  return score;
}

static const char *getVitalStatus(const VitalSnapshot &vital) {
  if (!vital.fingerDetected) return "warning";
  if (
    (vital.spo2 > 0 && vital.spo2 < 90)
    || (vital.bpm > 0 && (vital.bpm < 45 || vital.bpm > 130))
  ) return "critical";
  if (
    vital.signalQuality < 2
    || (vital.spo2 > 0 && vital.spo2 < 94)
    || (vital.bpm > 0 && (vital.bpm < 55 || vital.bpm > 110))
  ) return "warning";
  return "normal";
}

static const char *getSignalQuality(const VitalSnapshot &vital) {
  if (!vital.fingerDetected || vital.signalQuality == 0) return "no_finger";
  return vital.signalQuality >= 2 ? "good" : "poor";
}

static bool ensureWiFiConnected() {
  if (WiFi.status() == WL_CONNECTED) return true;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Telemetria: conectando ao Wi-Fi");

  const unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 10000) {
    Serial.print('.');
    vTaskDelay(pdMS_TO_TICKS(250));
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Telemetria: Wi-Fi indisponivel");
    return false;
  }

  Serial.print("Telemetria: Wi-Fi conectado, IP ");
  Serial.println(WiFi.localIP());
  return true;
}

static void addCommonHeaders(HTTPClient &http) {
  http.addHeader("Content-Type", "application/json");
  http.addHeader("ngrok-skip-browser-warning", "true");
  if (strlen(DEVICE_API_KEY) > 0) {
    http.addHeader("x-device-key", DEVICE_API_KEY);
  }
}

static int sendRequest(const char *url, const String *payload = nullptr) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.setConnectTimeout(4000);
  http.setTimeout(6000);
  if (!http.begin(client, url)) {
    Serial.println("Telemetria: URL HTTPS invalida");
    return -1;
  }

  addCommonHeaders(http);
  const int statusCode = payload ? http.POST(*payload) : http.GET();
  http.end();
  return statusCode;
}

static void testApiStatus() {
  const int statusCode = sendRequest(API_STATUS_URL);
  Serial.print("Telemetria status HTTP ");
  Serial.println(statusCode);
}

static void uploadTemperature() {
  float minimum, maximum, average;
  int hotspotX, hotspotY;
  calculateRawStats(
    rawForUpload,
    minimum,
    maximum,
    average,
    hotspotX,
    hotspotY
  );

  String payload;
  payload.reserve(12000);
  payload += F("{\"patientId\":\"");
  payload += PATIENT_ID;
  payload += F("\",\"deviceId\":\"");
  payload += DEVICE_ID_AMG;
  payload += F("\",\"grid\":");
  appendMatrix(payload, rawForUpload, AMG_ROWS, AMG_COLS);
  payload += F(",\"interpolatedGrid\":");
  appendMatrix(
    payload,
    interpolatedForUpload,
    INTERPOLATED_ROWS,
    INTERPOLATED_COLS
  );
  payload += F(",\"interpolationWidth\":");
  payload += String(INTERPOLATED_COLS);
  payload += F(",\"interpolationHeight\":");
  payload += String(INTERPOLATED_ROWS);
  payload += F(",\"maxTemp\":");
  payload += String(maximum, 2);
  payload += F(",\"minTemp\":");
  payload += String(minimum, 2);
  payload += F(",\"avgTemp\":");
  payload += String(average, 2);
  payload += F(",\"hotspotX\":");
  payload += String(hotspotX);
  payload += F(",\"hotspotY\":");
  payload += String(hotspotY);
  payload += '}';

  const int statusCode = sendRequest(API_TEMPERATURE_URL, &payload);
  Serial.print("Telemetria AMG HTTP ");
  Serial.println(statusCode);
}

static void uploadVitals() {
  const int stressIndex = calculateStressIndex(vitalForUpload);

  String payload;
  payload.reserve(512);
  payload += F("{\"patientId\":\"");
  payload += PATIENT_ID;
  payload += F("\",\"deviceId\":\"");
  payload += DEVICE_ID_MAX;
  payload += F("\",\"bpm\":");
  payload += vitalForUpload.bpm > 0 ? String(vitalForUpload.bpm) : "null";
  payload += F(",\"spo2\":");
  payload += vitalForUpload.spo2 > 0 ? String(vitalForUpload.spo2) : "null";
  payload += F(",\"temperature\":null,\"stressIndex\":");
  payload += stressIndex >= 0 ? String(stressIndex) : "null";
  payload += F(",\"respiration\":");
  payload += vitalForUpload.respiration > 0
    ? String(vitalForUpload.respiration)
    : "null";
  payload += F(",\"hrv\":");
  payload += vitalForUpload.hrv > 0 ? String(vitalForUpload.hrv) : "null";
  payload += F(",\"pi\":");
  payload += vitalForUpload.pi > 0.0f ? String(vitalForUpload.pi, 2) : "null";
  payload += F(",\"ppg\":[],\"signalQuality\":\"");
  payload += getSignalQuality(vitalForUpload);
  payload += F("\",\"fingerDetected\":");
  payload += vitalForUpload.fingerDetected ? "true" : "false";
  payload += F(",\"status\":\"");
  payload += getVitalStatus(vitalForUpload);
  payload += F("\"}");

  const int statusCode = sendRequest(API_VITALS_URL, &payload);
  Serial.print("Telemetria MAX HTTP ");
  Serial.println(statusCode);
}

static void telemetryTask(void *parameter) {
  (void)parameter;
  bool apiStatusTested = false;

  for (;;) {
    if (!ensureWiFiConnected()) {
      apiStatusTested = false;
      vTaskDelay(pdMS_TO_TICKS(3000));
      continue;
    }

    if (!apiStatusTested) {
      testApiStatus();
      apiStatusTested = true;
    }

    bool shouldUploadTemperature = false;
    bool shouldUploadVitals = false;
    portENTER_CRITICAL(&snapshotMux);
    if (temperatureUploadPending) {
      memcpy(rawForUpload, rawSnapshot, sizeof(rawForUpload));
      memcpy(
        interpolatedForUpload,
        interpolatedSnapshot,
        sizeof(interpolatedForUpload)
      );
      temperatureUploadPending = false;
      shouldUploadTemperature = true;
    }
    if (vitalsUploadPending) {
      vitalForUpload = vitalSnapshot;
      vitalsUploadPending = false;
      shouldUploadVitals = true;
    }
    portEXIT_CRITICAL(&snapshotMux);

    if (shouldUploadVitals) uploadVitals();
    if (shouldUploadTemperature) uploadTemperature();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void iniciarTelemetria() {
  WiFi.mode(WIFI_STA);
  xTaskCreatePinnedToCore(
    telemetryTask,
    "synapsea-telemetry",
    12288,
    nullptr,
    1,
    nullptr,
    0
  );
}

void agendarTelemetriaAMG() {
  const unsigned long now = millis();
  if (now - lastTemperatureScheduled < TELEMETRY_UPLOAD_INTERVAL_MS) return;

  portENTER_CRITICAL(&snapshotMux);
  if (!temperatureUploadPending) {
    memcpy(rawSnapshot, pixels, sizeof(rawSnapshot));
    memcpy(interpolatedSnapshot, dest_2d, sizeof(interpolatedSnapshot));
    temperatureUploadPending = true;
    lastTemperatureScheduled = now;
  }
  portEXIT_CRITICAL(&snapshotMux);
}

void agendarTelemetriaMAX() {
  const unsigned long now = millis();
  if (now - lastVitalsScheduled < TELEMETRY_UPLOAD_INTERVAL_MS) return;

  portENTER_CRITICAL(&snapshotMux);
  if (!vitalsUploadPending) {
    vitalSnapshot.bpm = bpmReal;
    vitalSnapshot.spo2 = spo2Real;
    vitalSnapshot.respiration = respRPM;
    vitalSnapshot.hrv = hrvRMSSD;
    vitalSnapshot.pi = piVal;
    vitalSnapshot.signalQuality = qualSinal;
    vitalSnapshot.fingerDetected = dedoDetectado;
    vitalsUploadPending = true;
    lastVitalsScheduled = now;
  }
  portEXIT_CRITICAL(&snapshotMux);
}
