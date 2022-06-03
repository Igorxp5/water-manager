class APIException(RuntimeError):
    def __init__(self, message, arg=None, response=None):
        self.arg = arg
        self.message = message
        self.response = response
        super().__init__(message)
    
    def __repr__(self):
        return f'{self.__class__.__name__}({repr(self.message)}, {repr(self.arg)})'


class APIInvalidRequest(APIException):
    pass


class APIRuntimeError(APIException):
    pass
