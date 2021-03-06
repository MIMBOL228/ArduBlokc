/* Подключение библиотек*/

#include <SPI.h>                                   // библиотека для шины SPI
#include <Wire.h>                                  // библиотека для шины I2C
#include <Servo.h>                                 // библиотека для сервопривода
#include <MFRC522.h>                               // библиотека для модуля считывателя RFID
#include <LiquidCrystal_I2C.h>                     // библиотека для дисплея
#include <iarduino_RTC.h>                          // библиотека для часов

/* Объявлеине пинов*/
 
#define SS_PIN     10                              // Пин SS на RFID считывателе
#define RST_PIN    9                               // Пин RST на считывателе
#define servo_pin  6                               // Пин сервы
#define rled       A0                              // Пин красного цвета у RGB светодиона
#define gled       A1                              // Пин зелёного цвета у RGB светодиона
#define bled       A2                              // Пин синего   цвета у RGB светодиона
#define Zumer_pin  5                               // Пин активного зуммера
#define button_pin 7                               // Пин кнопки выхода
#define clk 4                                      // Пин CLK  на часах реального времени
#define dat 3                                      // Пин DAT  на часах реального времени
#define rst 2                                      // Пин RST  на часах реального времени

/* Создание объектов для модулей */
 
MFRC522 mfrc522(SS_PIN, RST_PIN);                  // Инициилизация RFID модуля
LiquidCrystal_I2C lcd(0x27,16,2);                  // Инициилизация дисплея
Servo servo;                                       // Инициилизация сервы
iarduino_RTC time(RTC_DS1302,rst,clk,dat);         // Инициилизация часов

/* Создание переменных*/
 
unsigned long uidDec, uidDecTemp;                  // для храниения номера метки в десятичном формате
String sec,f;
byte c;

/* Функциии*/
 
String checkCard(long uid){                        // Функция, возвращающая имя по ID карты
  switch (uid){                                    // Сравнивание ID, которое нам дали
    case 3504866949:                               // ID 2161025113
      return "Sasha";                              // Возпращаем имя "Sasha"
      break;                                       // Конец
    case 2789610488:                               // ID 2789610488
      return "Maksim";                             // Возпращаем имя "Maksim"
      break;                                       // Конец
    default:                                       // Если ничего не подходит
      return " ";                                  // Ничего не возвращаем
      break;                                       // Конец
  }
}

String checkPass(byte pass){
  if(pass == 1){
    return "Sasha";
  }else if(pass == 2){
    return "Maksim";
  }else{
    return " ";
  }
}


void setup() {                                     // Функция setup, которая выполняется первый раз при загрузке системы
  Serial.begin(9600);                              // Инициилизация Serial 
  SPI.begin();                                     // Инициализация SPI 
  time.begin();                                    // Инициализация часов реального времени
  mfrc522.PCD_Init();                              // Инициализация MFRC522 
  lcd.init();                                      // Инициилизация дисплея
  lcd.backlight();                                 // Включаем подсведку у дисплея
  servo.attach(servo_pin);                         // Обозначение пина сервы
  servo.write(0);                                  // Устанавливаем серву в закрытое сосотояние
  pinMode(Zumer_pin,OUTPUT);                       // Устанавливаем пин зуммера, как выход
  pinMode(button_pin,INPUT_PULLUP);                // Устанавливаем пин кнопки, как выход с подтяжкой
  led(200,255,0);                                  // Устанавливаем жёлтый цвет светодиода
  wait();                                          // Включаем режим ожидания
  //time.settime(0,27,13,27,9,20,7);
  Wire.begin();                // join i2c bus with address #4
}
void loop() {
  //Serial.println(time.gettime("d-m-Y, H:i:s, D")); // Выводим время (временно)
   
  Wire.requestFrom(8, 1);    // request 6 bytes from slave device #8
  
  while(0 < Wire.available()) // loop through all but the last
  {
    c = Wire.read(); // receive byte as a character
    Serial.println(c);
    delay(500);
  }
  if(c != "" && c != 0){
    if(f == "e"){
      close();
      wait();  
    }else{
      Serial.println(c);         // print the character
      String checkPas = checkPass(c);
      if(checkPas != " "){                         // Если она нам хоть что-то вернула
        open(checkPas);                                 // То, открываем
        delay(7000);                               // И ждём 3 секунды
      }else{                                       // Иначе
        close();                                   // Отклоняем попытку входа
      }
      wait();                                      // После этого, переходим в режим ожидания
      Serial.println(")");
      f = "";
    }
  }
  
  if(!digitalRead(button_pin)){                    // Если кнопка нажата
    open("good mood!");                            // Открываем замок, и на дисплее желаем хорошего настроения :)
    delay(7000);                                   // Ждём..
    wait();                                        // После этого, переходим в режим ожидания
  }
  if (mfrc522.PICC_IsNewCardPresent()) {           // Поиск новой метки
    if (mfrc522.PICC_ReadCardSerial()) {           // Выбор метки
 
      uidDec = 0;                                  // Обнуление переменной, вдруг она уже содержит что-то
      
      /* Выдача серийного номера метки */
      
        for (byte i = 0; i < mfrc522.uid.size; i++){
          uidDecTemp = mfrc522.uid.uidByte[i];
          uidDec = uidDec * 256 + uidDecTemp;
        }

      Serial.println("Card UID: ");                // Выводим UID метки в консоль.
      Serial.println(uidDec);                      // Выводим UID метки в консоль.

      sec = checkCard(uidDec);                     // Записываем в переменную sec имя пользователя карточки

      if(sec != " "){                                   // Если она нам вернула не не какой-то текст (каждый воцклецательный знак обозначает инвертирование)
        open(sec);                                 // То, открываем
        delay(7000);                               // И ждём 3 секунды
      }else{                                       // Иначе
        close();                                   // Отклоняем попытку входа
      }
      wait();                                      // После этого, переходим в режим ожидания
    }
  }
}

void zum(int del){                                 // Функция zum, которая принимает значение задержки
  digitalWrite(Zumer_pin,HIGH);                    // Подаём напряжение на контакт пищалки
  delay(del);                                      // Ждём то время, которое нам сказали
  digitalWrite(Zumer_pin,LOW);                     // Убираем напряжение с контакта пищалки
}

void open(String name){                            // Функция открытия двери
  lcdp("Hello,",5,name,0);                         // Приветствуем хозяина карточки
  lgreen();                                        // Радостно загораем зелёный цвет светодиода
  servo.write(90);                                 // Поворациваем серву на угол 90 градусов(Отпираем какой либо механизм: задвижку, поворациваем ключ и т.д.)
  zum(400);                                        // Делаем звуковой сигнал, открытие
}

void wait(){                                       // Функция ожидания
  lcdp("Waiting for card",0,"....",6);             // Пишем, что мы рады почувствовать рядом с считывателем карту (пишем, что мы ждём карту)
  lyellow();                                       // Загораем жёлтый светодиод
  servo.write(0);                                  // Устанавливаем серву в закрытое сосотояние
  zum(200);                                        // Делаем звуковой сигнал, закрытие
}

void close(){                                      // Функция, которая сообщает, что карточки нет в белом списке
  lcdp("ERROR!",5,"Unikown card",2);               // Пишем на дисплей, что карточка не имеет права открыть замок
  lred();                                          // Загораем красный светодиод
  zum(1500);                                       // Делаем звуковой сигнал, неправильная карточка
}

void lcdp(String a,int b,String c,int d){          // Функция, которая пишет на дисплее наши сообщения
  lcd.clear();                                     // Очищаем дисплей
  lcd.setCursor(b,0);                              // Ставим курсор на первую строку, и на тот символ, который нам указали
  lcd.print(a);                                    // Пишем то, что нам сказали писать на первой строке
  lcd.setCursor(d,1);                              // Ставим курсор на ворую строку, и на тот символ, который нам указали
  lcd.print(c);                                    // Пишем то, что нам сказали писать на второй строке
}
  
void led(int r,int g,int b){                       // Функция, которая по значениям RGB загорает светодиоды
  analogWrite(rled,r);                             // Красному светодиоду подаём аналоговый сигнал, значение которого нам передали
  analogWrite(gled,g);                             // Зелёному светодиоду подаём аналоговый сигнал, значение которого нам передали
  analogWrite(bled,b);                             // Жёлтому  светодиоду подаём аналоговый сигнал, значение которого нам передали
} 

void lred(){                                       // Функция , которая зажигает красный свет светодиода
  led(255,0,0);
}

void lgreen(){                                     // Функция , которая зажигает зелёный свет светодиода
  led(0,255,0);
}

void lyellow(){                                    // Функция , которая зажигает жёлтый свет светодиода
  led(200,255,0);
}
