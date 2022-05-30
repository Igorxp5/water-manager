from typing import Dict, Type

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
        field.setdefault('enabled', False)
        field.setdefault('sourceWaterTank', None)
        return field


class APIResponse:
    ERROR_EXCEPTIONS: Dict[str, Type[APIException]] = {
        'True': APIException,
        '0': APIException,
        '1': APIRuntimeError,
        '2': APIInvalidRequest
    }
    GET_FIRST_FIELD_PARSER: APIResponseMessageParser = GetFirstFieldParser()
    MESSAGE_PARSERS: Dict[str, APIResponseMessageParser] = {
        '_TestResponseValue': GET_FIRST_FIELD_PARSER,
        'PrimitiveValue': GET_FIRST_FIELD_PARSER,
        'Value': GET_FIRST_FIELD_PARSER,
        'WaterSourceState': WaterSourceStateParser()
    }

    def __init__(self, id_: int, message: str, error: APIException = None):
        self.id = id_
        self.message = message
        self.error = error
    
    def __repr__(self):
        return f'{self.__class__.__name__}({self.id}, {repr(self.message)}, {repr(self.error)}))'

    @staticmethod
    def parse(response_pb):
        fields = APIResponse.parse_dict_field(response_pb)
        id_ = fields.get('id', 0)
        message = fields.get('message', '')
        error = fields.get('error', None)
        error = APIResponse.ERROR_EXCEPTIONS.get(str(error))
        return APIResponse(id_, message, error)
    
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
