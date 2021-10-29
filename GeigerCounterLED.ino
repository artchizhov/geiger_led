#define LOG_PERIOD        3000  // интервал времени замера (миллисекунды)
#define EXPO_FILTER_K     0.1   // коэффициент экспоненциального сглаживания

// Выводы Arduino для светодиодов (D5, D6, D7, D8, D9, D10, D11, D12)
// Младший (низкое значение уровня радиации) - LED0
// Старший (высокое значение уровня радиации) - LED7
#define LED0 5
#define LED1 6
#define LED2 7
#define LED3 8
#define LED4 9
#define LED5 10
#define LED6 11
#define LED7 12

unsigned long INT_counts;       // кол-во импульсов за интервал (периодически обнуляется)
unsigned long cpm;              // CPM - колличество импульсов в минуту.

unsigned long runTime;          // время работы
unsigned long nextPeriodTime;   // время следующего периода расчетов
unsigned long startMillis;      // нчальное время для отсчета интервалов (миллисекунды)

float cpm_multiplier;           // расчет CPM из кол-ва импульсов за период.

// Обработчик прерывания.
// Прерывание вызывается спадом на контакте D2 Arduino.
// Собирает импульсы со счетчика
void tube_impulse() {
  INT_counts++;   // при каждом срабатывании счетчика, эта переменная увеличивается на +1.
}

// Настройка
void setup() {
  // Установка начальных значений
  INT_counts = 0;       // импульсов сначала нет
  cpm = 0;              // импульсов за минуту сначала = 0

  runTime         = 0;  // время работы при включении = 0
  nextPeriodTime  = 0;  // следующий период расчетов = 0, т.е. при включении надо сразу начинать считать

  // мультипликатор (множитель) для перевода "импульс за период" в "импульс за минуту"
  // (1000 мс в 1 сек; 60 сек в 1 мин)
  cpm_multiplier = (float)60000 / (float)LOG_PERIOD;

  // Делаем все контакты со светодиодами "выходами".
  pinMode(LED0, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  pinMode(LED5, OUTPUT);
  pinMode(LED6, OUTPUT);
  pinMode(LED7, OUTPUT);

  // после настройки получаем "нулевое" время работы микроконтроллера.
  // Это будет точкой отсчета начала работы (время на настройки выше не будут учитываться в расчетах)
  startMillis = millis();

  // Задаем прерывание, описанное выше.
  attachInterrupt(0, tube_impulse, FALLING);
}

// Функция обновления светодиодной полосы согласно уровням радиации.
// Задается в виде значений CPM (импульсов в минуту)
// 20, 30, 40, 60, 80, 100, 140, 200
void showCpm(unsigned long val) {
  // если значение превышает указанное то светодиод - включается, иначе - выключается.
  if (val >= 20)  digitalWrite(LED0, HIGH); else digitalWrite(LED0, LOW);
  if (val >= 30)  digitalWrite(LED1, HIGH); else digitalWrite(LED1, LOW);
  if (val >= 40)  digitalWrite(LED2, HIGH); else digitalWrite(LED2, LOW);
  if (val >= 60)  digitalWrite(LED3, HIGH); else digitalWrite(LED3, LOW);
  if (val >= 80)  digitalWrite(LED4, HIGH); else digitalWrite(LED4, LOW);
  if (val >= 100) digitalWrite(LED5, HIGH); else digitalWrite(LED5, LOW);
  if (val >= 140) digitalWrite(LED6, HIGH); else digitalWrite(LED6, LOW);
  if (val >= 200) digitalWrite(LED7, HIGH); else digitalWrite(LED7, LOW);
}

// Цикл.
// Инструкуции выполняемые постоянно,
// сверху вниз,
// много раз пока контроллер включен.
void loop() {
  // при работе с прерываниями нельзя использовать переменную
  // которая может внезапно измениться (по прерыванию): могут быть ошибки в расчетах
  // для этого копируем значение один раз и используем копию.
  unsigned long cnts = INT_counts;

  // получаем текущее время
  // (для расчета разницы во времени между началом работы и текущим моментом)
  // так как цикл выполняется многократно - время работы будет каждый раз увеличиваться
  // ждем наступления новых интервалов и считаем знаения радиации, используя полученное здесь время.
  unsigned long currentMillis = millis();
  
  // общее время работы, расчитывается из разницы текущего времени (currentMillis) и времени начала работы (startMillis)
  runTime = currentMillis - startMillis;  // в миллисекундах

  // если время работы больше либо равно ожидаемому времени следующиего расчета то:
  if (runTime >= nextPeriodTime) {
    // производим расчеты и отображаем результаты

    // Експоненциальное сглаживание:
    // S_t = a*p + (1-a)*S_(t-1)
    // a    - EXPO_FILTER_K
    // p    - cnts * cpm_multiplier
    // S_t  - cpm (counts per minute, c/m)
    cpm = round(EXPO_FILTER_K * (cnts * cpm_multiplier) + (1 - EXPO_FILTER_K) * (float)cpm);

    // Вызываем функцию для отображения результатов измерений
    showCpm(cpm);

    // обнуляем кол-во имупльсов за интервал, для нового интервала времени.
    INT_counts = 0;

    // устанавливаем время следующих расчетов, что бы контроллер ожидал
    nextPeriodTime = runTime + LOG_PERIOD;
  }
}
