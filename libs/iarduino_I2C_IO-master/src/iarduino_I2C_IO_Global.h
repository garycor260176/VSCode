//	Библиотека для работы с Trema I2C расширителями выводов: http://iarduino.ru/shop/Expansion-payments/rasshiritel-na-8-vhodov-vyhodov-trema-modul.html
//  Версия: 1.1.1
//  Последнюю версию библиотеки Вы можете скачать по ссылке: http://iarduino.ru/file/285.html
//  Подробное описание функции бибилиотеки доступно по ссылке: http://wiki.iarduino.ru/page/rasshiritel-na-8-vhodov-vyhodov-trema-modul/
//  Библиотека является собственностью интернет магазина iarduino.ru и может свободно использоваться и распространяться!
//  При публикации устройств или скетчей с использованием данной библиотеки, как целиком, так и её частей,
//  в том числе и в некоммерческих целях, просим Вас опубликовать ссылку: http://iarduino.ru
//  Автор библиотеки: Панькин Павел.
//  Если у Вас возникли технические вопросы, напишите нам: shop@iarduino.ru

#ifndef __IARDUINO_I2C_IO_GLOBAL_H__
#define __IARDUINO_I2C_IO_GLOBAL_H__

#include   <iarduino_I2C_IO.h>
			iarduino_I2C_IO										iarduino_I2C_IO_Global;
	void	pinMode				(uint16_t i, uint8_t j)	{		iarduino_I2C_IO_Global.pinMode		(i,j);}
	bool	digitalRead			(uint16_t i)			{return	iarduino_I2C_IO_Global.digitalRead	(i  );}
	void	digitalWrite		(uint16_t i, uint8_t j)	{		iarduino_I2C_IO_Global.digitalWrite	(i,j);}
#endif