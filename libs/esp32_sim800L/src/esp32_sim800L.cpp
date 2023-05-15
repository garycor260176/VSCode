#include "esp32_sim800L.h"

esp32_sim800l::esp32_sim800l( HardwareSerial* _SIM800, 
                              unsigned long _baud, 
                              int8_t _rxPin, 
                              int8_t _txPin, 
                              String _phones, 
                              uint32_t _updatePeriod,
                              boolean _Serialprint){
  SIM800 = _SIM800;

  Serialprint = _Serialprint;

  baud = _baud;
  rxPin = _rxPin;
  txPin = _txPin;

  phones = _phones;
  lastUpdate = millis();
  updatePeriod = _updatePeriod;
  hasmsg = false;
}

void esp32_sim800l::delSMS(){
  _response = sendATCommand("AT+CMGDA=\"DEL ALL\"", true);
}

void esp32_sim800l::Serial_println(String msg){
  if(!Serialprint) return;

  Serial.println(msg);
}

void esp32_sim800l::loop(){
if (lastUpdate + updatePeriod < millis() ) {                    // Пора проверить наличие новых сообщений
    do {
      _response = sendATCommand("AT+CMGF=1", true);
      _response = sendATCommand("AT+CMGL=\"REC UNREAD\",1", true);// Отправляем запрос чтения непрочитанных сообщений
      if (_response.indexOf("+CMGL: ") > -1) {                    // Если есть хоть одно, получаем его индекс
        int msgIndex = _response.substring(_response.indexOf("+CMGL: ") + 7, _response.indexOf("\"REC UNREAD\"", _response.indexOf("+CMGL: ")) - 1).toInt();
        char i = 0;                                               // Объявляем счетчик попыток
        do {
          i++;                                                    // Увеличиваем счетчик
          _response = sendATCommand("AT+CMGR=" + (String)msgIndex + ",1", true);  // Пробуем получить текст SMS по индексу
          _response.trim();                                       // Убираем пробелы в начале/конце
          if (_response.endsWith("OK")) {                         // Если ответ заканчивается на "ОК"
            if (!hasmsg) hasmsg = true;                           // Ставим флаг наличия сообщений для удаления
            sendATCommand("AT+CMGR=" + (String)msgIndex, true);   // Делаем сообщение прочитанным
            sendATCommand("\n", true);                            // Перестраховка - вывод новой строки
            parseSMS(_response);                                  // Отправляем текст сообщения на обработку
            break;                                                // Выход из do{}
          }
          else {                                                  // Если сообщение не заканчивается на OK
            Serial_println("Error answer");                      // Какая-то ошибка
            sendATCommand("\n", true);                            // Отправляем новую строку и повторяем попытку
          }
        } while (i < 10);
        break;
      }
      else {
        lastUpdate = millis();                                    // Обнуляем таймер
        if (hasmsg) {
          sendATCommand("AT+CMGDA=\"DEL READ\"", true);           // Удаляем все прочитанные сообщения
          hasmsg = false;
        }
        break;
      }
    } while (1);
  }

  if (SIM800->available())   {                         // Если модем, что-то отправил...
    _response = waitResponse();                       // Получаем ответ от модема для анализа
    _response.trim();                                 // Убираем лишние пробелы в начале и конце
    Serial_println(_response);                        // Если нужно выводим в монитор порта
    pub_response(_response);
    if (_response.indexOf("+CMTI:")>-1) {             // Пришло сообщение об отправке SMS
      lastUpdate = millis() -  updatePeriod;          // Теперь нет необходимости обрабатываеть SMS здесь, достаточно просто
                                                      // сбросить счетчик автопроверки и в следующем цикле все будет обработано
    } else if(_response.startsWith("+CUSD:")) {
      String msg = _response.substring(_response.indexOf("\"") + 1);
      msg = msg.substring(0, msg.indexOf("\""));
      msg = UCS2ToString(msg);
      USSD_ANSWER( msg );
    }
  }
  if (Serial.available())  {                          // Ожидаем команды по Serial...
    SIM800->write(Serial.read());                      // ...и отправляем полученную команду модему
  };
}

void esp32_sim800l::sendSMS(String phone, String message)
{
  //sendATCommand("AT+CSMP=17,167,0,25", true);
  sendATCommand("AT+CMGF=1", true);
  //sendATCommand("AT+CSCS=\"UCS2\"", true);

  sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
  //sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
  String _result = sendATCommand(message + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}

void esp32_sim800l::parseSMS(String msg){
  String msgheader  = "";
  String msgbody    = "";
  String msgphone   = "";

ParseSms_01( msg );

  msg = msg.substring(msg.indexOf("+CMGR: "));
  msgheader = msg.substring(0, msg.indexOf("\r"));            // Выдергиваем телефон

  msgbody = msg.substring(msgheader.length() + 2);
  msgbody = msgbody.substring(0, msgbody.lastIndexOf("OK"));  // Выдергиваем текст SMS
  msgbody.trim();

  int firstIndex = msgheader.indexOf("\",\"") + 3;
  int secondIndex = msgheader.indexOf("\",\"", firstIndex);
  msgphone = msgheader.substring(firstIndex, secondIndex);

  Serial_println("Phone: " + msgphone);                       // Выводим номер телефона
  Serial_println("Message: " + msgbody);                      // Выводим текст SMS

  NewMessage( UCS2ToString(msgbody), msgphone );
}

bool esp32_sim800l::is_MyPhone(String msgphone){
  if (msgphone.length() > 6 && phones.indexOf(msgphone) > -1) { // Если телефон в белом списке, то...
    return true;
  }
  return false;
}

void esp32_sim800l::sendUSSD(String ussd){
  String response = sendATCommand("AT+CUSD=1,\"" + ussd + "\"", true);
  pub_response( response );
}

void esp32_sim800l::begin(){
  SIM800->begin(baud, SERIAL_8N1, rxPin, txPin, false);
  sendATCommand("AT", true);                                  // Отправили AT для настройки скорости обмена данными
  //sendATCommand("AT+CMGDA=\"DEL ALL\"", true);               // Удаляем все SMS, чтобы не забивать память

  // Команды настройки модема при каждом запуске
  //_response = sendATCommand("AT+CLIP=1", true);             // Включаем АОН
  //_response = sendATCommand("AT+DDET=1", true);             // Включаем DTMF
  sendATCommand("AT+CMGF=1;&W", true);                        // Включаем текстовый режима SMS (Text mode) и сразу сохраняем значение (AT&W)!
  lastUpdate = millis();                                      // Обнуляем таймер
}

String esp32_sim800l::sendATCommand(String cmd, bool waiting){
  String _resp = "";                                              // Переменная для хранения результата
  Serial_println("SIM command: " + cmd);                                            // Дублируем команду в монитор порта
  SIM800->println(cmd);                                            // Отправляем команду модулю
  if (waiting) {                                                  // Если необходимо дождаться ответа...
    _resp = waitResponse();                                       // ... ждем, когда будет передан ответ
    // Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
    if (_resp.startsWith(cmd)) {                                  // Убираем из ответа дублирующуюся команду
      _resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
    }
    Serial_println("SIM answer: " + _resp);                                        // Дублируем ответ в монитор порта
  }
  return _resp;                     
}

String esp32_sim800l::waitResponse() {                                           // Функция ожидания ответа и возврата полученного результата
  String _resp = "";                                              // Переменная для хранения результата
  long _timeout = millis() + 10000;                               // Переменная для отслеживания таймаута (10 секунд)
  while (!SIM800->available() && millis() < _timeout)  {};         // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
  if (SIM800->available()) {                                       // Если есть, что считывать...
    _resp = SIM800->readString();                                  // ... считываем и запоминаем
  }
  else {                                                          // Если пришел таймаут, то...
    Serial_println("Timeout...");                                 // ... оповещаем об этом и...
  }
  return _resp;                                                   // ... возвращаем результат. Пусто, если проблема
}

String esp32_sim800l::UCS2ToString(String s) {                       // Функция декодирования UCS2 строки
  String result = "";
  unsigned char c[5] = "";                            // Массив для хранения результата
  for (int i = 0; i < s.length() - 3; i += 4) {       // Перебираем по 4 символа кодировки
    unsigned long code = (((unsigned int)HexSymbolToChar(s[i])) << 12) +    // Получаем UNICODE-код символа из HEX представления
                         (((unsigned int)HexSymbolToChar(s[i + 1])) << 8) +
                         (((unsigned int)HexSymbolToChar(s[i + 2])) << 4) +
                         ((unsigned int)HexSymbolToChar(s[i + 3]));
    if (code <= 0x7F) {                               // Теперь в соответствии с количеством байт формируем символ
      c[0] = (char)code;                              
      c[1] = 0;                                       // Не забываем про завершающий ноль
    } else if (code <= 0x7FF) {
      c[0] = (char)(0xC0 | (code >> 6));
      c[1] = (char)(0x80 | (code & 0x3F));
      c[2] = 0;
    } else if (code <= 0xFFFF) {
      c[0] = (char)(0xE0 | (code >> 12));
      c[1] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[2] = (char)(0x80 | (code & 0x3F));
      c[3] = 0;
    } else if (code <= 0x1FFFFF) {
      c[0] = (char)(0xE0 | (code >> 18));
      c[1] = (char)(0xE0 | ((code >> 12) & 0x3F));
      c[2] = (char)(0x80 | ((code >> 6) & 0x3F));
      c[3] = (char)(0x80 | (code & 0x3F));
      c[4] = 0;
    }
    result += String((char*)c);                       // Добавляем полученный символ к результату
  }
  return (result);
}

unsigned char esp32_sim800l::HexSymbolToChar(char c) {
  if      ((c >= 0x30) && (c <= 0x39)) return (c - 0x30);
  else if ((c >= 'A') && (c <= 'F'))   return (c - 'A' + 10);
  else                                 return (0);
}

String esp32_sim800l::StringToUCS2(String s)
{
  String output = "";                                               // Переменная для хранения результата

  for (int k = 0; k < s.length(); k++) {                            // Начинаем перебирать все байты во входной строке
    byte actualChar = (byte)s[k];                                   // Получаем первый байт
    unsigned int charSize = getCharSize(actualChar);                // Получаем длину символа - кличество байт.

    // Максимальная длина символа в UTF-8 - 6 байт плюс завершающий ноль, итого 7
    char symbolBytes[charSize + 1];                                 // Объявляем массив в соответствии с полученным размером
    for (int i = 0; i < charSize; i++)  symbolBytes[i] = s[k + i];  // Записываем в массив все байты, которыми кодируется символ
    symbolBytes[charSize] = '\0';                                   // Добавляем завершающий 0

    unsigned int charCode = symbolToUInt(symbolBytes);              // Получаем DEC-представление символа из набора байтов
    if (charCode > 0)  {                                            // Если все корректно преобразовываем его в HEX-строку
      // Остается каждый из 2 байт перевести в HEX формат, преобразовать в строку и собрать в кучу
      output += byteToHexString((charCode & 0xFF00) >> 8) +
                byteToHexString(charCode & 0xFF);
    }
    k += charSize - 1;                                              // Передвигаем указатель на начало нового символа
  }
  return output;                                                    // Возвращаем результат
}

unsigned int esp32_sim800l::getCharSize(unsigned char b) { // Функция получения количества байт, которыми кодируется символ
  // По правилам кодирования UTF-8, по старшим битам первого октета вычисляется общий размер символа
  // 1  0xxxxxxx - старший бит ноль (ASCII код совпадает с UTF-8) - символ из системы ASCII, кодируется одним байтом
  // 2  110xxxxx - два старших бита единицы - символ кодируется двумя байтами
  // 3  1110xxxx - 3 байта и т.д.
  // 4  11110xxx
  // 5  111110xx
  // 6  1111110x

  if (b < 128) return 1;             // Если первый байт из системы ASCII, то он кодируется одним байтом

  // Дальше нужно посчитать сколько единиц в старших битах до первого нуля - таково будет количество байтов на символ.
  // При помощи маски, поочереди исключаем старшие биты, до тех пор пока не дойдет до нуля.
  for (int i = 1; i <= 7; i++) {
    if (((b << i) & 0xFF) >> 7 == 0) {
      return i;
    }
  }
  return 1;
}

unsigned int esp32_sim800l::symbolToUInt(const String& bytes) {  // Функция для получения DEC-представления символа
  unsigned int charSize = bytes.length();         // Количество байт, которыми закодирован символ
  unsigned int result = 0;
  if (charSize == 1) {
    return bytes[0]; // Если символ кодируется одним байтом, сразу отправляем его
  }
  else  {
    unsigned char actualByte = bytes[0];
    // У первого байта оставляем только значимую часть 1110XXXX - убираем в начале 1110, оставляем XXXX
    // Количество единиц в начале совпадает с количеством байт, которыми кодируется символ - убираем их
    // Например (для размера 2 байта), берем маску 0xFF (11111111) - сдвигаем её (>>) на количество ненужных бит (3 - 110) - 00011111
    result = actualByte & (0xFF >> (charSize + 1)); // Было 11010001, далее 11010001&(11111111>>(2+1))=10001
    // Каждый следующий байт начинается с 10XXXXXX - нам нужны только по 6 бит с каждого последующего байта
    // А поскольку остался только 1 байт, резервируем под него место:
    result = result << (6 * (charSize - 1)); // Было 10001, далее 10001<<(6*(2-1))=10001000000

    // Теперь у каждого следующего бита, убираем ненужные биты 10XXXXXX, а оставшиеся добавляем к result в соответствии с расположением
    for (int i = 1; i < charSize; i++) {
      actualByte = bytes[i];
      if ((actualByte >> 6) != 2) return 0; // Если байт не начинается с 10, значит ошибка - выходим
      // В продолжение примера, берется существенная часть следующего байта
      // Например, у 10011111 убираем маской 10 (биты в начале), остается - 11111
      // Теперь сдвигаем их на 2-1-1=0 сдвигать не нужно, просто добавляем на свое место
      result |= ((actualByte & 0x3F) << (6 * (charSize - 1 - i)));
      // Было result=10001000000, actualByte=10011111. Маской actualByte & 0x3F (10011111&111111=11111), сдвигать не нужно
      // Теперь "пристыковываем" к result: result|11111 (10001000000|11111=10001011111)
    }
    return result;
  }
}

String esp32_sim800l::byteToHexString(byte i) { // Функция преобразования числового значения байта в шестнадцатиричное (HEX)
  String hex = String(i, HEX);
  if (hex.length() == 1) hex = "0" + hex;
  hex.toUpperCase();
  return hex;
}

void esp32_sim800l::sendSMSinPDU(String phone, String message){
  // ============ Подготовка PDU-пакета =============================================================================================
  // В целях экономии памяти будем использовать указатели и ссылки
  String *ptrphone = &phone;                                    // Указатель на переменную с телефонным номером
  String *ptrmessage = &message;                                // Указатель на переменную с сообщением

  String PDUPack;                                               // Переменная для хранения PDU-пакета
  String *ptrPDUPack = &PDUPack;                                // Создаем указатель на переменную с PDU-пакетом

  int PDUlen = 0;                                               // Переменная для хранения длины PDU-пакета без SCA
  int *ptrPDUlen = &PDUlen;                                     // Указатель на переменную для хранения длины PDU-пакета без SCA

  getPDUPack(ptrphone, ptrmessage, ptrPDUPack, ptrPDUlen);      // Функция формирующая PDU-пакет, и вычисляющая длину пакета без SCA

  Serial.println("PDU-pack: " + PDUPack);
  Serial.println("PDU length without SCA:" + (String)PDUlen);

  // ============ Отправка PDU-сообщения ============================================================================================
  sendATCommand("AT+CMGF=0", true);                             // Включаем PDU-режим
  sendATCommand("AT+CMGS=" + (String)PDUlen, true);             // Отправляем длину PDU-пакета
  sendATCommand(PDUPack + (String)((char)26), true);            // После PDU-пакета отправляем Ctrl+Z  
}

void esp32_sim800l::getPDUPack(String *phone, String *message, String *result, int *PDUlen)
{
  // Поле SCA добавим в самом конце, после расчета длины PDU-пакета
  *result += "01";                                // Поле PDU-type - байт 00000001b
  *result += "00";                                // Поле MR (Message Reference)
  *result += getDAfield(phone, true);             // Поле DA
  *result += "00";                                // Поле PID (Protocol Identifier)
  *result += "08";                                // Поле DCS (Data Coding Scheme)
  //*result += "";                                // Поле VP (Validity Period) - не используется

  String msg = StringToUCS2(*message);            // Конвертируем строку в UCS2-формат

  *result += byteToHexString(msg.length() / 2);   // Поле UDL (User Data Length). Делим на 2, так как в UCS2-строке каждый закодированный символ представлен 2 байтами.
  *result += msg;

  *PDUlen = (*result).length() / 2;               // Получаем длину PDU-пакета без поля SCA
  *result = "00" + *result;                       // Добавляем поле SCA
}

String esp32_sim800l::getDAfield(String *phone, bool fullnum) {
  String result = "";
  for (int i = 0; i <= (*phone).length(); i++) {  // Оставляем только цифры
    if (isDigit((*phone)[i])) {
      result += (*phone)[i];
    }
  }
  int phonelen = result.length();                 // Количество цифр в телефоне
  if (phonelen % 2 != 0) result += "F";           // Если количество цифр нечетное, добавляем F

  for (int i = 0; i < result.length(); i += 2) {  // Попарно переставляем символы в номере
    char symbol = result[i + 1];
    result = result.substring(0, i + 1) + result.substring(i + 2);
    result = result.substring(0, i) + (String)symbol + result.substring(i);
  }

  result = fullnum ? "91" + result : "81" + result; // Добавляем формат номера получателя, поле PR
  result = byteToHexString(phonelen) + result;    // Добавляем длиу номера, поле PL

  return result;
}

boolean esp32_sim800l::isOK(String data){
  boolean ret = false;
  String in = data;
  in.replace('\r', ' ');
  in.replace('\n', ' ');
  int i = in.length( ) - 2;
  if(i >= 0 ){
    in = in.substring(i - 2, i );
    if(in == "OK")  ret = true;
  }
  return ret;
}

bool esp32_sim800l::gprsDisconnect() {
    if(!isOK(sendATCommand("AT+CIPSHUT", true))) return false;
    if(!isOK(sendATCommand("AT+CGATT=0", true))) return false;
    return true;
  }
  
bool esp32_sim800l::gprsConnect(const char* apn, const char* user, const char* pwd) {
    gprsDisconnect();

    sendATCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", true);
    sendATCommand("AT+SAPBR=3,1,\"APN\",\"" + String(apn) + "\"", true);
    sendATCommand("AT+SAPBR=3,1,\"USER\",\"" + String(user) + "\"", true);
    sendATCommand("AT+SAPBR=3,1,\"PWD\",\"" + String(pwd) + "\"", true);
    sendATCommand("AT+CGDCONT=1,\"IP\",\""+ String(apn) + "\"", true);
    sendATCommand("AT+CGACT=1,1", true);
    sendATCommand("AT+SAPBR=1,1", true);
    
    if(!isOK(sendATCommand("AT+SAPBR=2,1", true))) return false;
    if(!isOK(sendATCommand("AT+CGATT=1", true))) return false;
    if(!isOK(sendATCommand("AT+CIPMUX=1", true))) return false;
    if(!isOK(sendATCommand("AT+CIPQSEND=1", true))) return false;
    if(!isOK(sendATCommand("AT+CIPRXGET=1", true))) return false;
    if(!isOK(sendATCommand("AT+CSTT=\"" + String(apn) + "\",\"" + String(user) + "\",\"" + String(pwd) + "\"", true))) return false;
    if(!isOK(sendATCommand("AT+CIICR", true))) return false;
    if(!isOK(sendATCommand("AT+CIFSR;E0", true))) return false;
    if(!isOK(sendATCommand("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"", true))) return false;

    return true;
  }

boolean esp32_sim800l::IsGrpsConnected(){
  boolean ret = false;
  String s = sendATCommand("AT+CGATT?", true);
  if(isOK(s)){
    if(s.indexOf("+CGATT: ") > 0){
      s = s.substring(10, 11);
      if(s == "1"){
        ret = true;
      }
    }
  }
  return ret;
}

stru_datetime esp32_sim800l::GetTime(){
  stru_datetime ret;

  String s = sendATCommand("AT+CCLK?", true);
  if(!isOK(s)) return ret;

  s.replace('\r', ' ');  s.replace('\n', ' ');
  String s_search = "+CCLK: \"";
  int i = s.indexOf(s_search);
  if(i < 0) return ret;
  s = s.substring(i + s_search.length( ));
  i = s.indexOf('"');
  if(i < 0) return ret;
  s = s.substring(0, i);

  ParseDateTimeString(s, ret.year, ret.month, ret.day, ret.hours, ret.minutes, ret.seconds, ret.timezone);

  return ret;
}

boolean esp32_sim800l::SetAutoTimeSync(){
  if(!isOK(sendATCommand("AT+CLTS=1", true))) return false;
  if(!isOK(sendATCommand("AT&W", true))) return false;
  sendATCommand("AT+CFUN=0", true);
  if(!isOK(sendATCommand("AT+CFUN=1", true))) return false;
  return true;
}

bool esp32_sim800l::ParseDateTimeString(String datetime, byte &year, byte &month, byte &day, byte &hours, byte &minutes, byte &seconds, byte &timezone)
{ 
   return sscanf(datetime.c_str(), "%d/%d/%d,%d:%d:%d+%d", &year, &month, &day, &hours, &minutes, &seconds, &timezone) == 7;
}