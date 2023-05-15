//  Раскомментируйте для программной реализации шины I2C:   //
//  #define pin_SW_SDA 3                                    // Назначение любого вывода Arduino для работы в качестве линии SDA программной шины I2C.
//  #define pin_SW_SCL 9                                    // Назначение любого вывода Arduino для работы в качестве линии SCL программной шины I2C.
//  Раскомментируйте для совместимости с большинством плат: //
//  #include <Wire.h>                                       // Библиотека iarduino_I2C_IO будет использовать методы и функции библиотеки Wire.
//  Ссылки для ознакомления:                                //
//  Подробная информация о подключении модуля к шине I2C:   // http://wiki.iarduino.ru/page/i2c_connection/
//  Подробная информация о функциях и методах библиотеки:   // http://wiki.iarduino.ru/page/rasshiritel-na-8-vhodov-vyhodov-trema-modul/
                                                            //
#include <iarduino_I2C_IO.h>                                // Подключаем библиотеку iarduino_I2C_IO для работы с модулем
iarduino_I2C_IO extIO(0x20);                                // Создаём объект extIO для работы с модулем, адрес которого указан в качестве параметра.
                                                            //
void setup(){                                               //
  extIO.begin();                                            // Инициируем раборту с модулем расширения выводов
  extIO.pinMode(0, OUTPUT);                                 // Переводим 0 вывод модуля  в режим выход
  extIO.pinMode(1, OUTPUT);                                 // Переводим 1 вывод модуля  в режим выход
  extIO.pinMode(2, OUTPUT);                                 // Переводим 2 вывод модуля  в режим выход
  extIO.pinMode(3, OUTPUT);                                 // Переводим 3 вывод модуля  в режим выход
  extIO.pinMode(4, INPUT);                                  // Переводим 4 вывод модуля  в режим вход
  extIO.pinMode(5, INPUT);                                  // Переводим 5 вывод модуля  в режим вход
  extIO.pinMode(6, INPUT);                                  // Переводим 6 вывод модуля  в режим вход
  extIO.pinMode(7, INPUT);                                  // Переводим 7 вывод модуля  в режим вход
}                                                           //
                                                            //
void loop(){                                                //
  extIO.digitalWrite(0, extIO.digitalRead(4));              // Копируем состояние с 4 вывода на 0 вывод модуля
  extIO.digitalWrite(1, extIO.digitalRead(5));              // Копируем состояние с 5 вывода на 1 вывод модуля
  extIO.digitalWrite(2, extIO.digitalRead(6));              // Копируем состояние с 6 вывода на 2 вывод модуля
  extIO.digitalWrite(3, extIO.digitalRead(7));              // Копируем состояние с 7 вывода на 3 вывод модуля
}                                                           //