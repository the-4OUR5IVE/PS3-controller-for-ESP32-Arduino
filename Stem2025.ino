#include <Ps3Controller.h>
#include <ESP32Servo.h>

const int ENA = 25; // ШИМ для левого мотора
const int ENB = 26; // ШИМ для правого мотора
const int IN1 = 14; // Направление левого мотора
const int IN2 = 27;
const int IN3 = 13; // Направление правого мотора
const int IN4 = 12;

// Пин для ESC
const int ESC_PIN = 33;

// Пин для сервопривода
const int SERVO_PIN = 32;

// Константы для настроек
const int deadzone = 30; // Увеличенная зона нечувствительности стика
const int maxSpeed = 165; // Максимальная скорость двигателя (PWM)
const int escMin = 1000;  // Минимальная ширина импульса (в микросекундах)
const int escMax = 1100;  // Максимальная ширина импульса (в микросекундах)

// Углы для сервопривода
const int servoActiveAngle = 90; // Угол, на который будет поворачиваться сервопривод
const int servoRestAngle = 0;    // Угол, в который вернется сервопривод

// Объекты для управления
Servo esc;
Servo servo;

// Функция вызывается при подключении контроллера
void onConnect() {
  Serial.println("PS3 контроллер подключен!");
}

void setup() {
  Serial.begin(115200);

  // Настройка пинов
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Настройка ESC
  esc.attach(ESC_PIN, escMin, escMax); // Привязка ESC к пину 33 с указанием диапазона
  esc.writeMicroseconds(escMin);      // Установить минимальное значение для инициализации
  delay(2000); // Задержка для инициализации ESC

  // Настройка сервопривода
  servo.attach(SERVO_PIN);
  servo.write(servoRestAngle); // Установить начальный угол

  // Инициализация PS3 контроллера
  Ps3.begin("44:d8:32:c8:8d:1a"); // Замените MAC-адрес на свой
  Ps3.attach(onConnect);

  Serial.println("Соединение с PS3 контроллером...");
}

void loop() {
  if (Ps3.isConnected()) {
    // Проверяем, нажата ли кнопка X
    if (Ps3.data.button.cross) {
      Serial.println("Кнопка X нажата. Остановка программы...");
      stopMotors();             // Останавливаем двигатели
      esc.writeMicroseconds(escMin); // Останавливаем ESC
      servo.write(servoRestAngle);   // Возвращаем сервопривод в исходное положение
      while (true);             // Бесконечный цикл для остановки программы
    }

    // Чтение положения левого стика по осям
    int stickY = Ps3.data.analog.stick.ly; // Вперед/назад
    int stickX = Ps3.data.analog.stick.lx; // Влево/вправо (повороты)

    // Управление движением вперед/назад
    if (abs(stickY) > deadzone && abs(stickX) < deadzone) {
      if (stickY > 0) {
        moveBackward(map(stickY, deadzone, 127, 0, maxSpeed)); // Стик назад
      } else {
        moveForward(map(-stickY, deadzone, 128, 0, maxSpeed)); // Стик вперед
      }
    } 
    // Управление поворотами
    else if (abs(stickX) > deadzone && abs(stickY) < deadzone) {
      if (stickX > 0) {
        turnRight(map(stickX, deadzone, 127, 0, maxSpeed)); // Поворот вправо
      } else {
        turnLeft(map(-stickX, deadzone, 128, 0, maxSpeed)); // Поворот влево
      }
    } 
    // Если оба стика в пределах deadzone
    else {
      stopMotors();
    }

    // Управление ESC через правый стик (RY)
    int stickRY = Ps3.data.analog.stick.ry;

    // Преобразование положения стика RY в сигнал для ESC
    if (abs(stickRY) > deadzone) {
      int escSpeed = map(stickRY, -128, 127, escMin, escMax);
      esc.writeMicroseconds(escSpeed); // Установить скорость ESC
      Serial.print("ESC скорость: ");
      Serial.println(escSpeed);
    } else {
      esc.writeMicroseconds(escMin); // Остановить ESC
      Serial.println("ESC остановлен (в пределах deadzone)");
    }

    // Управление сервоприводом через кнопку L2
    if (Ps3.data.button.l2) {
      servo.write(servoActiveAngle); // Поворот сервопривода на заданный угол
      Serial.println("Сервопривод активирован (L2 зажата)");
    } else {
      servo.write(servoRestAngle); // Возврат в исходное положение
      Serial.println("Сервопривод отключен (L2 отпущена)");
    }
  }
}

// Функции управления моторами
void moveForward(int speed) {
  Serial.println("Едем вперед");
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward(int speed) {
  Serial.println("Едем назад");
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight(int speed) {
  Serial.println("Поворачиваем вправо");
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnLeft(int speed) {
  Serial.println("Поворачиваем влево");
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  Serial.println("Останавливаемся");
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
