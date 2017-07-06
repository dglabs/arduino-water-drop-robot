# arduino-water-drop-robot
The automation robot for automatic plants watering or/and drip irrigation from the water tank. Uses hall sensors for water level metering and motorized valve for water flow controlling.
Functions:
- Do irrigation by schedule using RTC;
- Fill-in water tank when empty;
- Open the cover when it's raining; 
- Close the rain cover when water tank is full;
- Measure water volume in liters consumed for irrigation; 
- Display current status;
- Handle 2-key keyboard for manual operation;
- Maximum power save. All peripheral devices have separate power switch. 
Connected devices/sensors:
- Motorized valve for water-out;
- Solenoid valve for water-in;
- LCS Display (1602 or 1604);
- 2-keys keyboard;
- 4-Hall sensor water level meter;
- Water consumed volume counter;
- Rain sensor;
- RTC_DS1307 RTC for timing;
- Step motor for rain cover open/close;
- Solar panel with charger and accumulators;
