#include "Exception.h"

const Exception* CANNOT_ENABLE_WATER_SOURCE = new Exception(
    "Não é possível ligar a fonte de água com o reservatório abaixo do mínimo");

const Exception* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE = new Exception(
    "Não é possível encher um reservatório sem fonte de água");

const Exception* WATER_TANK_NOT_FOUND = new Exception("Não foi encontrado um reservatório com nome provido");
const Exception* WATER_TANK_ALREADY_REGISTERED = new Exception(
    "Um reservatório com esse nome já foi registrado");

const RuntimeError* WATER_TANK_STOPPED_TO_FILL = new RuntimeError("O reservatório não está mais enchendo");
const RuntimeError* WATER_TANK_IS_NOT_FILLING = new RuntimeError("O reservatório não está enchendo");

const Exception* WATER_SOURCE_NOT_FOUND = new Exception("Não foi encontrado um fonte d'água com nome provido");
const Exception* WATER_SOURCE_ALREADY_REGISTERED = new Exception(
    "Uma fonte d'água com esse nome já foi registrado");

const RuntimeError* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC = new RuntimeError(
    "Não é permitido abrir/fechar uma fonte d'água no modo automático");

const RuntimeError* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME = new RuntimeError(
    "Não é permitido abrir uma fonte d'água porque o reservatório associado está abaixo do nível mínimo");

const RuntimeError* CANNOT_FILL_WATER_TANK_MAX_VOLUME = new RuntimeError(
    "Não é permitido encher o reservatório, nível máximo atingido");

Exception::Exception(const String message) {
    this->message = (String) message;
}

String Exception::getMessage() {
	return this->message;
}

RuntimeError::RuntimeError(const String message) : Exception(message) {

}
