[![](https://iarduino.ru/img/logo.svg)](https://iarduino.ru)[![](https://wiki.iarduino.ru/img/git-shop.svg?3)](https://iarduino.ru) [![](https://wiki.iarduino.ru/img/git-wiki.svg?2)](https://wiki.iarduino.ru) [![](https://wiki.iarduino.ru/img/git-lesson.svg?2)](https://lesson.iarduino.ru)[![](https://wiki.iarduino.ru/img/git-forum.svg?2)](http://forum.trema.ru)

# iarduino\_I2C\_IO

**This is a library for Arduino IDE. It allows to control [GPIO Expander](https://iarduino.ru/shop/Expansion-payments/rasshiritel-na-8-vhodov-vyhodov-trema-modul.html) made by iArduino.ru**

**Данная библиотека для Arduino IDE позвляет управлять [Расширительем выводов GPIO](https://iarduino.ru/shop/Expansion-payments/rasshiritel-na-8-vhodov-vyhodov-trema-modul.html).**

> Подробнее про установку библиотеки читайте в нашей [инструкции](https://wiki.iarduino.ru/page/Installing_libraries/).

> Подробнее про подключение к [Arduino UNO](https://iarduino.ru/shop/boards/arduino-uno-r3.html)/[Piranha UNO](https://iarduino.ru/shop/boards/piranha-uno-r3.html) читайте на нашей [wiki](https://wiki.iarduino.ru/page/rasshiritel-na-8-vhodov-vyhodov-trema-modul/)


| Модель | Ссылка на магазин |
|---|---|
| ![](https://wiki.iarduino.ru/img/resources/759/759.svg) | https://iarduino.ru/shop/Expansion-payments/rasshiritel-na-8-vhodov-vyhodov-trema-modul.html |

## Управление выводами модулей, через функции объекта библиотеки:

```C++
#include <iarduino_I2C_IO.h> // Подключаем библиотеку.
iarduino_I2C_IO ОБЪЕКТ( АДРЕС_МОДУЛЯ ); // Создаём объект (для каждого модуля, свой объект).
```

**Инициализация** 

```C++
ОБЪЕКТ.begin(); // Инициализация работы с модулем.
```

**Установка режима вывода** 

```C++
ОБЪЕКТ.pinMode( №_ВЫВОДА_МОДУЛЯ , РЕЖИМ ); // Установка режима работы вывода (вход или выход).
```

**Чтение логического уровня** 

```C++
ОБЪЕКТ.digitalRead( №_ВЫВОДА_МОДУЛЯ ); // Чтение логического состояния с вывода модуля.
```

**Установка логического уровня** 

```C++
ОБЪЕКТ.digitalWrite( №_ВЫВОДА_МОДУЛЯ , СОСТОЯНИЕ ); // Установка логического состояния на выводе.
```

## Управление выводами модулей, через стандартные функции:

```C++
#include <iarduino_I2C_IO_Global.h> // Подключаем библиотеку.
void setup(){ iarduino_I2C_IO_Global.begin(); } // Инициируем работу модулей.
```

После подключения библиотеки и инициализации модулей, управление выводами модулей и Arduino, осуществляется через стандартные функции pinMode(), digitalRead() и digitalWrite().
При этом, выводы Arduino указываются типом uint8\_t, а выводы модулей типом uint16\_t.
