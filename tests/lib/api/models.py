import enum

class OperationMode(enum.IntEnum):
    MANUAL = 0
    AUTO = 1

class IOType(enum.IntEnum):
    DIGITAL = 0;
    ANALOGIC = 1;
