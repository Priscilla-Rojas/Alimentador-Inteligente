#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <Servo.h>
#include "HX711.h"

#define DT_PIN A1     // DT de HX711 -> pin A1
#define SCK_PIN A2    // SCK de HX711 -> pin A2

HX711 celda;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// TECLADO 
const byte FILAS = 4;
const byte COLUMNAS = 4;
char arrayPins[FILAS][COLUMNAS] = {  
  {'1', '2', '3', 'A'},  
  {'4', '5', '6', 'B'},  
  {'7', '8', '9', 'C'},  
  {'*', '0', '#', 'D'}
};
// Pines del keypad 
byte arrayFilas[] = {9, 8, 7, 6};
byte arrayColumnas[] = {5, 4, 3, 2};

Keypad teclado = Keypad(makeKeymap(arrayPins), arrayFilas, arrayColumnas, FILAS, COLUMNAS);

// ACTUADORES / SENSORES
const int pinServo = 10;       // PWM para servo
const int trigPin = 12;        //pin trigger, ENVIA EL PULSO
const int echoPin = 11;        //pin echo, RECIBE EL PULSO
const int pinInfrarojo = 13;   // Sensor infrarrojo
const int pinValvula = A0;     // Control válvula
const int pinBoya = A3;        // Sensor boya

// Configuración
unsigned int intervaloHoras;
unsigned int cantidadGramos;

Servo miServo; // Creamos el objeto servo para controlar el servomotor

bool servir = false;
bool modificarConfig = false;
float escalaCelda = 1.0;
//const float KNOWN_WEIGHT = 100.0; // AJUSTAR según peso de calibración (g)
unsigned long ultimaComida = 0;
const unsigned long tiempoDeInactividad = 20000;
unsigned long ultimaActividad = 0;
bool menuActive = false;
float distancia;
float tiempo;

const int ADDR_INTERVALO = 0; // Posicion en memoria
const int ADDR_CANTIDAD = 10;
const int ADDR_ESCALA = 20;

int leerNumero(const char* mensaje) {
  String numerosIngresados = "";
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(mensaje);
  lcd.setCursor(0,1);
  Serial.println("Tecla presionada");
  while (true) {
    char teclaPresionada = teclado.getKey();
    if (teclaPresionada) {
      if (teclaPresionada >= '0' && teclaPresionada <= '9') {
        numerosIngresados += teclaPresionada;
        lcd.print(teclaPresionada);
      } else if (teclaPresionada == 'A') {
        if (numerosIngresados.length() > 0) {
          return numerosIngresados.toInt();
        }
      } else if (teclaPresionada == 'B') {
        if (numerosIngresados.length() > 0) {
          numerosIngresados.remove(numerosIngresados.length()-1); // Hola -> 4 len indice-> 3
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print(mensaje);
          lcd.setCursor(0,1);
          lcd.print(numerosIngresados);
        }
      }
    }
  }
}

// --- funciones de peso / servir
void servirComida(int pesoDeseado, float pesoActual){
  unsigned long t0 = millis();
  while (!celda.is_ready()) {
    if (millis() - t0 > 2000) break;
    delay(10);
  }

  if (!celda.is_ready()) { //Verificamos si la celda esta lista para usarse.
    lcd.clear();
    lcd.print("Error balanza");
    Serial.println("No se puede servir: HX711 no listo");
    delay(1500);
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("En proceso");
  Serial.println("Sirviendo");
  miServo.write(19);
  // delay(500);
  
  unsigned long tStart = millis();
  float pesoFormula = pesoDeseado - 40;
  
  while(pesoActual < pesoFormula){
    //Verifica que no sirva por mas de 30s
    if (millis() - tStart > 30000UL) { //Provar sino sacar
      Serial.println("Timeout sirviendo: detener.");
      lcd.clear(); 
      lcd.print("Verificar Deposito");
      break;
    }

    pesoActual = celda.get_units(10);
    Serial.print("Pesando... ");
    Serial.println(pesoActual);
    if (pesoActual >= pesoFormula) break;
    delay(100);
  }

  miServo.write(-5); // Regresa al origen
  delay(1000);
  lcd.clear();
  lcd.print("Comida Lista");
  Serial.print("Comida Lista");
  delay(4000);
  lcd.clear();
  servir = false;
}

void verificarPeso(){
  float pesoActual = celda.get_units(10); // promedio de 10 lecturas
  Serial.print("Peso actual: ");
  Serial.println(pesoActual);
  if (pesoActual < cantidadGramos) {
    Serial.println("Sirviendo...");
    servirComida(cantidadGramos, pesoActual);
  } else {
    Serial.println("Plato completo.");
  }
}

void configuracion(){
  intervaloHoras = leerNumero("Intervalo (hs):");
  cantidadGramos = leerNumero("Cantidad (g):");
  EEPROM.put(ADDR_INTERVALO, intervaloHoras);
  EEPROM.put(ADDR_CANTIDAD, cantidadGramos);
  lcd.clear();
  lcd.print("Guardado!");
  delay(1500);
  lcd.clear();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando sistema");
  
  // Pines
  pinMode(pinValvula, OUTPUT);
  digitalWrite(pinValvula, LOW); // cerrar por defecto
  pinMode(pinBoya, INPUT_PULLUP);
  pinMode(pinInfrarojo, INPUT);
  pinMode(trigPin, OUTPUT); //de salida (envia pulso)
  digitalWrite(trigPin, LOW); //tener apagado el trigger
  pinMode(echoPin, INPUT);//de entrada (recibe el pulso)

  miServo.attach(pinServo);
  miServo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Iniciando Pantalla");

  // Leer EEPROM (no sobrescribir aquí)
  EEPROM.get(ADDR_INTERVALO, intervaloHoras);
  EEPROM.get(ADDR_CANTIDAD, cantidadGramos);

  if (intervaloHoras <= 0 || intervaloHoras > 12 || cantidadGramos <= 0 || cantidadGramos > 500) {
    configuracion();
  } else {
    Serial.print("Intervalo leido EEPROM: "); 
    Serial.println(intervaloHoras);
    Serial.print("Cantidad leida EEPROM: "); 
    Serial.println(cantidadGramos);
  }

  //CELDA DE CARGA
  // HX711 inicialización
  // Serial.println("Inicializando HX711 en pines A1 (DT) / A2 (SCK) ...");
  // celda.begin(DT_PIN, SCK_PIN);

  // unsigned long t0 = millis();
  // while (!celda.is_ready()) {
  //   if (millis() - t0 > 2000) break;
  //   delay(10);
  // }

  // if (!celda.is_ready()) {
  //   Serial.println("ATENCION: HX711 NO listo. Revisar cableado y alimentacion.");
  // } else {
  //   Serial.println("HX711 listo. Tare y calibracion...");
  //   celda.set_scale(); // temporal
  //   celda.tare();
  //   delay(500);
  //   Serial.println("Coloca peso conocido (ej. 100g) en 10s...");
  //   lcd.clear(); 
  //   lcd.print("Calibrando");
  //   delay(10000);

  //   const int N = 10;
  //   float lecturaProm = celda.get_units(N);
  //   Serial.print("Lectura promedio cruda: "); 
  //   Serial.println(lecturaProm, 6);

  //   if (KNOWN_WEIGHT <= 0.0) {
  //     Serial.println("KNOWN_WEIGHT no valido. Ajustarlo.");
  //   } else if (lecturaProm <= 0.0) {
  //     Serial.println("Lectura cruda invalida; calibracion fallida.");
  //   }else {
  //     escalaCelda = lecturaProm / KNOWN_WEIGHT;
  //     Serial.print("Factor calibracion: "); 
  //     Serial.println(escalaCelda, 6);
  //     celda.set_scale(escalaCelda);
  //     EEPROM.put(ADDR_ESCALA, escalaCelda);
  //     float peso_calculado = celda.get_units(N);
  //     Serial.print("Peso calculado despues de set_scale: "); 
  //     Serial.println(peso_calculado, 0);
  //   }
  // }

  //CONFIGURACION FINAL CELDA

  //Despues de que la calibracion haya sido exitosa:
  // Primero verifico que tenga el valor de la escala guaradada (Descomentar): 
  // EEPROM.get(ADDR_ESCALA, escalaCelda);
  // Serial.println("Escala guardada: ");
  // Serial.println(escalaCelda);


  //Si lo de arriba no me da error, borrar o comentar la calibracion y solo dejar lo abajo activo
  //(descomenatar lo de abajo):
  Serial.println("Probando Balanza...");
  celda.begin(DT_PIN, SCK_PIN);
  EEPROM.get(ADDR_ESCALA, escalaCelda);
  celda.set_scale(escalaCelda);
  celda.tare();

  ultimaActividad = millis();
  ultimaComida = millis();

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Sistema Listo!");
  Serial.println("Sist. completamente Config.");
  delay(3000);
  lcd.clear();
  lcd.noBacklight();
}

// loop principal: control de riego/servir/autodiagnóstico
void loop() {
  //Servir comida de forma AUTOMATICA
  unsigned long tiempoActual = millis();
  unsigned long intervaloMs = (unsigned long)intervaloHoras * 60000UL; //Minutos
  // unsigned long intervaloMs = (unsigned long)intervaloHoras * 3600000UL; //Horas
  
  if (intervaloMs > 0 && tiempoActual - ultimaComida >= intervaloMs) {
    float pesoActual = celda.get_units(10); // Leer peso actual del plato
    if (pesoActual < cantidadGramos) verificarPeso();
    ultimaComida = tiempoActual; // Actualizar la última comida
  }

  char teclaPresionada = teclado.getKey();

  if (teclaPresionada) {
    lcd.backlight();
    ultimaActividad = millis();
    menuActive = true;
  }

  // CONFIGURACION TECLADO
  if (teclaPresionada == 'D') {
    menuActive = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Desea servir comida: ");
    lcd.setCursor(0, 1);
    lcd.print("A:Si B:NO ");
    delay(200);
    servir = true;

  } else if (teclaPresionada == 'A' && servir) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Iniciando...");
    delay(300);
    verificarPeso();
    menuActive = false;
    ultimaComida = tiempoActual; // Actualizar la última comida
    lcd.clear();

  } else if (teclaPresionada == 'A' && modificarConfig) {
    lcd.clear();
    configuracion();
    modificarConfig = false;
    menuActive = false;
    lcd.clear();

  } else if (teclaPresionada == 'B' && (servir || modificarConfig)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Saliendo");
    delay(800);
    lcd.clear();
    servir = false;
    modificarConfig = false;
    menuActive = false;

  } else if (teclaPresionada == '*') {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Intervalo: " + String(intervaloHoras) + "hs");
    lcd.setCursor(0,1);
    lcd.print( "Cantidad: " + String(cantidadGramos) + "g");
    delay(2000);
    lcd.clear();

  } else if (teclaPresionada == '#') {
    menuActive = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit config");
    lcd.setCursor(0, 1);
    lcd.print("A:Si B:NO");
    modificarConfig = true;
  }

  //BOYA - controla valvula
  int estadoBoya = digitalRead(pinBoya); 

  if (estadoBoya == LOW) {  // Agua faltante 
    digitalWrite(pinValvula, HIGH); // Abre válvula
    if (!menuActive) {
      lcd.clear();
      lcd.backlight();
      lcd.setCursor(0,0);
      lcd.print("Cargando Agua");
      delay(3000);
    }
  }else {
    digitalWrite(pinValvula, LOW);
  }

  // Sensor DEPOSITO: comida baja (infrarrojo)
  int sensorComidaBaja = digitalRead(pinInfrarojo);
  if (sensorComidaBaja == HIGH) {
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Deposito bajo");
    lcd.setCursor(0, 1);
    lcd.print("Cargar Comida");
    delay(3000);
    lcd.clear();
  }

  // agua_bajo();
  digitalWrite(trigPin, HIGH); //generar pulso
  delayMicroseconds (10); //tarda 10 microsegundos segun el datasheet
  digitalWrite(trigPin,LOW);
  tiempo = pulseIn(echoPin,HIGH); //pulseIn devuelve un vialor de tiempo en microsegundos, cuanto tarda la señal en ir y volver
  distancia = tiempo / 58.77; //calcular la distancia usando la formula del datasheet
  
  if (distancia > 12){
    Serial.println(distancia);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("DEPOSITO BAJO");
    lcd.setCursor(0,1);
    lcd.print("Cargar Agua");
    delay(3000);
  }
  
   // Apagar backlight si inactividad (opcional)
  if (millis() - ultimaActividad > tiempoDeInactividad) {
    lcd.clear(); //Limpia la pantalla
    lcd.noBacklight(); // Apaga la luz de fondo
    menuActive = false;
  }
}
