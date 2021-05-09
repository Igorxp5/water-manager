import time

def test_serial_communication(arduino):
    raw = b'TESTING\0'
    expected = b'GOT: ' + raw[:-1] + b'\r\n'
    arduino.write(raw)
    assert arduino.read(len(expected)) == expected 
