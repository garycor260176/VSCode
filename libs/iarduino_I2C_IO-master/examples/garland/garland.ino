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
  extIO.pinMode(0, OUTPUT);                                 // Переводим 0 вывод модуля в режим выход
  extIO.pinMode(1, OUTPUT);                                 // Переводим 1 вывод модуля в режим выход
  extIO.pinMode(2, OUTPUT);                                 // Переводим 2 вывод модуля в режим выход
  extIO.pinMode(3, OUTPUT);                                 // Переводим 3 вывод модуля в режим выход
  extIO.pinMode(4, OUTPUT);                                 // Переводим 4 вывод модуля в режим выход
  extIO.pinMode(5, OUTPUT);                                 // Переводим 5 вывод модуля в режим выход
  extIO.pinMode(6, OUTPUT);                                 // Переводим 6 вывод модуля в режим выход
  extIO.pinMode(7, OUTPUT);                                 // Переводим 7 вывод модуля в режим выход
}                                                           //
                                                            //
void loop(){                                                //
  switch((millis()/100)%16){                                //
    case 0x0: extIO.digitalWrite(0, HIGH); break;           // Устанавливаем на 0 выводе модуля , состояние логической  1
    case 0x1: extIO.digitalWrite(0, LOW ); break;           // Устанавливаем на 0 выводе модуля , состояние логического 0
    case 0x2: extIO.digitalWrite(1, HIGH); break;           // Устанавливаем на 1 выводе модуля , состояние логической  1
    case 0x3: extIO.digitalWrite(1, LOW ); break;           // Устанавливаем на 1 выводе модуля , состояние логического 0
    case 0x4: extIO.digitalWrite(2, HIGH); break;           // Устанавливаем на 2 выводе модуля , состояние логической  1
    case 0x5: extIO.digitalWrite(2, LOW ); break;           // Устанавливаем на 2 выводе модуля , состояние логического 0
    case 0x6: extIO.digitalWrite(3, HIGH); break;           // Устанавливаем на 3 выводе модуля , состояние логической  1
    case 0x7: extIO.digitalWrite(3, LOW ); break;           // Устанавливаем на 3 выводе модуля , состояние логического 0
    case 0x8: extIO.digitalWrite(4, HIGH); break;           // Устанавливаем на 4 выводе модуля , состояние логической  1
    case 0x9: extIO.digitalWrite(4, LOW ); break;           // Устанавливаем на 4 выводе модуля , состояние логического 0
    case 0xA: extIO.digitalWrite(5, HIGH); break;           // Устанавливаем на 5 выводе модуля , состояние логической  1
    case 0xB: extIO.digitalWrite(5, LOW ); break;           // Устанавливаем на 5 выводе модуля , состояние логического 0
    case 0xC: extIO.digitalWrite(6, HIGH); break;           // Устанавливаем на 6 выводе модуля , состояние логической  1
    case 0xD: extIO.digitalWrite(6, LOW ); break;           // Устанавливаем на 6 выводе модуля , состояние логического 0
    case 0xE: extIO.digitalWrite(7, HIGH); break;           // Устанавливаем на 7 выводе модуля , состояние логической  1
    case 0xF: extIO.digitalWrite(7, LOW ); break;           // Устанавливаем на 7 выводе модуля , состояние логического 0
    default:                               break;           //
  }                                                         //
}                                                           //