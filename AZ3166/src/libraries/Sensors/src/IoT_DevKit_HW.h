#ifndef _IOT_DEVKIT_HW_H_
#define _IOT_DEVKIT_HW_H_

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief    Initialize the board, include all sensors and Wi-Fi.
     * 
     * @param    isShowInfo - Indicate whether show the initialization progress via the OLED Screen.
     * 
     * @return   0 upon success or other values upon failure:
     *              -100 - Failed to initialize Wi-Fi.
     *              -101 - Failed to initialize I2C for sensors.
     *              -102 - Failed to initialize the gyroscope and accelerator sensor.
     *              -103 - Failed to initialize the humidity and temperature sensor
     *              -104 - Failed to initialize the magnetometer sensor.
     *              -105 - Failed to initialize IrDa.
     *              -106 - Failed to initialize pressure sensor.
    **/
    int initIoTDevKit(int isShowInfo);

    /**
     * @brief    Get the board name, the format is "MXChip IoT DevKit" + serial number of the board.
     *
     * @return   Board name string.
    **/
    const char *getDevKitName(void);

    /**
     * @brief    Get the serial number of the board.
     *
     * @return   Board serial number string.
    **/
    const char *getDevKitSerialNumber(void);

    /**
     * @brief    Retrieve the saved IoT Hub connection string from the STSAFE chip.
     *
     * @return   IoT Hub connection string.
    **/
    const char *getIoTHubConnectionString(void);

    /**
     * @brief    Retrieve the Humidity value from sensor.
     *
     * @return   Humidity value.
    **/
    float getDevKitHumidityValue(void);

    /**
     * @brief    Retrieve the Temperature value from sensor.
     *
     * @param    isFahrenheit - Indicate return in Fahrenheit or Celsius.
     *
     * @return   Temperature value.
    **/
    float getDevKitTemperatureValue(int isFahrenheit);

    /**
     * @brief    Retrieve the Pressure value from sensor.
     *
     * @return   Pressure value.
    **/
    float getDevKitPressureValue(void);

    /**
     * @brief    Retrieve the Magnetometer value from sensor.
     *
     * @return   Magnetometer value.
    **/
    void getDevKitMagnetometerValue(int *x, int *y, int *z);

    /**
     * @brief    Retrieve the Gyroscope value from sensor.
     *
     * @return   Gyroscope value.
    **/
    void getDevKitGyroscopeValue(int *x, int *y, int *z);

    /**
     * @brief    Retrieve the Accelerator value from sensor.
     *
     * @return   Accelerator value.
    **/
    void getDevKitAcceleratorValue(int *x, int *y, int *z);
    
    /**
     * @brief    Turn on the onboard User LED.
    **/
    void turnOnUserLED(void);

    /**
     * @brief    Turn off one onboard LED.
    **/
    void turnOffUserLED(void);

    /**
     * @brief    Turn on the onboard RGB LED.
     *
     * @param    red - The value of red.
     *           green - The value of green.
     *           blue - The value of blue.
    **/
    void turnOnRGBLED(int red, int green, int blue);

    /**
     * @brief    Turn off the onboard RGB LED.
    **/
    void turnOffRGBLED(void);

    /**
     * @brief    Start to blink the onboard User LED.
     *
     * @param    msDuration - How long of the blinking in ms, -1 means infinity. 
     * 
     * @remarks  Once to enable blink, then call the invokeDevKitSensors in a look can blink the User LED.
    **/
    void startBlinkDevKitUserLED(int msDuration);

    /**
     * @brief    Start to blink the onboard RGB LED.
     *
     * @param    msDuration - How long of the blinking in ms, -1 means infinity. 
     * 
     * @remarks  Once to enable blink, then call the invokeDevKitSensors in a look can blink the RGB LED.
    **/
    void startBlinkDevKitRGBLED(int msDuration);

    /**
     * @brief    Write a character string at the specified line on the onboard OLED screen.
     *
     * @param    line - The line number.
     *           s - A pointer to the string to be drawn.
     *           wrap - Indicate wrap the string or not.
    **/
    int textOutDevKitScreen(unsigned int line, const char *s, int wrap);

    /**
     * @brief    Draw an image at the specified position on the onboard OLED screen.
     *
     * @param x0 - Position the X axis of the top left corner of the area image to screen, valid value is [0, 127].
     *        y0 - Position the Y axis of the top left corner of the area image to screen, valid value is [0, 7].
     *        x1 - Position the X axis of the bottom right corner of the area image to screen, valid value is [1, 128].
     *        y1 - Position the Y axis of the bottom right corner of the area image to screen, valid value is [1, 8].
     *        image - Image pixel byte array. Every array element is an 8-bit binary data that draws 8-connected pixels in the same column.
     */
    void drawDevKitScreen(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char *image);

    /**
     * @brief    Clean up the OLED screen.
    **/
    void cleanDevKitScreen(void);

    /**
     * @brief    Retrieve the state of the button A.
     *
     * @return   1 if the button A is down, or 0 is up.
    **/
    int getButtonAState(void);

    /**
     * @brief    Retrieve the state of the button B.
     *
     * @return   1 if the button B is down, or 0 is up.
    **/
    int getButtonBState(void);

    /**
     * @brief    Transmit data via the onboard IrDA.
     *
     * @param    data - A pointer to the data to be sent.
     *           size - Size of the data to be sent.
     *           timeout - Timeout for the transmition in ms.
     * 
     * @return   0 upon success or other values upon failure.
    **/
    int transmitIrDA(unsigned char *data, int size, int timeout);
    
    /**
     * @brief    Execute the commands for onboard sensors and actuators.
     *
     * @remarks  Supported commands:
     *                 Blink RGB LED    - startBlinkDevKitRGBLED
     *                 Blink User LED   - startBlinkDevKitUserLED
    **/
    void invokeDevKitPeripheral(void);

#ifdef __cplusplus
}
#endif // _IOT_DEVKIT_HW_H_

#endif
