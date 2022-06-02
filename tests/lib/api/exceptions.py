class APIException(RuntimeError):
    def __init__(self, message, arg=None, response=None):
        self.arg = arg
        self.response = response
        super().__init__(message)


class APIInvalidRequest(APIException):
    pass


class APIRuntimeError(APIException):
    pass
