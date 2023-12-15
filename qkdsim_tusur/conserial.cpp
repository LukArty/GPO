/// @file
/// @brief Файл реализации класса, общающегося с микроконтроллером.
///
/// @copyright Copyright 2022 InfoTeCS.

#include <conserial.h>

namespace hwe
{

Conserial::Conserial()
{
#ifdef CE_WINDOWS
    com_.SetPortName(string("COM4"));
#else
    com_.SetPortName(string("/dev/ttyStandQKD"));
#endif
    com_.SetBaudRate(115200);
    com_.SetDataSize(8);
    com_.SetParity('N');
    com_.SetStopBits(1);
    com_.Open();
    com_.Close();
    com_.Open();

}

Conserial::~Conserial()
{ }

api:: InitResponse Conserial:: Init()
{
    api::InitResponse response; // Структура для формирования ответа
    std::fstream ini_("./Angles.ini");
    if (!ini_.is_open()) { response = InitByPD();  }
    else {
        const int n =6;
        char temp_ [n];

        ini_.getline(temp_, n);//считываем первую строку где лежит угл для 1 пластины. Вид : 003.4
        temp_[3]='.';//Меняем разделитель на точку (На случай если запятая)
        string angle1_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle2_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle3_ = temp_;
        ini_.getline(temp_, n);
        temp_[3]='.';
        string angle4_ = temp_;

        if (angle1_.length() > 4 && angle2_.length() > 4 && angle3_.length() > 4 && angle4_.length() > 4){ //Если в файл записаны углы
            WAngles<angle_t> anglesIni_;
            anglesIni_.aHalf_= stof (angle1_) ;
            anglesIni_.aQuart_= stof (angle2_);
            anglesIni_.bHalf_= stof (angle3_);
            anglesIni_.bQuart_= stof (angle4_);
            response =  InitByButtons(anglesIni_);} //Инициализация по концевикам
        else {response = InitByPD();}//Инициализация по фотодиодам
    }
    return response;
}

api::InitResponse Conserial::InitByPD()
{

    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;
    uint16_t tempTimeOut_ = 900;
    uint16_t tempData = standOptions.timeoutTime_;
    standOptions.timeoutTime_ = tempTimeOut_;

    UartResponse pack;
    pack = Twiting(dict_.find("Init")->second, 0);

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[9];
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.curAngles_ = standOptions.startPlatesAngles_;
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;
    standOptions.timeoutTime_ = tempData;
    return response; // Возвращаем сформированный ответ
}

api::InitResponse Conserial::InitByButtons(WAngles<angle_t> angles)
{
    api::InitResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("InitByButtons")->second, 4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[8];
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.curAngles_ = standOptions.startPlatesAngles_;
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;
    return response; // Возвращаем сформированный ответ
}

api::AdcResponse Conserial::RunTest()
{

    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("RunSelfTest")->second, 0);

    response.adcResponse_ = pack.parameters_[0]; // Возвращаем целое число
    response.errorCode_ = pack.status_; // Команда отработала корректно

    return response;
}

api::SendMessageResponse Conserial::Sendmessage(WAngles<angle_t> angles, adc_t power)
{
    api::SendMessageResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("SendMessage")->second, 5, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_, power);

    // Заполняем поля
    response.newPlatesAngles_.aHalf_  = ((float)pack.parameters_[0]) * standOptions.rotateStep_; // <- полуволновая пластина "Алисы"     (1я пластинка)
    response.newPlatesAngles_.aQuart_ = ((float)pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.newPlatesAngles_.bHalf_  = ((float)pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.newPlatesAngles_.bQuart_ = ((float)pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.currentLightNoises_.h_ = pack.parameters_[4]; // <- засветка детектора, принимающего горизонтальную поляризацию
    response.currentLightNoises_.v_ = pack.parameters_[5]; // <- засветка детектора, принимающего вертикальную поляризацию

    response.currentSignalLevels_.h_ = pack.parameters_[6]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.currentSignalLevels_.v_ = pack.parameters_[7]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.newPlatesAngles_; // Запомнили текущие значения углов
    standOptions.lightNoises = response.currentLightNoises_;
    standOptions.signalLevels_ = response.currentSignalLevels_;

    return response;
}

api::AdcResponse Conserial::SetTimeout(adc_t timeout)
{

    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды
    if(standOptions.premissions!=1){
        response = {0,0};
        return response;
    }

    if (timeout <= 0){
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return response;
    }
    else if (timeout >= 900){timeout = 900;}

    UartResponse pack;
    pack = Twiting(dict_.find("SetLaserState")->second,  1, timeout);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_ = response.adcResponse_ ;

    return response;
}

api::AdcResponse Conserial::SetLaserState(adc_t on)
{

    api::AdcResponse response; // Структура для формирования ответа

    if(on != 1 && on != 0)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return response;
    }

    UartResponse pack;
    pack = Twiting(dict_.find("SetLaserState")->second,  1, on);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;
    standOptions.laserState_ = response.adcResponse_;
    return response; // Возвращаем значение, соответствующее установленному состоянию
}

api::AdcResponse Conserial::SetLaserPower(adc_t power)
{
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    if (power > standOptions.maxLaserPower_)
    {
        response.errorCode_ = 2; // Принят некорректный входной параметр
        return response;
    }

    pack = Twiting(dict_.find("SetLaserPower")->second,  1, power);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return response; // Возвращаем значение, соответствующее установленному уровню
}

api::WAnglesResponse Conserial::SetPlatesAngles(WAngles<angle_t> angles)
{
    api::WAnglesResponse response; // Структура для формирования ответа

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("SetPlatesAngles")->second,  4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::UpdateBaseAngle(WAngles<angle_t> angles)
{
    api::WAnglesResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {{0,0,0,0},0};
        return response;
    }

    WAngles<adc_t> steps = CalcSteps(angles);

    UartResponse pack;
    pack = Twiting(dict_.find("UpdateBaseAngles")->second,  4, steps.aHalf_, steps.aQuart_, steps.bHalf_, steps.bQuart_);

    steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return response; // Возвращаем, чего там получилось установить
}

api::WAnglesResponse Conserial::ReadBaseAngles()
{
    api::WAnglesResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("ReadBaseAngles")->second, 0);

    WAngles<adc_t> steps = {pack.parameters_[0], pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::ReadEEPROM(uint8_t numberUnit_)
{
    api::AdcResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {0,0};
        return response;
    }

    UartResponse pack;
    pack = Twiting(dict_.find("ReadEEPROM")->second, 1, numberUnit_);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::WriteEEPROM(uint8_t numberUnit_, uint16_t param_)
{
    api::AdcResponse response; // Структура для формирования ответа

    if(standOptions.premissions!=1){
        response = {0,0};
        return response;
    }

    UartResponse pack;
    pack = Twiting(dict_.find("WriteEEPROM")->second, 2, numberUnit_, param_);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserState()
{
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLaserState")->second, 0);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserState_ = response.adcResponse_;
    return response; // Возвращаем полученное состояние
}

api::AdcResponse Conserial::GetLaserPower()
{
    api::AdcResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLaserPower")->second, 0);

    // Заполняем поля для ответа
    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.laserPower_= response.adcResponse_;
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetPlatesAngles()
{
    api::WAnglesResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetCurPlatesAngles")->second, 0);

    // Получаем текущие углы поворота волновых пластин от МК
    WAngles<adc_t> steps = {pack.parameters_[0],pack.parameters_[1], pack.parameters_[2],pack.parameters_[3]};

    // Записываем полученное в структуру
    response.angles_ =  CalcAngles(steps);
    response.errorCode_ = pack.status_;

    standOptions.curAngles_ = response.angles_;
    return response;
}

api::SLevelsResponse Conserial::GetSignalLevels()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetSignalLevel")->second, 0);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазер
    response.errorCode_ = pack.status_;

    standOptions.signalLevels_ = response.signal_;
    return response;
}

api::AngleResponse Conserial::GetRotateStep()
{
    api::AngleResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetRotateStep")->second, 0);

    // Получаем от МК количество шагов для поворота на 360 градусов
    uint16_t steps_ = pack.parameters_[0];
    if(steps_!=0){  standOptions.rotateStep_ = 360.0 / steps_;} // Считаем сколько градусов в одном шаге

    response.angle_= standOptions.rotateStep_;
    response.errorCode_ = pack.status_;

    return response;
}

api::SLevelsResponse Conserial::GetLightNoises()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    UartResponse pack;
    pack = Twiting(dict_.find("GetLightNoises")->second, 0);

    // Заполняем структуру для ответа
    response.signal_.h_ = pack.parameters_[0]; // <- уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.signal_.v_ = pack.parameters_[1]; // <- уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере
    response.errorCode_ = pack.status_;

    standOptions.lightNoises = response.signal_;
    return response;
}

api::AdcResponse Conserial::GetHardwareState(){
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack;
    pack = Twiting(dict_.find("GetHardwareState")->second, 0);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response;
};

api::AdcResponse Conserial::GetErrorCode()
{
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack;
    pack = Twiting(dict_.find("GetErrorCode")->second, 0);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    return response;
}

api::AdcResponse Conserial::GetTimeout()
{
    api::AdcResponse response; // Поле типа adc_t c ответом и код ошибки команды

    UartResponse pack;
    pack = Twiting(dict_.find("GetTimeout")->second, 0);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.timeoutTime_  = response.adcResponse_;

    return response;
}

api::InitResponse Conserial::GetInitParams(){

    api::InitResponse response; // Структура для формирования ответа
    response.errorCode_ = 0;


    UartResponse pack;
    pack = Twiting(dict_.find("GetInitParams")->second, 0);


    // Заполняем поля структуры
    response.startPlatesAngles_.aHalf_  = ((float) pack.parameters_[0]) * standOptions.rotateStep_; //<- полуволновая пластина "Алисы"     (1я пластинка)
    response.startPlatesAngles_.aQuart_ = ((float) pack.parameters_[1]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Алисы" (2я пластинка)
    response.startPlatesAngles_.bHalf_  = ((float) pack.parameters_[2]) * standOptions.rotateStep_; // <- полуволновая пластина "Боба"      (3я пластинка)
    response.startPlatesAngles_.bQuart_ = ((float) pack.parameters_[3]) * standOptions.rotateStep_; // <- четвертьволновая пластина "Боба"  (4я пластинка)

    response.startLightNoises_.h_ = pack.parameters_[4]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.startLightNoises_.v_ = pack.parameters_[5]; //<- начальная засветка детектора, принимающего вертикальную поляризацию

    response.maxSignalLevels_.h_ = pack.parameters_[6]; // <- максимальный уровень сигнала на детекторе, принимающем горизонтальную поляризацию, при включенном лазере
    response.maxSignalLevels_.v_ = pack.parameters_[7]; // <- максимальный уровень сигнала на детекторе, принимающем вертикальную поляризацию, при включенном лазере

    response.maxLaserPower_ = pack.parameters_[9];
    response.errorCode_ = pack.status_;

    standOptions.startPlatesAngles_ = response.startPlatesAngles_; // Сохраняем текущее значение углов на будущее
    standOptions.startLightNoises_ = response.startLightNoises_;
    standOptions.maxSignalLevels_ = response.maxSignalLevels_;
    standOptions.maxLaserPower_ = response.maxLaserPower_;

    return response; // Возвращаем сформированный ответ

};

api::SLevelsResponse Conserial::GetStartLightNoises()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    // получаем от МК начальные уровни засветки
    UartResponse pack;
    pack = Twiting(dict_.find("GetMaxLaserPower")->second, 0);

    // Заполняем структуру
    response.signal_.h_ = pack.parameters_[0]; // <- начальная засветка детектора, принимающего горизонтальную поляризацию
    response.signal_.v_ = pack.parameters_[1]; // <- начальная засветка детектора, принимающего вертикальную поляризацию
    response.errorCode_ = pack.status_;
    return response;
}

api::AngleResponse Conserial::SetPlateAngle(adc_t plateNumber, angle_t angle)
{
    api::AngleResponse response; // Структура для формирования ответа
    api::WAnglesResponse tempResponse;
    WAngles<angle_t> angles;

    if(plateNumber < 1 || plateNumber > 4)
    {
        response.errorCode_ = 2; // // Принят некорректный входной параметр
        return response;
    }

    switch (plateNumber)
    {
    case 1:
        angles = {angle,standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 2:
        angles = {standOptions.curAngles_.aHalf_, angle,
                  standOptions.curAngles_.bHalf_, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.aQuart_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 3:
        angles = {standOptions.curAngles_.aHalf_,standOptions.curAngles_.aQuart_,
                  angle, standOptions.curAngles_.bQuart_};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.bHalf_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    case 4:
        angles = {standOptions.curAngles_.aHalf_,standOptions.curAngles_.aQuart_,
                  standOptions.curAngles_.bHalf_, angle};
        tempResponse = SetPlatesAngles(angles);

        // Заполняем поля
        response.angle_ = tempResponse.angles_.bQuart_;
        response.errorCode_ = tempResponse.errorCode_;
        standOptions.curAngles_ = tempResponse.angles_;

        break;
    }

    return response; // Возвращаем, чего там получилось установить
}

api::AdcResponse Conserial::GetMaxLaserPower()
{
    api::AdcResponse response; // Структура для формирования ответа

    // Заполняем поля для ответа
    response.adcResponse_ = standOptions.maxLaserPower_;
    response.errorCode_ = 0;
    return response; // Возвращаем полученное состояние
}

api::WAnglesResponse Conserial::GetStartPlatesAngles()
{
    api::WAnglesResponse response; // Структура для формирования ответа

    // Записываем полученное в структуру
    response.angles_ =  standOptions.startPlatesAngles_;
    response.errorCode_ = 0;
    // возвращаем структуру
    return response;
}

api::SLevelsResponse Conserial::GetMaxSignalLevels()
{
    api::SLevelsResponse response; // Структура для формирования ответа

    response.signal_ = standOptions.maxSignalLevels_;
    response.errorCode_ = 0;
    return response;
}

api::AdcResponse Conserial::CreateConfigSecret(string passwd){

    api::AdcResponse response;

    if(standOptions.premissions!=1){
        response = {0,0};
        return response;
    }



    int str_length = size(passwd);
    if(str_length>20)
    {
        response.errorCode_ = 5; // Принят некорректный входной параметр
        return response;
    }
    //Старшие байты добиваются нулями в случае короткого пароля
    while (str_length<20 ){
        passwd=(char) 0x00 + passwd;
        str_length = size(passwd);
    }

    UartResponse pack;
    pack = Twiting(dict_.find("CreateConfigSecret")->second, (uint8_t *)&passwd, str_length);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.premissions  = response.adcResponse_;

    return response;
}

api::AdcResponse Conserial::OpenConfigMode(string passwd){

    api::AdcResponse response;

    int str_length = size(passwd);
    if(str_length>20)
    {
        response.errorCode_ = 5; // Принят некорректный входной параметр
        return response;
    }
    while (str_length<20 ){
        passwd=(char) 0x00 + passwd;
        str_length = size(passwd);
    }


    UartResponse pack;
    pack = Twiting(dict_.find("OpenConfigMode")->second, (uint8_t *)&passwd, str_length);

    response.adcResponse_ = pack.parameters_[0];
    response.errorCode_ = pack.status_;

    standOptions.premissions  = response.adcResponse_;

    return response;
}

uint16_t Conserial::CloseConfigMode()
{
    standOptions.premissions  = 0;
    return 1;
}

uint16_t Conserial::GetCurrentMode()
{
    return standOptions.premissions;
}

uint16_t Conserial::GetMaxPayloadSize()
{
    UartResponse pack;
    pack = Twiting(dict_.find("GetMaxPayloadSize")->second, 0);
    standOptions.maxPayloadSize = pack.parameters_[0];
    return standOptions.maxPayloadSize;
}

hwe::versionProtocolResponse Conserial::GetProtocolVersion (){
    hwe::versionProtocolResponse response;
    UartResponse pack;
    pack = Twiting(dict_.find("GetProtocolVersion")->second, 0);

    response.version_ = pack.parameters_[0];
    response.subversion_ = pack.parameters_[1];
    response.errorCode_ = pack.status_;

    versionProtocol  = {response.version_,response.subversion_};

    return response;
}

hwe::versionFirmwareResponse Conserial::GetCurrentFirmwareVersion(){
    hwe::versionFirmwareResponse response;
    UartResponse pack;
    pack = Twiting(dict_.find("GetCurrentFirmwareVersion")->second, 0);

    response.major_ = pack.parameters_[0];
    response.minor_ = pack.parameters_[1];
    response.micro_ = pack.parameters_[2];
    response.errorCode_ = pack.status_;

    versionFirmware  = {response.major_,response.minor_,response.micro_};
    return response;
};

Conserial::UartResponse Conserial::Twiting (char commandName, int N, ... ){
    UartResponse pack;
    // Проверка соединения
    if (!StandIsConected())
    {
        pack.status_= 17;

    }else {

        va_list temp_params;
        va_start(temp_params,N);
        uint16_t * params =new uint16_t [N];

        for (int i = 0; i < N; i++){
            params[i]= va_arg(temp_params, unsigned int);
        }
        va_end(temp_params);

        uint8_t * temp_= new uint8_t [2*N];
        int j = 0;
        for (int i = 1; i <= 2 * N; i= i+2){
            temp_[i] = params[j]>>8;
            temp_[i+1] = params[j];
            j++;
        };

        int count = 0;
        while (count < 3) {
            //Посылаем запрос МК
            SendUart(commandName, temp_, 2*N );

            //Чтение ответа
            pack = ParsePackege(standOptions.timeoutTime_);
            if (pack.status_==1){break;}
            ++count;
        }
        delete [] params;
    }
    pack.status_= CheckStatus(pack.status_);
    return pack;
}

Conserial::UartResponse Conserial::Twiting (char commandName, uint8_t * bytes, uint16_t length){

    UartResponse pack;

    // Проверка соединения
    if (!StandIsConected())
    {
        pack.status_= 17;
    }else {

        int count = 0;
        while (count < 3) {
            //Посылаем запрос МК
            SendUart(commandName, bytes, length);

            //Чтение ответа
            if (versionProtocol.version == 1 && versionProtocol.subversion == 2){
                pack =  ParsePackege(standOptions.timeoutTime_);
            }else if(versionProtocol.version == 1 && versionProtocol.subversion == 1){
                pack =  ParsePackege_1_0(standOptions.timeoutTime_);
            }

            if (pack.status_==1){break;}
            ++count;
        }
    }
    pack.status_= CheckStatus(pack.status_);
    return pack;
}

uint16_t Conserial:: SendUart (char commandName, uint8_t * bytes, uint16_t N){

    uint8_t start1 = 255;
    uint8_t start2 = 254;
    uint16_t end = 65535;
    uint8_t solt = 0;
    uint8_t crc;

    uint8_t temp_[256] = {(uint8_t) commandName,};

    for (int i = 1; i <=  N; i++){
        temp_[i] = bytes[i-1];
    };
    crc = Crc8((uint8_t *) &temp_, N+2);

    com_.Write(start1);
    com_.Write(start2);
    if (versionProtocol.version == 1 && (versionProtocol.subversion == 2 || versionProtocol.subversion == 5)){
        if (versionProtocol.subversion == 5){
            com_.Write(N);  // Payload_size
        }
        for (int s=0 ; s <= N; s++){
            com_.Write(temp_[s]);
        }
        com_.Write(solt);
        com_.Write(crc);
    }else if(versionProtocol.version == 1 && versionProtocol.subversion == 1){
        com_.Write(solt);
        com_.Write((uint8_t )commandName);
        com_.Write(crc);
        for (int s=1 ; s <= N; s++){
            com_.Write(temp_[s]);
        }
    }
    com_.Write(end);
    return 1;
};

Conserial::UartResponse Conserial::ParsePackege(unsigned int timeout){
    bool startPackFlag_=false;
    bool successReceipt_=false;
    bool flag_ = 0;
    bool a = 0;
    array <uint16_t, 201>  params  = {0};
    Conserial::UartResponse pack_;
    uint8_t currentByte_=0;
    timeout = timeout * 1000;

    if (!com_.IsOpened())
    {
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    clock_t end_time = clock() + timeout * (CLOCKS_PER_SEC/1000) ;
    while (clock()<end_time) {
        while (startPackFlag_ != true && clock()<end_time) {
            currentByte_ = com_.ReadChar(a);
            if(a){
                if (currentByte_ == 255){
                    flag_ = 1;
                }
                else{
                    if (flag_ == 1){
                        if (currentByte_ != 254)
                        {
                            flag_ = 0;
                        }
                        else{
                            startPackFlag_ = true;
                            break;
                        }
                    }
                    else {
                        flag_ = 0;
                    }
                }
            }
        }

        if (startPackFlag_ == true){
            int count = 9;
            int paramCnt =1;
            unsigned short p=0;
            while (p != 65535 && clock()<end_time) {
                currentByte_ = com_.ReadChar(a);
                if(a){
                    if (count>8){
                        pack_.nameCommand_=currentByte_;
                    }
                    else if (count>7){
                        pack_.status_=currentByte_;
                        if(pack_.status_!=1){
                            return {0,0,pack_.status_,{0,0,0,0,0,0,0,0,0,0}};
                        }
                    }
                    else{
                        p = p + currentByte_;
                        if (count>6){
                            p = p<<8;
                        }
                        else{
                            paramCnt++;
                            if (paramCnt > (int)params.size()+1){
                                return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
                            }
                            if (p==65535){
                                successReceipt_= true;
                                break;}
                            else{
                                params[0] = paramCnt-1;
                                params[paramCnt-1]= p;
                                p=0;
                                count=8;
                            }
                        }
                    }
                    count--;
                }
            }
        }
        else{
            cout<< " Ответа нет или он некорректный"<<endl;
            return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
        }
        if( successReceipt_ == true){
            break;
        }
    }

    // Формирование массива для подсчета CRC
    uint8_t temp_[64] = {pack_.nameCommand_,pack_.status_};
    int j = 1;
    for (int i = 2; i <= 2*params[0]; i= i+2){
        temp_[i] = params[j]>>8;
        temp_[i+1] = params[j];
        j++;
    };

    int crc = Crc8((uint8_t*)&temp_, 2*(uint8_t)params[0]+2);
    //Подсчет CRC
    if(crc!=0){
        cout<< "WrongCheckSum"<<"\t" << crc <<endl;
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    //Формирование параметров
    for(int i=0; i<(int)params.size()-1; i++){
        if(i+1 != params[0])
            pack_.parameters_[i] = params[i+1];
        else {
            pack_.crc_ = params[i+1];
            break;
        }
    }
    return pack_ ;
}

Conserial::UartResponse Conserial::ParsePackege_1_0(unsigned int timeout){
    bool startPackFlag_=false;
    bool successReceipt_=false;
    bool flag_ = 0;
    bool a = 0;
    array <uint16_t, 201>  params  = {0};
    Conserial::UartResponse pack_= {0,0,0,{0}};
    uint8_t currentByte_=0;
    timeout = timeout * 1000;


    if (!com_.IsOpened())
    {
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }
    clock_t end_time = clock() + timeout * (CLOCKS_PER_SEC/1000) ;
    while (clock()<end_time) {
        while (startPackFlag_ != true && clock()<end_time) {
            currentByte_ = com_.ReadChar(a);
            if(a){
                if (currentByte_ == 255){
                    flag_ = 1;
                }
                else{
                    if (flag_ == 1){
                        if (currentByte_ != 254)
                        {
                            flag_ = 0;
                        }
                        else{
                            startPackFlag_ = true;
                            break;
                        }
                    }
                    else {
                        flag_ = 0;
                    }
                }
            }
        }

        if (startPackFlag_ == true){
            int count = 10;
            int paramCnt =1;
            unsigned short p=0;
            while (p != 65535 && clock()<end_time) {
                currentByte_ = com_.ReadChar(a);
                if(a){
                    if (count>9){
                        pack_.status_=currentByte_;
                    }
                    else if (count>8){
                        pack_.nameCommand_=currentByte_;
                    }
                    else if (count>7){
                        pack_.crc_=currentByte_;
                    }
                    else{
                        p = p + currentByte_;
                        if (count>6){ p = p<<8; }
                        else{
                            paramCnt++;
                            if (paramCnt> (int)params.size()+1){
                                return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
                            }
                            if (p==65535){
                                successReceipt_= true;
                                break;}
                            else{
                                params[0] = paramCnt-1;
                                params[paramCnt-1]= p;
                                p=0;
                                count=8;
                            }
                        }
                    }
                    count--;
                }
            }
        }
        else{
            cout<< " Ответа нет или он некорректный"<<endl;
            return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
        }
        if( successReceipt_ == true){
            break;
        }
    }

    //Формирование параметров
    for(int i=0; i<params[0]; i++){
        pack_.parameters_[i] = params[i+1];
    }
    uint16_t temp = pack_.nameCommand_ + pack_.parameters_[0] + pack_.parameters_[1]
                    + pack_.parameters_[2] + pack_.parameters_[3] + pack_.parameters_[4]
                    + pack_.parameters_[5] + pack_.parameters_[6] + pack_.parameters_[7]
                    + pack_.parameters_[8]+ pack_.parameters_[9];
    uint8_t crc = Crc8((uint8_t*)&temp, sizeof(temp));


    //Подсчет CRC

    if(!(crc == pack_.crc_)){
        cout<< "WrongCheckSum"<<"\t" << (int)crc << (int) pack_.crc_<<endl;
        return {0,0,3,{0,0,0,0,0,0,0,0,0,0}};
    }

    return pack_ ;
}

// Функция подсчёта контрольной суммы
uint8_t Conserial::Crc8(uint8_t *pcBlock, uint8_t len)
{
    uint8_t crc = 0xFF;

    while (len--)
        crc = Crc8Table[crc ^ *pcBlock++];
    return crc;
}

uint16_t Conserial::CalcStep(angle_t angle, angle_t rotateStep){

    if (angle < 0){
        angle = angle + 360;
    }

    angle = fmod(angle , 360.0); // Подсчет кратчайшего угла поворота

    int Steps = round (angle / rotateStep); //Подсчёт и округление шагов
    return Steps;
}

WAngles<adc_t> Conserial::CalcSteps(WAngles<angle_t> angles){

    WAngles<adc_t> steps;
    steps.aHalf_ = CalcStep(angles.aHalf_,standOptions.rotateStep_);
    steps.aQuart_ = CalcStep(angles.aQuart_,standOptions.rotateStep_);
    steps.bHalf_ = CalcStep(angles.bHalf_,standOptions.rotateStep_);
    steps.bQuart_ = CalcStep(angles.bQuart_,standOptions.rotateStep_);
    return steps;
}

WAngles<angle_t> Conserial::CalcAngles(WAngles<adc_t> steps)
{
    WAngles<angle_t> angles;
    angles.aHalf_ = ((float)steps.aHalf_) * standOptions.rotateStep_;
    angles.aQuart_ = ((float)steps.aQuart_) * standOptions.rotateStep_;
    angles.bHalf_ = ((float)steps.bHalf_) * standOptions.rotateStep_;
    angles.bQuart_ = ((float)steps.bQuart_) * standOptions.rotateStep_;
    return angles;
}

bool Conserial::StandIsConected (){
    if(!com_.IsOpened())
    {
        com_.Open();
        if(!com_.IsOpened())
            return 0;
    }
    return 1;
};

uint8_t Conserial::CheckStatus(uint8_t status){
    uint8_t errorCode = 3;
    switch (status) {
    case 1:
        errorCode = 0;
        break;//Успех
    case 2:
        cout<<"Количество принятых параметров превышает допустимый предел"<<endl;
        errorCode = 3;
        break;
    case 4:
        cout<<"Необнаружена метка конца пакета"<<endl;
        errorCode = 3;
        break;
    case 8:
        cout<<"Неизвестный ID  Команды"<<endl;
        errorCode = 3;
        break;
    case 16:
        cout<<"Несоответствие CRC"<<endl;
        errorCode = 3;
        break;
    case 17:
        errorCode = 1; //Проблема с подключением
        break;
    case 18:
        errorCode = 2; //Переданы неверные параметры на вход функции
        break;
    case 32:
        cout<<"Не удалось выполнить команду"<<endl;
        errorCode = 3;
        break;
    case 64:
        cout<<"Аппаратная платформа в аварийном состоянии"<<endl;
        errorCode = 3;
        break;
    default: errorCode = 3; //Битый пакет (Не известно)
    }

    return errorCode;
}



}//namespace
