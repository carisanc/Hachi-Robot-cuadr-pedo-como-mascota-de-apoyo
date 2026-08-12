#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_PWMServoDriver.h>
#include <DFRobotDFPlayerMini.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WebSocketsClient.h>

// ============ CONFIGURACIÓN DE PINES ============
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define TRIG_PIN 5
#define ECHO_PIN 18
#define VIBRA_PIN 12
#define PIN_VOLTAJE 32

// ============ SERVOS ============
#define CENTRO 375
#define PASO_HOMBRO 8
#define PASO_PIERNA 40
#define PASO_ACOSTADO 180
#define PASO_GIRO_HOMBRO 10
#define PASO_GIRO_PIERNA 40

// ============ BATERÍA ============
const float FACTOR_VOLTAJE = 3.2;
const float VOLT_MAX = 8.4;
const float VOLT_MIN = 5.0;
float ultimo_porcentaje = -1;

// ============ WEBSOCKET ============
WebSocketsClient webSocket;
const char* serverWS_IP = "192.168.100.9";
const int serverWS_port = 3000;
unsigned long lastTelemetryTime = 0;
const int telemetryInterval = 500;
const char* emotionNames[] = { "NORMAL", "HAPPY", "SAD", "SURPRISED", "SLEEPY" };
float distanciaFiltroActual = 999;
String stringVoltaje = "0.0V";
int pctBateriaGlobal = 0;

// ============ VARIABLES GLOBALES ============
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_PWMServoDriver pca = Adafruit_PWMServoDriver(0x40);
HardwareSerial mySerial(2);
DFRobotDFPlayerMini player;
WebServer server(80);

// ============ EMOCIONES ============
enum Emotion { NORMAL, HAPPY, SAD, SURPRISED, SLEEPY };
Emotion emotion = HAPPY;

// ============ ANIMACIÓN OJOS ============
float eyeX = 0, eyeY = 0;
float targetX = 0, targetY = 0;
unsigned long lastLookMove = 0;
unsigned long lastBlink = 0;
bool blinking = false;
int blinkLevel = 0;
unsigned long nextBlink = 4000;

// ============ ANIMACIÓN MANITAS ============
float manitaIzqY = 0;
float manitaDerY = 0;
unsigned long lastManitaAnim = 0;
int pasoManitas = 0;

// ============ MOVIMIENTO ============
unsigned long lastMovimientoTime = 0;
int pasoMovimiento = 0;
bool movimientoActivo = false;
bool estaRetrocediendo = false;

// ============ VARIABLES PARA BAILES ============
unsigned long tiempoInicioBaile = 0;
bool ejecutandoBaile = false;
int baileActual = 0;
bool acostado = false;

// ============ FRASES Y GÉNERO ============
String generoUsuario = "F"; // "F" = Femenino, "M" = Masculino

const char* frasesFemenino[] = {
  "TU PUEDES REINA",
  "ERES HERMOSA!",
  "ERES COOL!",
  "SONRIE REINA",
  "ERES UNICA"
};

const char* frasesMasculino[] = {
  "TU PUEDES REY",
  "ERES ASOMBROSO!",
  "ERES COOL!",
  "SONRIE REY",
  "ERES UNICO"
};

int numFrases = 5;
unsigned long lastFraseTime = 0;
bool mostrandoFrase = false;
unsigned long fraseDuration = 3000;
String fraseActual = "";

// ============ MÚSICA ============
int currentTrack = 1;
int ultimaCancion = 0;
unsigned long tiempoInicioCancion = 0;
const int duracionCancion = 85000;

// ============ CONTROL ============
int modoActual = 0;
bool robotEncendido = false;
bool menuVisible = true;
unsigned long lastEmotionChange = 0;
unsigned long lastBaileTime = 0;
unsigned long lastBateriaTime = 0;

// ============ VARIABLES PARA EL SENSOR ============
unsigned long ultimoIntentoSensor = 0;
int fallosConsecutivos = 0;
float ultimaDistanciaValida = 999;
bool sensorBloqueado = false;

// ============ VARIABLES PARA SECUENCIA DE OBSTÁCULO ============
bool secuenciaObstaculoActiva = false;
int direccionAleatoria = -1;
bool direccionBloqueada = false;
bool sonidoReproducido = false;
bool retrocesoCompletado = false;
bool sonidoTerminado = false;
bool giroCompletado = false;
unsigned long tiempoInicioSonidoDir = 0;
unsigned long tiempoInicioGiro = 0;
const int duracionSonidoDir = 4000;
const int duracionGiro = 4000;

// ============ PROTOTIPOS ============
void mostrarMenu();
void drawDogFace();
void updateAnimation();
void drawEarLeft();
void drawEarRight();
void drawEye(int cx, int cy, bool leftEye);
void drawNose();
float medirDistancia();
float medirDistanciaSegura();
void vibrar(int duracionMs);
void vibracionLarga();
void animarManitas();
void mostrarFraseAleatoria();
void actualizarBateria();
void cambiarMusica(int track);
void iniciarMusicaAleatoria();
void reproducirLadrido();
void detenerServos();
void ejecutarMovimiento();
void retroceder();
void girarDerecha();
void girarIzquierda();
void bailarAcostarse();
void bailarOrinar();
void bailarSaludar();
void bailarSentarseMano();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
void enviarTelemetriaWeb();

// ============ EEPROM Y WIFI ============

String leerStringDeEEPROM(int direccion) {
    String cadena = "";
    char caracter = EEPROM.read(direccion);
    int i = 0;
    while (caracter != '\0' && i < 100) {
        cadena += caracter;
        i++;
        caracter = EEPROM.read(direccion + i);
    }
    return cadena;
}

void escribirStringEnEEPROM(int direccion, String cadena) {
    int longitudCadena = cadena.length();
    for (int i = 0; i < longitudCadena; i++) {
        EEPROM.write(direccion + i, cadena[i]);
    }
    EEPROM.write(direccion + longitudCadena, '\0');
    EEPROM.commit();
}

void handleRoot() {
    String html = "";
    html += "<html><body style='text-align:center;font-family:Arial;'>";
    html += "<h1>PERRO ROBOT</h1>";
    html += "<h2>Configuracion WiFi</h2>";
    html += "<form action='/wifi'>";
    html += "Red Wi-Fi: <input type='text' name='ssid'><br><br>";
    html += "Contraseña: <input type='text' name='password'><br><br>";
    html += "<input type='submit' value='Conectar'>";
    html += "</form>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void handleWifi() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    Serial.print("Conectando a: ");
    Serial.println(ssid);
    
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str(), 6);
    
    int cnt = 0;
    while (WiFi.status() != WL_CONNECTED && cnt < 8) {
        delay(1000);
        Serial.print(".");
        cnt++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado!");
        String varsave = leerStringDeEEPROM(300);
        int posW = 50;
        if (varsave == "a") {
            posW = 50;
            escribirStringEnEEPROM(300, "b");
        } else {
            posW = 0;
            escribirStringEnEEPROM(300, "a");
        }
        escribirStringEnEEPROM(0 + posW, ssid);
        escribirStringEnEEPROM(100 + posW, password);
        server.send(200, "text/plain", "Conexion establecida");
    } else {
        Serial.println("\nConexion fallida");
        server.send(200, "text/plain", "Conexion no establecida");
    }
}

bool lastRed() {
    for (int psW = 0; psW <= 50; psW += 50) {
        String usu = leerStringDeEEPROM(0 + psW);
        String cla = leerStringDeEEPROM(100 + psW);
        if (usu == "") continue;
        Serial.print("Intentando: ");
        Serial.println(usu);
        WiFi.disconnect();
        WiFi.begin(usu.c_str(), cla.c_str(), 6);
        int cnt = 0;
        while (WiFi.status() != WL_CONNECTED && cnt < 5) {
            delay(1000);
            Serial.print(".");
            cnt++;
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConectado!");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            return true;
        }
    }
    return false;
}

void initAP(const char *apSsid, const char *apPassword) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid, apPassword);
    server.on("/", handleRoot);
    server.on("/wifi", handleWifi);
    server.begin();
    Serial.println("AP creada: ");
    Serial.println(apSsid);
    Serial.println("IP: 192.168.4.1");
}

void loopAP() {
    server.handleClient();
}

void intentoConexion(const char *apname, const char *appassword) {
    EEPROM.begin(512);
    if (!lastRed()) {
        Serial.println("Conectate a la red:");
        Serial.println(apname);
        Serial.println("IP: 192.168.4.1");
        initAP(apname, appassword);
    }
    while (WiFi.status() != WL_CONNECTED) {
        loopAP();
        delay(50);
    }
}

// ============ WEBSOCKET ============

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WS] Desconectado del servidor de Node.js");
            break;
        case WStype_CONNECTED:
            Serial.println("[WS] Conectado exitosamente al servidor Node.js");
            break;
        case WStype_TEXT: {
            Serial.printf("[WS] Mensaje recibido: %s\n", payload);
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                if (doc.containsKey("command") && doc["command"] == "SET_MODO") {
                    int nuevoModo = doc["modo"];
                    
                    // Leer género enviado desde la web
                    if (doc.containsKey("genero")) {
                        generoUsuario = doc["genero"].as<String>();
                        Serial.print("Género recibido: ");
                        Serial.println(generoUsuario);
                    }
                    
                    if (nuevoModo == 0) {
                        modoActual = 0;
                        menuVisible = true;
                        robotEncendido = false;
                        player.stop();
                        detenerServos();
                        mostrarMenu();
                        Serial.println("REGRESO A MENU OLED DESDE FRONTEND");
                    } 
                    else if (nuevoModo == 1) {
                        menuVisible = false;
                        modoActual = 1;
                        robotEncendido = true;
                        emotion = HAPPY;
                        cambiarMusica(1);
                        Serial.println("MODO ASISTENTE ACTIVADO DESDE FRONTEND");
                    } 
                    else if (nuevoModo == 2) {
                        menuVisible = false;
                        modoActual = 2;
                        robotEncendido = true;
                        emotion = HAPPY;
                        lastFraseTime = millis();
                        lastEmotionChange = millis();
                        lastBaileTime = millis();
                        tiempoInicioBaile = millis();
                        mostrandoFrase = false;
                        ejecutandoBaile = false;
                        iniciarMusicaAleatoria();
                        Serial.println("MODO DEPRESION ACTIVADO DESDE FRONTEND");
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void enviarTelemetriaWeb() {
    if (menuVisible) return;

    JsonDocument doc;
    doc["source"] = "ESP32";
    JsonObject data = doc.createNestedObject("data");
    
    data["distancia"] = (modoActual == 1) ? distanciaFiltroActual : 999;
    data["bateria"] = pctBateriaGlobal;
    data["voltaje"] = stringVoltaje;
    data["emocion"] = emotionNames[emotion];
    data["modoActual"] = modoActual;
    data["frase"] = (modoActual == 2 && mostrandoFrase) ? fraseActual : "";

    String output;
    serializeJson(doc, output);
    webSocket.sendTXT(output);
}

// ============ MOVIMIENTOS ============

void detenerServos() {
  for (int i = 0; i < 8; i++) {
    pca.setPWM(i, 0, CENTRO);
    delay(10);
  }
}

// ============ CAMINATA NORMAL ============
void ejecutarMovimiento() {
  int delayTime = 500;

  if (millis() - lastMovimientoTime >= delayTime) {
    lastMovimientoTime = millis();
    pasoMovimiento++;
    if (pasoMovimiento > 7) pasoMovimiento = 0;

    switch (pasoMovimiento) {
      case 0:
        pca.setPWM(1, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_HOMBRO);
        break;
      case 1:
        pca.setPWM(0, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_PIERNA);
        break;
      case 2:
        pca.setPWM(1, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_HOMBRO);
        break;
      case 3:
        pca.setPWM(0, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_PIERNA);
        break;
      case 4:
        pca.setPWM(1, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_HOMBRO);
        break;
      case 5:
        pca.setPWM(0, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_PIERNA);
        break;
      case 6:
        pca.setPWM(1, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_HOMBRO);
        break;
      case 7:
        pca.setPWM(0, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_PIERNA);
        break;
    }
  }
}

// ============ RETROCEDER ============
void retroceder() {
  int delayTime = 500;

  if (millis() - lastMovimientoTime >= delayTime) {
    lastMovimientoTime = millis();
    pasoMovimiento++;
    if (pasoMovimiento > 7) pasoMovimiento = 0;

    switch (pasoMovimiento) {
      case 0:
        pca.setPWM(1, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_HOMBRO);
        break;
      case 1:
        pca.setPWM(0, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_PIERNA);
        break;
      case 2:
        pca.setPWM(1, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_HOMBRO);
        break;
      case 3:
        pca.setPWM(0, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_PIERNA);
        break;
      case 4:
        pca.setPWM(1, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_HOMBRO);
        break;
      case 5:
        pca.setPWM(0, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_PIERNA);
        break;
      case 6:
        pca.setPWM(1, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_HOMBRO);
        break;
      case 7:
        pca.setPWM(0, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_PIERNA);
        break;
    }
  }
}

// ============ GIRO A LA DERECHA (TLD QUIETA) ============
void girarDerecha() {
  int delayTime = 500;

  if (millis() - lastMovimientoTime >= delayTime) {
    lastMovimientoTime = millis();
    pasoMovimiento++;
    if (pasoMovimiento > 7) pasoMovimiento = 0;

    switch (pasoMovimiento) {
      case 0:
        pca.setPWM(1, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_GIRO_HOMBRO);
        break;
      case 1:
        pca.setPWM(0, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_GIRO_PIERNA);
        break;
      case 2:
        pca.setPWM(1, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_GIRO_HOMBRO);
        break;
      case 3:
        pca.setPWM(0, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_GIRO_PIERNA);
        break;
      case 4:
        pca.setPWM(1, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(5, 0, CENTRO + PASO_GIRO_HOMBRO);
        break;
      case 5:
        pca.setPWM(0, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(4, 0, CENTRO + PASO_GIRO_PIERNA);
        break;
      case 6:
        pca.setPWM(1, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(5, 0, CENTRO - PASO_GIRO_HOMBRO);
        break;
      case 7:
        pca.setPWM(0, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(4, 0, CENTRO - PASO_GIRO_PIERNA);
        break;
    }
  }
}

// ============ GIRO A LA IZQUIERDA (TLI QUIETA) ============
void girarIzquierda() {
  int delayTime = 500;

  if (millis() - lastMovimientoTime >= delayTime) {
    lastMovimientoTime = millis();
    pasoMovimiento++;
    if (pasoMovimiento > 7) pasoMovimiento = 0;

    switch (pasoMovimiento) {
      case 0:
        pca.setPWM(1, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_GIRO_HOMBRO);
        break;
      case 1:
        pca.setPWM(0, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_GIRO_PIERNA);
        break;
      case 2:
        pca.setPWM(1, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_GIRO_HOMBRO);
        break;
      case 3:
        pca.setPWM(0, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_GIRO_PIERNA);
        break;
      case 4:
        pca.setPWM(1, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(7, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO - PASO_GIRO_HOMBRO);
        break;
      case 5:
        pca.setPWM(0, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(6, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO - PASO_GIRO_PIERNA);
        break;
      case 6:
        pca.setPWM(1, 0, CENTRO + PASO_GIRO_HOMBRO);
        pca.setPWM(7, 0, CENTRO - PASO_GIRO_HOMBRO);
        pca.setPWM(3, 0, CENTRO + PASO_GIRO_HOMBRO);
        break;
      case 7:
        pca.setPWM(0, 0, CENTRO + PASO_GIRO_PIERNA);
        pca.setPWM(6, 0, CENTRO - PASO_GIRO_PIERNA);
        pca.setPWM(2, 0, CENTRO + PASO_GIRO_PIERNA);
        break;
    }
  }
}

// ============ FUNCIÓN DE VIBRACIÓN POR PULSOS LENTOS ============
void vibrarPulsosLentos(int duracionMs) {
  unsigned long inicio = millis();
  unsigned long tiempoVibracion = 0;
  bool vibrando = true;
  int intervalo = 400;
  
  while (millis() - inicio < duracionMs) {
    if (millis() - tiempoVibracion > intervalo) {
      tiempoVibracion = millis();
      vibrando = !vibrando;
      digitalWrite(VIBRA_PIN, vibrando ? HIGH : LOW);
    }
    delay(10);
  }
  digitalWrite(VIBRA_PIN, LOW);
}

// ============ BAILES PARA MODO DEPRESION ============

void bailarAcostarse() {
  Serial.println("Bailando: Acostarse");
  pca.setPWM(0, 0, CENTRO - PASO_ACOSTADO);
  pca.setPWM(2, 0, CENTRO + PASO_ACOSTADO);
  vibrarPulsosLentos(1000);
  
  pca.setPWM(4, 0, CENTRO - PASO_ACOSTADO);
  pca.setPWM(6, 0, CENTRO + PASO_ACOSTADO);
  vibrarPulsosLentos(1000);
  
  pca.setPWM(1, 0, CENTRO);
  pca.setPWM(3, 0, CENTRO);
  pca.setPWM(5, 0, CENTRO);
  pca.setPWM(7, 0, CENTRO);
  
  acostado = true;
  vibrarPulsosLentos(3000);
  
  for (int i = 0; i < 8; i++) {
    pca.setPWM(i, 0, CENTRO);
    delay(20);
  }
  
  acostado = false;
  digitalWrite(VIBRA_PIN, LOW);
  Serial.println("Baile completado");
}

void bailarOrinar() {
  Serial.println("Bailando: Orinar");
  pca.setPWM(5, 0, CENTRO - 60);
  pca.setPWM(4, 0, CENTRO - 80);
  vibrarPulsosLentos(2000);
  
  for (int i = 0; i < 4; i++) {
    pca.setPWM(4, 0, CENTRO - 80 - 40);
    delay(150);
    pca.setPWM(4, 0, CENTRO - 80 + 40);
    delay(150);
  }
  
  pca.setPWM(4, 0, CENTRO - 80);
  delay(200);
  
  pca.setPWM(4, 0, CENTRO);
  pca.setPWM(5, 0, CENTRO);
  vibrarPulsosLentos(2000);
  
  digitalWrite(VIBRA_PIN, LOW);
  Serial.println("Baile Orinar completado");
}

void bailarSaludar() {
  Serial.println("Saludando...");
  pca.setPWM(3, 0, CENTRO - 30);
  vibrarPulsosLentos(300);
  
  pca.setPWM(2, 0, CENTRO + 180);
  vibrarPulsosLentos(500);
  
  for (int i = 0; i < 3; i++) {
    pca.setPWM(2, 0, CENTRO + 180 - 50);
    vibrarPulsosLentos(300);
    pca.setPWM(2, 0, CENTRO + 180);
    vibrarPulsosLentos(300);
  }
  
  pca.setPWM(2, 0, CENTRO);
  vibrarPulsosLentos(300);
  
  pca.setPWM(3, 0, CENTRO);
  vibrarPulsosLentos(300);
  
  digitalWrite(VIBRA_PIN, LOW);
  Serial.println("Saludo completado!");
}

void bailarSentarseMano() {
  Serial.println("Bailando: Sentarse y dar la mano");
  pca.setPWM(0, 0, CENTRO);
  pca.setPWM(2, 0, CENTRO);
  pca.setPWM(4, 0, CENTRO - 150);
  pca.setPWM(6, 0, CENTRO + 150);
  vibrarPulsosLentos(1000);
  
  pca.setPWM(3, 0, CENTRO - 60);
  vibrarPulsosLentos(2000);
  pca.setPWM(2, 0, CENTRO + 150);
  vibrarPulsosLentos(2000);
  
  pca.setPWM(2, 0, CENTRO);
  pca.setPWM(3, 0, CENTRO);
  vibrarPulsosLentos(500);
  
  for (int i = 0; i < 8; i++) {
    pca.setPWM(i, 0, CENTRO);
    delay(20);
  }
  digitalWrite(VIBRA_PIN, LOW);
  Serial.println("Sentarse y dar la mano completado!");
}

// ============ MÚSICA ============

void cambiarMusica(int track) {
  if (direccionBloqueada) {
    if (direccionAleatoria == 0 && track != 3) return;
    if (direccionAleatoria == 1 && track != 4) return;
  }
  
  if (track == currentTrack) return;
  player.stop();
  delay(50);
  player.play(track);
  currentTrack = track;
}

void iniciarMusicaAleatoria() {
  int cancion = random(5, 11);
  if (cancion == ultimaCancion) {
    cancion = random(5, 11);
  }
  ultimaCancion = cancion;
  player.play(cancion);
  currentTrack = cancion;
  tiempoInicioCancion = millis();
}

void reproducirLadrido() {
  if (currentTrack == 2) return;
  player.stop();
  delay(50);
  player.play(2);
  currentTrack = 2;
}

// ============ BATERÍA ============
const int NUM_MUESTRAS_FILTRO = 10;
float historialBateria[NUM_MUESTRAS_FILTRO] = {0};
int indiceHistorial = 0;

void actualizarBateria() {
  const int NUM_MUESTRAS = 5;
  float suma = 0;
  
  for (int i = 0; i < NUM_MUESTRAS; i++) {
    int raw = analogRead(PIN_VOLTAJE);
    float voltajePin = (raw * 3.3) / 4095.0;
    float voltajePila = voltajePin * FACTOR_VOLTAJE;
    suma += voltajePila;
    delay(20);
  }
  
  float voltajePila = suma / NUM_MUESTRAS;
  float porcentaje = (voltajePila - VOLT_MIN) / (VOLT_MAX - VOLT_MIN) * 100.0;
  if (porcentaje < 0) porcentaje = 0;
  if (porcentaje > 100) porcentaje = 100;
  
  historialBateria[indiceHistorial] = porcentaje;
  indiceHistorial++;
  if (indiceHistorial >= NUM_MUESTRAS_FILTRO) {
    indiceHistorial = 0;
  }
  
  float sumaFiltro = 0;
  for (int i = 0; i < NUM_MUESTRAS_FILTRO; i++) {
    sumaFiltro += historialBateria[i];
  }
  float pctFiltrado = sumaFiltro / NUM_MUESTRAS_FILTRO;
  
  int pct = round(pctFiltrado);
  pctBateriaGlobal = pct;
  stringVoltaje = String(voltajePila, 2) + "V";
  
  if (pct != ultimo_porcentaje) {
    ultimo_porcentaje = pct;
  }
}

// ============ LECTURA DEL SENSOR ============

float medirDistancia() {
  if (sensorBloqueado) return 999;
  
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duracion = pulseIn(ECHO_PIN, HIGH, 20000);
  if (duracion == 0) return 999;
  
  float distancia = duracion * 0.0343 / 2.0;
  if (distancia < 2 || distancia > 400) return 999;
  
  return distancia;
}

float medirDistanciaSegura() {
  if (secuenciaObstaculoActiva) return ultimaDistanciaValida;
  
  unsigned long tiempoActual = millis();
  if (tiempoActual - ultimoIntentoSensor < 50) return ultimaDistanciaValida;
  
  ultimoIntentoSensor = tiempoActual;
  float distancia = medirDistancia();
  
  if (distancia != 999) {
    fallosConsecutivos = 0;
    ultimaDistanciaValida = distancia;
    sensorBloqueado = false;
    return distancia;
  }
  
  fallosConsecutivos++;
  if (fallosConsecutivos >= 5) {
    sensorBloqueado = true;
    fallosConsecutivos = 0;
  }
  
  if (sensorBloqueado) {
    if (tiempoActual - ultimoIntentoSensor > 3000) {
      sensorBloqueado = false;
      return medirDistanciaSegura();
    }
    return ultimaDistanciaValida;
  }
  
  return ultimaDistanciaValida;
}

// ============ SETUP ============

void setup() {
  Serial.begin(115200);
  delay(1000);
  randomSeed(esp_random());
  
  secuenciaObstaculoActiva = false;
  direccionAleatoria = -1;
  direccionBloqueada = false;
  sonidoReproducido = false;
  retrocesoCompletado = false;
  sonidoTerminado = false;
  giroCompletado = false;

  intentoConexion("hachi", "secodepollo81");

  Wire.setClock(100000);
  Wire.begin(21, 22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pca.begin();
  delay(100);
  pca.setPWMFreq(50);
  delay(200);
  detenerServos();

  mySerial.begin(9600, SERIAL_8N1, 16, 17);
  delay(500);
  if (player.begin(mySerial)) {
    player.volume(30);
  }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(VIBRA_PIN, OUTPUT);
  digitalWrite(VIBRA_PIN, LOW);
  
  analogReadResolution(12);
  pinMode(PIN_VOLTAJE, INPUT);

  webSocket.begin(serverWS_IP, serverWS_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(2000);
  webSocket.setExtraHeaders("User-Agent: ESP32-Robot");

  nextBlink = random(2500, 7000);
  menuVisible = true;
  lastBaileTime = millis();
  tiempoInicioBaile = millis();
  mostrarMenu();
  
  ultimoIntentoSensor = millis();
  ultimaDistanciaValida = 999;
  sensorBloqueado = false;
  fallosConsecutivos = 0;
}

// ============ LOOP ============

void loop() {
  loopAP();
  webSocket.loop();
  
  if (menuVisible) {
    mostrarMenu();
    delay(100);
    return;
  }
  
  if (millis() - lastBateriaTime > 2000) {
    lastBateriaTime = millis();
    actualizarBateria();
  }
  
  if (modoActual == 1) {
    if (!secuenciaObstaculoActiva) {
      distanciaFiltroActual = medirDistanciaSegura();
    }
    float distancia = distanciaFiltroActual;
    
    if (sensorBloqueado && !secuenciaObstaculoActiva) {
      updateAnimation();
      drawDogFace();
      display.display();
      delay(50);
    }
    
    if (!secuenciaObstaculoActiva && distancia < 20 && distancia > 2) {
      secuenciaObstaculoActiva = true;
      sonidoReproducido = false;
      retrocesoCompletado = false;
      sonidoTerminado = false;
      giroCompletado = false;
      estaRetrocediendo = false;
      pasoMovimiento = 0;
      
      emotion = SURPRISED;
      vibracionLarga();
      detenerServos();
      delay(200);
      
      if (direccionAleatoria == -1) {
        direccionAleatoria = random(0, 2);
        direccionBloqueada = true;
      }
      
      sonidoReproducido = true;
      tiempoInicioSonidoDir = millis();
      
      if (direccionAleatoria == 0) {
        cambiarMusica(3);
      } else {
        cambiarMusica(4);
      }
    }
    
    if (secuenciaObstaculoActiva) {
      if (!retrocesoCompletado) {
        if (!estaRetrocediendo) {
          estaRetrocediendo = true;
          pasoMovimiento = 0;
          lastMovimientoTime = millis();
        }
        
        if (estaRetrocediendo) {
          retroceder();
          if (pasoMovimiento >= 7) {
            estaRetrocediendo = false;
            pasoMovimiento = 0;
            detenerServos();
            delay(200);
            retrocesoCompletado = true;
          }
        }
      }
      
      if (retrocesoCompletado && !sonidoTerminado) {
        if (millis() - tiempoInicioSonidoDir > duracionSonidoDir) {
          sonidoTerminado = true;
          tiempoInicioGiro = millis();
          pasoMovimiento = 0;
          lastMovimientoTime = millis();
          cambiarMusica(4);
        }
      }
      
      if (sonidoTerminado && !giroCompletado) {
        if (direccionAleatoria == 0) {
          girarDerecha();
        } else {
          girarIzquierda();
        }
        
        if (millis() - tiempoInicioGiro > duracionGiro) {
          giroCompletado = true;
          pasoMovimiento = 0;
          detenerServos();
          delay(200);
        }
      }
      
      if (giroCompletado) {
        secuenciaObstaculoActiva = false;
      }
      
      updateAnimation();
      drawDogFace();
      display.display();
      delay(100);
    }
    
    if (!secuenciaObstaculoActiva) {
      if (distancia > 50 && direccionAleatoria != -1) {
        direccionAleatoria = -1;
        direccionBloqueada = false;
      }
      
      if (distancia == 999 && !sensorBloqueado) {
        updateAnimation();
        drawDogFace();
        display.display();
        delay(50);
      } else {
        if (distancia >= 20 && distancia <= 50) {
          if (emotion != SAD) {
            emotion = SAD;
            vibrar(200);
            reproducirLadrido();
          }
          ejecutarMovimiento();
        }
        else if (distancia > 50) {
          if (emotion != HAPPY) {
            emotion = HAPPY;
            cambiarMusica(1);
          }
          ejecutarMovimiento();
        }
      }
      
      updateAnimation();
      drawDogFace();
      display.display();
      delay(100);
    }
  }
  
  else if (modoActual == 2) {
    updateAnimation();
    animarManitas();
    
    if (millis() - lastEmotionChange > 4000) {
      lastEmotionChange = millis();
      int r = random(0, 3);
      if (r == 0 || r == 1) {
        emotion = HAPPY;
      } else {
        emotion = SURPRISED;
      }
    }
    
    if (!mostrandoFrase && (millis() - lastFraseTime > 10000)) {
      mostrarFraseAleatoria();
    }
    
    if (mostrandoFrase && (millis() - lastFraseTime > fraseDuration)) {
      mostrandoFrase = false;
    }
    
    if (millis() - tiempoInicioCancion > duracionCancion) {
      iniciarMusicaAleatoria();
    }
    
    if (millis() - lastBaileTime > 8000) {
      lastBaileTime = millis();
      int baileAleatorio = random(0, 4);
      
      if (baileAleatorio == 0) {
        bailarAcostarse();
      } else if (baileAleatorio == 1) {
        bailarOrinar();
      } else if (baileAleatorio == 2) {
        bailarSaludar();
      } else {
        bailarSentarseMano();
      }
    }
    
    drawDogFace();
    display.display();
    delay(100);
  }
  
  if (millis() - lastTelemetryTime > telemetryInterval) {
    lastTelemetryTime = millis();
    enviarTelemetriaWeb();
  }
}

// ============ FUNCIONES ============

void vibrar(int duracionMs) {
  digitalWrite(VIBRA_PIN, HIGH);
  delay(duracionMs);
  digitalWrite(VIBRA_PIN, LOW);
}

void vibracionLarga() {
  digitalWrite(VIBRA_PIN, HIGH);
  delay(300);
  digitalWrite(VIBRA_PIN, LOW);
  delay(100);
  digitalWrite(VIBRA_PIN, HIGH);
  delay(300);
  digitalWrite(VIBRA_PIN, LOW);
}

// 🔥 MOSTRAR FRASE SEGÚN EL GÉNERO SELECCIONADO
void mostrarFraseAleatoria() {
  int idx = random(0, numFrases);
  
  if (generoUsuario == "M") {
    fraseActual = frasesMasculino[idx];
  } else {
    fraseActual = frasesFemenino[idx];
  }
  
  mostrandoFrase = true;
  fraseDuration = 3000;
  lastFraseTime = millis();
  Serial.print("Frase (");
  Serial.print(generoUsuario);
  Serial.print("): ");
  Serial.println(fraseActual);
}

// ============ ANIMACIONES ============

void animarManitas() {
  if (millis() - lastManitaAnim > 150) {
    lastManitaAnim = millis();
    pasoManitas++;
    if (pasoManitas > 5) pasoManitas = 0;
    
    switch (pasoManitas) {
      case 0: manitaIzqY = -3; manitaDerY = 3; break;
      case 1: manitaIzqY = 0;  manitaDerY = 0; break;
      case 2: manitaIzqY = 3;  manitaDerY = -3; break;
      case 3: manitaIzqY = 0;  manitaDerY = 0; break;
      case 4: manitaIzqY = -2; manitaDerY = 2; break;
      case 5: manitaIzqY = 2;  manitaDerY = -2; break;
    }
  }
}

void updateAnimation() {
  if (millis() - lastLookMove > 2000) {
    targetX = random(-8, 9);
    targetY = random(-4, 5);
    lastLookMove = millis();
  }
  
  eyeX += (targetX - eyeX) * 0.05;
  eyeY += (targetY - eyeY) * 0.05;
  
  if (!blinking && millis() - lastBlink > nextBlink) {
    blinking = true;
    blinkLevel = 0;
    lastBlink = millis();
    nextBlink = random(2500, 7000);
  }
  
  if (blinking) {
    blinkLevel += 3;
    if (blinkLevel > 24) {
      blinking = false;
      blinkLevel = 0;
    }
  }
}

// ============ OLED ============

void mostrarMenu() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("MENU");
  display.setTextSize(1);
  display.setCursor(10, 35);
  display.println("1. Asistente");
  display.setCursor(10, 50);
  display.println("2. Depresion");
  display.display();
}

void drawEarLeft() {
  display.fillTriangle(5, 12, 20, 2, 28, 20, SSD1306_WHITE);
}

void drawEarRight() {
  display.fillTriangle(123, 12, 108, 2, 100, 20, SSD1306_WHITE);
}

void drawEye(int cx, int cy, bool leftEye) {
  int w = 36;
  int h = 28;
  if (emotion == SURPRISED) h = 34;
  if (emotion == SLEEPY) h = 12;
  
  display.fillRoundRect(cx - w/2, cy - h/2, w, h, 10, SSD1306_WHITE);
  
  int lid = blinkLevel;
  if (emotion == HAPPY) lid += 6;
  if (emotion == SLEEPY) lid += 12;
  
  if (lid > 0) {
    display.fillRect(cx - w/2, cy - h/2, w, lid, SSD1306_BLACK);
  }
  
  int px = cx + eyeX;
  int py = cy + eyeY;
  
  display.fillCircle(px, py, 9, SSD1306_BLACK);
  display.fillCircle(px - 3, py - 3, 3, SSD1306_WHITE);
  display.fillCircle(px + 2, py + 2, 1, SSD1306_WHITE);
  
  switch (emotion) {
    case HAPPY:
      display.drawLine(cx - 15, cy - 20, cx + 15, cy - 16, SSD1306_WHITE);
      break;
    case SAD:
      if (leftEye)
        display.drawLine(cx - 15, cy - 16, cx + 15, cy - 22, SSD1306_WHITE);
      else
        display.drawLine(cx - 15, cy - 22, cx + 15, cy - 16, SSD1306_WHITE);
      break;
    case SURPRISED:
      display.drawLine(cx - 15, cy - 24, cx - 5, cy - 20, SSD1306_WHITE);
      display.drawLine(cx + 15, cy - 24, cx + 5, cy - 20, SSD1306_WHITE);
      break;
    default: break;
  }
}

void drawNose() {
  display.fillTriangle(60, 48, 68, 48, 64, 54, SSD1306_WHITE);
}

void drawDogFace() {
  display.clearDisplay();
  
  if (modoActual == 2 && mostrandoFrase) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(fraseActual, 0, 0, &x1, &y1, &w, &h);
    int x = (SCREEN_WIDTH - w) / 2;
    int y = (SCREEN_HEIGHT - h) / 2;
    
    display.setCursor(x, y);
    display.print(fraseActual);
    display.display();
    return;
  }
  
  drawEarLeft();
  drawEarRight();
  drawEye(40, 32, true);
  drawEye(88, 32, false);
  drawNose();
  
  if (emotion == HAPPY) {
    display.drawLine(56, 56, 64, 60, SSD1306_WHITE);
    display.drawLine(64, 60, 72, 56, SSD1306_WHITE);
    display.fillCircle(32, 28, 2, SSD1306_WHITE);
    display.fillCircle(80, 28, 2, SSD1306_WHITE);
    display.fillCircle(20, 40, 4, SSD1306_WHITE);
    display.fillCircle(108, 40, 4, SSD1306_WHITE);
  }
  
  if (emotion == SAD) {
    display.drawLine(56, 60, 64, 56, SSD1306_WHITE);
    display.drawLine(64, 56, 72, 60, SSD1306_WHITE);
  }
  
  if (emotion == SURPRISED) {
    display.drawCircle(64, 58, 4, SSD1306_WHITE);
    display.drawCircle(64, 58, 6, SSD1306_WHITE);
  }
  
  if (emotion == SLEEPY) {
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(108, 50);
    display.print("Z");
    display.setCursor(116, 42);
    display.print("z");
  }
  
  if (modoActual == 2) {
    int izqX = 8;
    int izqY = 30 + manitaIzqY;
    display.fillCircle(izqX, izqY, 3, SSD1306_WHITE);
    display.fillCircle(izqX - 5, izqY + 7, 3, SSD1306_WHITE);
    display.fillCircle(izqX + 5, izqY + 7, 3, SSD1306_WHITE);
    
    int derX = 120;
    int derY = 30 + manitaDerY;
    display.fillCircle(derX, derY, 3, SSD1306_WHITE);
    display.fillCircle(derX - 5, derY + 7, 3, SSD1306_WHITE);
    display.fillCircle(derX + 5, derY + 7, 3, SSD1306_WHITE);
  }
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  String texto = "";
  if (modoActual == 1) {
    switch (emotion) {
      case HAPPY: texto = "FELIZ"; break;
      case SAD: texto = "TRISTE"; break;
      case SURPRISED: texto = "SORPRESA"; break;
      default: texto = "MODO ASISTENTE"; break;
    }
  } else if (modoActual == 2) {
    texto = "BAILANDO";
  }
  
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(texto, 0, 0, &x1, &y1, &w, &h);
  int x = (SCREEN_WIDTH - w) / 2;
  
  display.setCursor(x, 0);
  display.print(texto);
  
  display.display();
}