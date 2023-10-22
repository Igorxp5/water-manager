# water-manager

The water-manager project is a C++ firmware for Arduino that controls the water level in residential water tanks by opening and closing the water sources.

## Project Structure

The project is organized into different modules to handle various aspects of the water tank management system. The main components of the project are as follows:

1. **Arduino Firmware**: The core logic of the water tank management system is implemented in C++ and runs on an Arduino device. The firmware controls the water sources and tanks based on the min/max water levels set up. The platform is also able to connect water tanks to other water tanks.

2. **Python Tests**: The project includes test scripts written in Python to verify the functionality of the Arduino firmware. These tests simulate different scenarios and ensure that the system behaves as expected.

3. **Tkinter GUI**: To interact with the Arduino and send commands, a Tkinter GUI interface has been developed. The GUI, located at `tests/lib/gui.py`, allows users to control the water sources and tanks by sending commands to the Arduino.

## Tkinter GUI

### Installation

1. Clone the project repository:

```bash
git clone https://github.com/Igorxp5/water-manager.git
```

2. Install the required dependencies by running the following command:

```bash
pip install -r tests/requirements.txt
```

### How to run it

To use the Water Manager Arduino project, follow these steps:

1. Connect your Arduino board to your computer.

2. Upload the project firmware into your Arduino board. You can find compiled version in the repo releases page.

2. Open the Tkinter GUI by running the following command:

```bash
python tests/lib/gui.py
```


## Project Requirements

The water-manager project aims to fulfill the following requirements:

### Water Tank Control

1. **Max Volume Limit**: The platform should stop filling a water tank when it reaches its maximum volume.

2. **Min Volume Limit**: The platform should fill a water tank when its water level is below the minimum volume.

3. **Source Min Volume Limit**: The platform should stop filling a water tank if its water source reaches its minimum volume.

4. **Runtime Error**: The platform should send a runtime error message when a water tank should be filling but is not.

5. **Runtime Error**: The platform should send a runtime error message when a water tank stops filling before reaching its maximum volume.

6. **Water Source Timing**: The platform should keep the water source turned on/off for at least 1 minute to avoid multiple filling calls.

7. **Error Interpolation**: The platform should send errors in 10-second intervals and interpolate the error of each water tank.

8. **Deactivation on Inactivity**: The platform should deactivate a water tank when it has stopped filling for 10 minutes.

9. **Deactivation on Inactivity**: The platform should deactivate a water tank when it is not filling for 10 minutes.

10. **Memory Management**: The platform should have enough memory to create the maximum number of water sources and water tanks. It should deallocate IOInterface instances when there are no water sources or water tanks using them to avoid memory leaks.

11. **EEPROM Persistence**: The platform should be able to save all resources created in the EEPROM and load them when it boots.

### Operation Modes and Reset

12. **Operation Mode**: The platform should be able to set and get the current operation mode. It should default to manual mode when the API is reset.

13. **API Reset**: When the API is reset, the platform should turn off all water sources before resetting.

14. **Concurrent Requests**: The platform should be able to receive multiple requests at the same time and answer both of them.

15. **Invalid Request Handling**: The platform should return an error response message when an invalid request is provided.

16. **Truncated Message Handling**: The platform should be able to drop bytes when an incomplete message arrives and respond with "Truncated message received".

17. **Large Request Handling**: The platform should not crash when receiving a large request and should respond with an error for an invalid message type in the request.

18. **Long Overflow**: The platform should keep receiving requests even after 50 days (long overflow).

19. **Reset with Dependencies**: The platform should be able to perform a reset even when there are water tank/source dependencies.

### IO Platform Testing

*Only in test release firmware.*

20. **CreateIO Error**: The platform should return an error when it receives a CreateIO request for an already set pin.

21. **TestIO Values**: The platform should be able to set and get values for TestIO.

22. **Undefined Pin Error**: The platform should answer with an error when setting or getting an undefined pin.

23. **Delete TestIOs**: The platform should be able to delete all set TestIOs.

24. **Time Mocking**: The platform should be able to mock time for testing purposes.

25. **Long Overflow Mocking**: The platform should be able to mock a long overflow on millis.

26. **Physical and Virtual IO**: The platform should be able to switch between physical and virtual IO.

### Water Source Management

27. **Create Water Source**: The platform should be able to create a water source and get the list of created water sources. It should not allow the creation of more than 5 water sources.

28. **Duplicate Water Source**: The platform should respond with an error when the user tries to create a water source with a name that is already registered.

29. **Water Source Name Limit**: The platform should not allow more than 20 characters to name a water source, as it would result in an undecodable request.

30. **Remove Water Source**: The platform should be able to remove a previously created water source. After removing a water source, the memory allocated in RAM for it should be deallocated. The IOs created for the water source should be kept.

31. **Remove Nonexistent Water Source**: The platform should be able to remove a water source that does not exist.

32. **Water Source with Water Tank**: The platform should be able to create a water source using a water tank as a source.

33. **Water Tank Dependency**: The platform should not allow the removal of a water source if it is associated with a water tank.

34. **Get Water Source**: The platform should be able to get information about a registered water source. It should respond with an error when trying to get an invalid water source.

35. **Water Source Control**: The platform should allow turning on/off water sources when it's in manual mode but should not allow it when it's in auto mode.

36. **Invalid Water Source Name**: The platform should respond with an error when trying to create a water source with the name of a water tank that does not exist.

37. **Water Source Activation**: The platform should be able to turn on a water source even if its water tank is below the minimum volume or the water source is deactivated.

38. **Water Source Deactivation**: The platform should be able to activate and deactivate a water source. It should respond with an error when trying to turn on a deactivated water source. The platform should turn off the water source when it's deactivated.

39. **Empty Water Source Name**: The platform should respond with an error when trying to create a water source without a name.

### Water Tank Management

40. **Create Water Tank**: The platform should be able to create a water tank and get the list of created water tanks. It should not allow the creation of more than 5 water tanks.

41. **Duplicate Water Tank**: The platform should respond with an error when the user tries to create a water tank with a name that is already registered.

42. **Water Tank Name Limit**: The platform should not allow more than 20 characters to name a water tank, as it would result in an undecodable request.

43. **Remove Water Tank**: The platform should be able to remove a previously created water tank. After removing a water tank, the memory allocated in RAM for it should be deallocated. The IOs created for the water tank should be kept.

44. **Remove Nonexistent Water Tank**: The platform should be able to remove a water tank that does not exist.

45. **Get Water Tank**: The platform should be able to get information about a registered water tank. It should respond with an error when trying to get an invalid water tank.

46. **Water Tank with Water Source**: The platform should be able to create a water tank associated with a water source.

47. **Water Source Dependency**: The platform should not allow the removal of a water tank if it is associated with a water source.

48. **Set Water Tank Parameters**: The platform should be able to set the minimum volume, maximum volume, volume factor, pressure factor, zero volume factor, and pressure changing value for a water tank.

49. **Invalid Water Tank Filling**: The platform should respond with an error when trying to fill an invalid water tank manually in manual mode.

50. **Invalid Water Tank Stopping**: The platform should respond with an error when trying to stop filling an invalid water tank manually in auto mode.

51. **Auto Mode Water Tank Filling**: The platform should respond with an error when trying to fill a water tank manually in auto mode.

52. **Manual Mode Water Tank Filling**: The platform should be able to fill a water tank manually in manual mode.

53. **Manual Mode Water Tank Stopping**: The platform should respond with an error when trying to stop filling a water tank manually in auto mode.

54. **Water Tank Volume Limit**: The platform should respond with an error when trying to fill a water tank with a volume that is already at its maximum allowed.

55. **Force Filling**: The platform should be able to fill a water tank even when its volume reaches the maximum value allowed if the force flag is present.

56. **Water Tank Without Source**: The platform should respond with an error when trying to fill a water tank without a water source.

57. **Water Tank Without Source Filling**: The platform should not be able to fill a water tank when it doesn't have a water source, even if the force flag is present.

58. **Water Tank Activation**: The platform should be able to activate and deactivate a water tank. It should respond with an error when trying to fill a deactivated water tank. The platform should stop filling the water tank when it's deactivated.

59. **Empty Water Tank Name**: The platform should respond with an error when trying to create a water tank without a name.

## License

The Water Manager Arduino project is licensed under the [GNU GPLv3](LICENSE).