#include "iarduino_I2C_IO.h"																						//
																													//
//		Инициация работы модуля					()																	//
void	iarduino_I2C_IO::begin					(){objI2C->begin(100);}												//	Инициируем работу по шине I2C на скорости 100 кГц
																													//
//		Установка режима работы вывода			(№ вывода,	режим работы)											//
void	iarduino_I2C_IO::pinMode				(uint8_t i,	uint8_t j){												//
			if(!EXP_func_FIND(i,EXP_REG_CONFIG)){return;}															//	Определяем адрес модуля EXP_uint_ADDRESS, адрес регистра EXP_uint_COMMAND и номер вывода
			EXP_uint_DATA = objI2C->readByte(EXP_uint_ADDRESS, EXP_uint_COMMAND);									//	Читаем байт данных из регистра EXP_uint_COMMAND модуля с адресом EXP_uint_ADDRESS в переменную EXP_uint_DATA
			if(j==OUTPUT)	{EXP_uint_DATA&=~EXP_uint_BIT;}															//	Сбрасываем    бит, соответствующего вывода, в прочтённых данных
			else			{EXP_uint_DATA|= EXP_uint_BIT;}															//	Устанавливаем бит, соответствующего вывода, в прочтённых данных
			objI2C->writeByte(EXP_uint_ADDRESS, EXP_uint_COMMAND, EXP_uint_DATA);									//	Записываем данные из переменной EXP_uint_DATA в регистр EXP_uint_COMMAND модуля с адресом EXP_uint_ADDRESS
}																													//
																													//
//		Установка логического состояния			(№ вывода,	состояние)												//
void	iarduino_I2C_IO::digitalWrite			(uint8_t i,	bool j){												//
			if(!EXP_func_FIND(i,EXP_REG_OUTPUT)){return;}															//	Определяем адрес модуля EXP_uint_ADDRESS, адрес регистра EXP_uint_COMMAND и номер вывода
			EXP_uint_DATA = objI2C->readByte(EXP_uint_ADDRESS, EXP_uint_COMMAND);									//	Читаем байт данных из регистра EXP_uint_COMMAND модуля с адресом EXP_uint_ADDRESS в переменную EXP_uint_DATA
			if(!j)			{EXP_uint_DATA&=~EXP_uint_BIT;}															//	Сбрасываем    бит, соответствующего вывода, в прочтённых данных
			else			{EXP_uint_DATA|= EXP_uint_BIT;}															//	Устанавливаем бит, соответствующего вывода, в прочтённых данных
			objI2C->writeByte(EXP_uint_ADDRESS, EXP_uint_COMMAND, EXP_uint_DATA);									//	Записываем данные из переменной EXP_uint_DATA в регистр EXP_uint_COMMAND модуля с адресом EXP_uint_ADDRESS
}																													//
																													//
//		Чтение логического состояния			(№ вывода)															//
bool	iarduino_I2C_IO::digitalRead			(uint8_t i){														//
			if(!EXP_func_FIND(i,EXP_REG_INPUT)){return LOW;}														//	Определяем адрес модуля EXP_uint_ADDRESS, адрес регистра EXP_uint_COMMAND и номер вывода
			EXP_uint_DATA = objI2C->readByte(EXP_uint_ADDRESS, EXP_uint_COMMAND);									//	Читаем байт данных из регистра EXP_uint_COMMAND модуля с адресом EXP_uint_ADDRESS в переменную EXP_uint_DATA
			return EXP_uint_DATA&=EXP_uint_BIT;																		//	Возвращаем бит из позиции EXP_uint_BIT байта EXP_uint_DATA
}																													//
																													//
//		Определение параметров для передачи		(№ вывода,	адрес регистра)											//
bool	iarduino_I2C_IO::EXP_func_FIND			(uint8_t i,	uint8_t j){												//
			EXP_uint_COMMAND=j;																						//	Определяем команду EXP_uint_COMMAND состоящую из адреса регистра
			EXP_uint_BIT=_BV(i%8);																					//	Устанавливаем в 1 один бит байта EXP_uint_BIT соответсвующий позиции бита управления выводом в модуле
			if(EXP_uint_ADDRESS_OBJ<255)		{EXP_uint_ADDRESS=EXP_uint_ADDRESS_OBJ;	return i<=7?true:false;}	//	Определяем адрес модуля EXP_uint_ADDRESS если был указан адрес EXP_uint_ADDRESS_OBJ
			if(i>=64)							{										return           false;}	//	Возвращаем ошибку если номер вывода превышает максимальное значение (8 модулей x 8 выводов = 64)
			EXP_uint_ADDRESS=(i/8)+0x20;												return true;				//	Определяем адрес модуля EXP_uint_ADDRESS если не был указан адрес EXP_uint_ADDRESS_OBJ
}																													//