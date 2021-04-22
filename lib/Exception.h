#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <Arduino.h>

class Exception
{
    public:
        Exception(String message);

        String getMessage();
    
    private:
        String message;
};

class RuntimeError: public Exception
{
    public:
        RuntimeError(String message);
};

// Exceptions
Exception* CANNOT_ENABLE_WATER_SOURCE = new Exception(
    "Não é possível ligar a fonte de água com o reservatório abaixo do mínimo");

Exception* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE = new Exception(
    "Não é possível encher um reservatório sem fonte de água");

Exception* WATER_TANK_NOT_FOUND = new Exception("Não foi encontrado um reservatório com nome provido");
Exception* WATER_TANK_ALREADY_REGISTERED = new Exception(
    "Um reservatório com esse nome já foi registrado");

RuntimeError* WATER_TANK_STOPPED_TO_FILL = new RuntimeError("O reservatório não está mais enchendo");
RuntimeError* WATER_TANK_IS_NOT_FILLING = new RuntimeError("O reservatório não está enchendo");

#endif
