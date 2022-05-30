class APIException(RuntimeError):
    def __init__(self, message, response=None):
        self.response = response
        super().__init__(message)


class APIInvalidRequest(APIException):
    pass


class APIRuntimeError(APIException):
    pass
