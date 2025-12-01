# 🐾 TobyFood – Alimentador Automático para Mascotas

### Proyecto académico – Prácticas Profesionalizantes I (Tecnicatura en Análisis de Sistemas)

## 📸 Vista general del proyecto

(Insertá aquí una imagen del prototipo o el diagrama que me enviaste)

## 🚀 Descripción

TobyFood es un sistema automático para alimentar e hidratar mascotas mediante sensores, actuadores y programación configurable.
Incluye un menú en pantalla, almacenamiento persistente en EEPROM, alarmas, y control autónomo de comida y agua.

Este proyecto combina electrónica, programación embebida, interfaz física y documentación profesional.

## 🧩 Tecnologías y Componentes

### 💻 Software

* Lenguaje: Arduino C/C++
* Librerías utilizadas:
* HX711
* Servo
* Keypad
* LiquidCrystal_I2C
* EEPROM
  
### 🔧 Hardware utilizado

* Arduino UNO
* Celda de carga + módulo HX711
* Sensor ultrasónico HC-SR04
* Sensor infrarrojo (nivel de alimento)
* Sensor de boya (nivel del bebedero)
* Display LCD 16×2 (I2C)
* Teclado matricial 4×4
* Servo motor SG90
* Válvula solenoide
* Fuente 5V

## 📂 Estructura del repositorio

```
📁 TobyFood
 ├── TobyFood.ino              # Código principal del sistema
 ├── README.md                 # Este archivo
 ├── Manual-Usuario.pdf        # Manual para el operador
 ├── Manual-Tecnico.pdf        # Documento técnico detallado
 └── Diagramas/
     └── circuito.png        # Diagrama del circuito

```

## 🎮 Funcionalidades principales
### 🕒 1. Alimentación automática

- Sirve comida según el intervalo configurado (1 a 12 horas).
- Controla el peso mediante la celda de carga.
- Solo sirve si falta alimento en el plato.

### 🖐️ 2. Alimentación manual

* Presionar D en el teclado.
* Confirmar con A para comenzar.

### ⚖️ 3. Balanza (HX711)

* Lecturas promediadas (10 muestras).
* Calibración guardada en EEPROM.

### 💧 4. Sistema automático de agua

 * Sensor de boya → controla el nivel del plato.
  
 * Sensor ultrasónico → mide nivel del tanque principal.
  
 * Válvula solenoide → abre y cierra automáticamente.

### 🧠 5. Configuración persistente

#### Guardado en EEPROM:

* Intervalo entre comidas
* Cantidad de gramos
* Factor de calibración

### 🔔 6. Mensajes en pantalla (LCD)

> “En proceso…”

> “Comida lista”

> “Depósito bajo”

> “Cargar agua”

> “Error balanza”

> “Cargando agua”

## 📖 Manuales

📘 Manual de Usuario
📗 Manual Técnico

Se incluyen ambos documentos en PDF dentro del repositorio.

## 🔌 Instalación y uso
### 1️⃣ Clonar el repositorio
    git clone https://github.com/TU-USUARIO/TobyFood.git

### 2️⃣ Abrir el proyecto

Abrir TobyFood.ino con Arduino IDE.

### 3️⃣ Instalar librerías requeridas

Desde Sketch > Include Library > Manage Libraries

Buscar e instalar:
  - HX711
  - Keypad
  - LiquidCrystal_I2C
  - Servo

### 4️⃣ Conectar sensores y actuadores

  - Seguir el diagrama incluido.

### 5️⃣ Cargar el código al Arduino

  - Conectar a USB → Subir código.

### 6️⃣ Configurar

Al iniciar por primera vez pedirá:

- Intervalo (horas)
- Cantidad de alimento (gramos)

## 📊 Especificaciones técnicas

| Parámetro                    | Valor                                 |
| ---------------------------- | ------------------------------------- |
| Alimentación                 | 5V DC / 2A                            |
| Capacidad máxima de alimento | 500 g                                 |
| Intervalo programable        | 1–12 horas                            |
| Pantalla                     | LCD 16×2 (I2C)                        |
| Entrada                      | Teclado 4×4                           |
| Sensores                     | IR, ultrasónico, boya, celda de carga |
| Actuadores                   | Servo motor + válvula solenoide       |


## 🎓 Proyecto académico

Desarrollado para la materia Prácticas Profesionalizantes I
Tecnicatura en Análisis de Sistemas.

## 👩‍💻 Autor

### Priscilla K. Rojas

GitHub: https://github.com/Priscilla-Rojas

LinkedIn: https://www.linkedin.com/in/priscilla-k-rojas
