#define PULSE_PIN 5  // Пин D5, свързан с T1

volatile unsigned long pulseCount = 0;
volatile bool secondElapsed = false;
volatile unsigned int isrCounter = 0;

// Рутина за прекъсване на Таймер 2 в режим CTC
ISR(TIMER2_COMPA_vect) {
  isrCounter++;
  if (isrCounter >= 122) {  // 122 * 8.192 ms ~= 1000 ms
    // Временно спиране на броенето на импулси, за да прочетем TCNT1
    TCCR1B = 0;

    _delay_us(3515);
    // Прочитане на 16-битовата стойност на брояча
    pulseCount = TCNT1;

    // Нулиране на брояча за следващата секунда

    TCNT1 = 0;

    // Възобновяване на броенето на импулси
    TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10);

    isrCounter = 0;
    secondElapsed = true;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Високопрецизен брояч на импулси стартиран.");

  // Настройка на пин D5 (T1) като вход
  pinMode(PULSE_PIN, INPUT);

  // Конфигуриране на Таймер 1 като външен брояч
  TCCR1A = 0;
  TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10);
  TCNT1 = 0;

  // Конфигуриране на Таймер 2 в режим CTC
  TCCR2A = 0;  // Нулираме регистър A
  TCCR2B = 0;  // Нулираме регистър B

  TCCR2A |= (1 << WGM21);                             // Задаваме режим CTC
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  // Задаваме предделител 1024

  // Задаваме стойността на Compare Match регистъра (OCR2A)
  // Изчисление: (16000000 / 1024 / 122) - 1 = ~127.
  OCR2A = 127;

  // Активиране на прекъсването при сравнение на Timer 2
  TIMSK2 |= (1 << OCIE2A);

  // Разрешаване на глобалните прекъсвания
  sei();
}

#define BUFF_LEN 16

uint32_t average = 0;
uint16_t buffer[BUFF_LEN];
uint8_t index = 0;


void loop() {
  if (secondElapsed) {
    average -= buffer[index];
    average += pulseCount;
    buffer[index] = pulseCount;


    Serial.print("Импулси в последната секунда: ");
    Serial.print(pulseCount);
    Serial.print(" среден брой за последните ");
    Serial.print(BUFF_LEN);
    Serial.print(" секунди: ");
    Serial.println(average / BUFF_LEN);

    index++;
    if (index >= BUFF_LEN) {
      index = 0;
    }
    secondElapsed = false;
  }
}