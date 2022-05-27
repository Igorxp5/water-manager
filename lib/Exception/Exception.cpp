#include "Exception.h"

const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE = new InvalidRequest(
    "Não é possível ligar a fonte de água com o reservatório abaixo do mínimo");

const InvalidRequest* CANNOT_FILL_WATER_TANK_WITHOUT_WATER_SOURCE = new InvalidRequest(
    "Não é possível encher um reservatório sem fonte de água");

const RuntimeError* WATER_TANK_NOT_FOUND = new RuntimeError("Não foi encontrado um reservatório com nome provido");
const RuntimeError* WATER_TANK_ALREADY_REGISTERED = new RuntimeError(
    "Um reservatório com esse nome já foi registrado");

const RuntimeError* WATER_TANK_STOPPED_TO_FILL = new RuntimeError("O reservatório não está mais enchendo");
const RuntimeError* WATER_TANK_IS_NOT_FILLING = new RuntimeError("O reservatório não está enchendo");

const RuntimeError* WATER_SOURCE_NOT_FOUND = new RuntimeError("Não foi encontrado um fonte d'água com nome provido");
const RuntimeError* WATER_SOURCE_ALREADY_REGISTERED = new RuntimeError(
    "Uma fonte d'água com esse nome já foi registrado");

const InvalidRequest* CANNOT_HANDLE_WATER_SOURCE_IN_AUTOMATIC = new InvalidRequest(
    "Não é permitido abrir/fechar uma fonte d'água no modo automático");

const InvalidRequest* CANNOT_ENABLE_WATER_SOURCE_DUE_MINIMUM_VOLUME = new InvalidRequest(
    "Não é permitido abrir uma fonte d'água porque o reservatório associado está abaixo do nível mínimo");

const InvalidRequest* CANNOT_FILL_WATER_TANK_MAX_VOLUME = new InvalidRequest(
    "Não é permitido encher o reservatório, nível máximo atingido");

const InvalidRequest* MAX_LENGTH_ERROR = new InvalidRequest("O nome deve ter no máximo 20 caracteres");
const InvalidRequest* MAX_WATER_SOURCES_ERROR = new InvalidRequest("Máximo de fontes d'água alcançado");
const InvalidRequest* MAX_WATER_TANKS_ERROR = new InvalidRequest("Máximo de reservatórios alcançado");

Exception::Exception(const char* message) {
    this->message = message;
}

RuntimeError::RuntimeError(const char* message) : Exception(message) {

}

InvalidRequest::InvalidRequest(const char* message) : Exception(message) {

}

const char* Exception::getMessage() {
	return this->message;
}
