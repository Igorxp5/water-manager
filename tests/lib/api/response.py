from typing import Dict, Type, Any, Union

from google.protobuf.pyext._message import RepeatedCompositeContainer

from .exceptions import APIException, APIRuntimeError, APIInvalidRequest


class APIResponseMessageParser:

    def parse(raw_field):
        raise NotImplementedError


class GetFirstFieldParser(APIResponseMessageParser):
    @staticmethod
    def parse(raw_field):
        list_fields = raw_field.ListFields()
        field_value = raw_field
        if len(list_fields):
            _, field_value = APIResponse.parse_field(list_fields[0])
        else:
            field_value = None
        return field_value


class WaterSourceStateParser(APIResponseMessageParser):
    @staticmethod
    def parse(raw_field):
        field = APIResponse.parse_dict_field(raw_field)
        field.setdefault('pin', 0)
        field.setdefault('active', False)
        field.setdefault('turnedOn', False)
        field.setdefault('sourceWaterTank', None)
        return field


class WaterTankStateParser(APIResponseMessageParser):
    @staticmethod
    def parse(raw_field):
        field = APIResponse.parse_dict_field(raw_field)
        field.setdefault('pressureSensorPin', 0)
        field.setdefault('filling', False)
        field.setdefault('active', False)
        field.setdefault('volumeFactor', 0)
        field.setdefault('pressureFactor', 0)
        field.setdefault('minimumVolume', 0)
        field.setdefault('maxVolume', 0)
        field.setdefault('zeroVolumePressure', 0)
        field.setdefault('pressureChangingValue', 0)
        field.setdefault('rawPressureValue', 0)
        field.setdefault('pressure', 0)
        field.setdefault('volume', 0)
        field.setdefault('waterSource', None)
        return field


class APIResponse:
    GET_FIRST_FIELD_PARSER: APIResponseMessageParser = GetFirstFieldParser()
    MESSAGE_PARSERS: Dict[str, APIResponseMessageParser] = {
        '_TestResponseValue': GET_FIRST_FIELD_PARSER,
        'PrimitiveValue': GET_FIRST_FIELD_PARSER,
        'Value': GET_FIRST_FIELD_PARSER,
        'WaterSourceState': WaterSourceStateParser(),
        'WaterTankState': WaterTankStateParser()
    }

    def __init__(self, id_: int, message: Any):
        self.id = id_
        self.message = message
    
    def __repr__(self):
        return f'{self.__class__.__name__}({self.id}, {repr(self.message)}, {repr(self.error)}))'

    @staticmethod
    def parse(response_pb):
        fields = APIResponse.parse_dict_field(response_pb)
        id_ = fields.get('id', 0)
        message = fields.get('message', None)
        error = fields.get('error', None)
        if error is not None:
            return APIErrorResponse.parse_error(id_, message, error)
        return APIResponse(id_, message)
    
    @staticmethod
    def parse_field(raw_field):
        field_descriptor, field_value = raw_field
        field_name = field_descriptor.name
        field_message_type = field_descriptor.message_type
        if isinstance(field_value, RepeatedCompositeContainer):
            field_value = [APIResponse.parse_field(field_value[i].ListFields()[0])[1] for i in range(len(field_value))]
        elif field_message_type and field_message_type.name:
            parser = APIResponse.MESSAGE_PARSERS.get(field_message_type.name)
            if parser:
                field_value = parser.parse(field_value)
        return field_name, field_value

    @staticmethod
    def parse_dict_field(raw_field):
        fields = dict()
        for field in raw_field.ListFields():
            field_name, field_value = APIResponse.parse_field(field)
            fields[field_name] = field_value
        return fields


class APIErrorResponse(APIResponse):
    EXCEPTIONS_TYPES: Dict[str, Type[APIException]] = {
        0: APIException,
        1: APIRuntimeError,
        2: APIInvalidRequest
    }

    def __init__(self, id_: int, message: str, exception_type: Type, arg: str=None):
        super().__init__(id_, message)
        self.exception_type = exception_type
        self.arg = arg

    @staticmethod
    def parse_error(id_: int, message: str, error: Union[bool, dict]):
        if isinstance(error, bool):  # handling _TestResponse
            return APIErrorResponse(id_, message, APIException)
        message = error.message
        exception_type = APIErrorResponse.EXCEPTIONS_TYPES.get(error.type, 0)
        return APIErrorResponse(id_, message, exception_type, error.arg)
