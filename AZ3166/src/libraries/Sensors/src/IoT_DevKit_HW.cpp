#include "Arduino.h"
#include "AzureIotHub.h"
#include "AZ3166WiFi.h"
#include "Sensor.h"
#include "SystemVersion.h"
#include "SystemTickCounter.h"
#include "EEPROMInterface.h"

#include "IoT_DevKit_HW.h"

static RGB_LED rgbLed;

static DevI2C *ext_i2c;
static LSM6DSLSensor *acc_gyro;
static HTS221Sensor *ht_sensor;
static LIS2MDLSensor *magnetometer;
static IRDASensor *IrdaSensor;
static LPS22HBSensor *pressureSensor;

static char *connString = NULL;
static const char *boardName = NULL;

// Blink the RGB LED
static volatile uint64_t blinkRGBLEDTimeStart = 0;
static volatile int64_t blinkRGBLEDTime = -1;
static volatile int colorRGBLED = 0;

// Blink the User LED
static volatile uint64_t blinkUserLEDTimeStart = 0;
static volatile int64_t blinkUserLEDTime = -1;
static volatile int userLEDStat = 0;

static struct _tagRGB
{
    int red;
    int green;
    int blue;
} _rgb[] =
    {
        {255, 0, 0},
        {0, 255, 0},
        {0, 0, 255},
};

static bool initWiFi(void)
{
    if (WiFi.begin() == WL_CONNECTED)
    {
        return 0;
    }
    else
    {
        LogError("Failed to initialize Wi-Fi.");
        return -100;
    }
}

int initIoTDevKit(int isShowInfo)
{
    // Init the screen
    Screen.init();

    if (isShowInfo)
    {
        Screen.print(0, "IoT DevKit");
        Screen.print(2, "Initializing...");
    }

    // Serial
    Serial.begin(115200);

    // Init pins
    pinMode(LED_WIFI, OUTPUT);
    pinMode(LED_AZURE, OUTPUT);
    pinMode(LED_USER, OUTPUT);
    pinMode(USER_BUTTON_A, INPUT);
    pinMode(USER_BUTTON_B, INPUT);

    // Turn off the RGB Led
    rgbLed.turnOff();

    // Init I2C bus
    if (isShowInfo)
    {
        Screen.print(3, "  I2C");
    }
    if ((ext_i2c = new DevI2C(D14, D15)) == NULL)
    {
        LogError("Failed to initialize I2C.");
        return -101;
    }
    
    // Init the gyroscope and accelerator sensor
    if (isShowInfo)
    {
        Screen.print(3, "  LSM6DSL");
    }
    if ((acc_gyro = new LSM6DSLSensor(*ext_i2c, D4, D5)) == NULL)
    {
        LogError("Failed to initialize gyroscope and accelerator sensor.");
        return -102;
    }
    acc_gyro->init(NULL);
    acc_gyro->enableAccelerator();
    acc_gyro->enableGyroscope();

    // Init the humidity and temperature sensor
    if (isShowInfo)
    {
        Screen.print(3, "  HTS221");
    }
    if ((ht_sensor = new HTS221Sensor(*ext_i2c)) == NULL)
    {
        LogError("Failed to initialize humidity and temperature sensor.");
        return -103;
    }
    ht_sensor->init(NULL);
    ht_sensor->reset();

    // Init the magnetometer sensor
    if (isShowInfo)
    {
        Screen.print(3, "  LIS2MDL");
    }
    if ((magnetometer = new LIS2MDLSensor(*ext_i2c)) == NULL)
    {
        LogError("Failed to initialize magnetometer sensor.");
        return -104;
    }
    magnetometer->init(NULL);

    // Init IrDA
    if (isShowInfo)
    {
        Screen.print(3, "  IrDA");
    }
    if ((IrdaSensor = new IRDASensor()) == NULL)
    {
        LogError("Failed to initialize IrDa sensor.");
        return -105;
    }
    IrdaSensor->init();

    // Init pressure sensor
    if (isShowInfo)
    {
        Screen.print(3, "  LPS22HB");
    }
    if ((pressureSensor = new LPS22HBSensor(*ext_i2c)) == NULL)
    {
        LogError("Failed to initialize pressure sensor.");
        return -106;
    }
    pressureSensor->init(NULL);

    // Init WiFi
    if (isShowInfo)
    {
        Screen.print(3, "  Wi-Fi");
    }
    return initWiFi();
}

const char *getIoTHubConnectionString(void)
{
    if (connString == NULL)
    {
        uint8_t _connString[AZ_IOT_HUB_MAX_LEN + 1] = {'\0'};
        EEPROMInterface eeprom;
        int ret = eeprom.read(_connString, AZ_IOT_HUB_MAX_LEN, 0, AZ_IOT_HUB_ZONE_IDX);
        if (ret < 0)
        {
            LogError("Unable to get the azure iot connection string from EEPROM. Please set the value in configuration mode.");
            return NULL;
        }
        else if (ret == 0)
        {
            LogError("The connection string is empty.\r\nPlease set the value in configuration mode.");
            return NULL;
        }
        connString = strdup((char*)_connString);
    }
    return connString;
}

const char *getDevKitName(void)
{
    if (boardName == NULL)
    {
        int len = snprintf(NULL, 0, "MXChip IoT DevKit %s", GetBoardID()) + 1;
        boardName = (const char *)malloc(len);
        if (boardName == NULL)
        {
            LogError("No memory");
            return NULL;
        }
        snprintf((char *)boardName, len, "MXChip IoT DevKit %s", GetBoardID());
    }
    return boardName;
}

const char *getDevKitSerialNumber(void)
{
    return GetBoardID();
}

float getDevKitHumidityValue(void)
{
    float humidity = 0;
    ht_sensor->getHumidity(&humidity);
    return humidity;
}

float getDevKitTemperatureValue(int isFahrenheit)
{
    float temperature = 0;
    ht_sensor->getTemperature(&temperature);
    if (isFahrenheit)
    {
        //convert from C to F
        temperature = temperature * 1.8 + 32;
    }
    return temperature;
}

float getDevKitPressureValue(void)
{
    float pressure = 0;
    pressureSensor->getPressure(&pressure);
    return pressure;
}

void getDevKitMagnetometerValue(int *x, int *y, int *z)
{
    int axes[3];
    magnetometer->getMAxes(axes);
    *x = axes[0];
    *y = axes[1];
    *z = axes[2];
}

void getDevKitGyroscopeValue(int *x, int *y, int *z)
{
    int axes[3];
    acc_gyro->getGAxes(axes);
    *x = axes[0];
    *y = axes[1];
    *z = axes[2];
}

void getDevKitAcceleratorValue(int *x, int *y, int *z)
{
    int axes[3];
    acc_gyro->getXAxes(axes);
    *x = axes[0];
    *y = axes[1];
    *z = axes[2];
}

void turnOnUserLED(void)
{
    digitalWrite(LED_USER, 1);
}

void turnOffUserLED(void)
{
    digitalWrite(LED_USER, 0);
}

void turnOnRGBLED(int red, int green, int blue)
{
    rgbLed.setColor(red, green, blue);
}

void turnOffRGBLED(void)
{
    rgbLed.turnOff();
}

void startBlinkDevKitUserLED(int msDuration)
{
    blinkUserLEDTimeStart = SystemTickCounterRead();
    blinkUserLEDTime = (msDuration == -1 ? 0x7fffffffffffffff : msDuration);
    userLEDStat = 1;
    digitalWrite(LED_USER, userLEDStat);
}

void startBlinkDevKitRGBLED(int msDuration)
{
    blinkRGBLEDTimeStart = SystemTickCounterRead();
    blinkRGBLEDTime = (msDuration == -1 ? 0x7fffffffffffffff : msDuration);
    colorRGBLED = 0;
    rgbLed.setColor(_rgb[colorRGBLED].red, _rgb[colorRGBLED].green, _rgb[colorRGBLED].blue);
}

int textOutDevKitScreen(unsigned int line, const char *s, int wrap)
{
    return Screen.print(line, s, (bool)wrap);
}

void drawDevKitScreen(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1, unsigned char *image)
{
    Screen.draw(x0, y0, x1, y1, image);
}

void cleanDevKitScreen(void)
{
    Screen.clean();
}

int getButtonAState(void)
{
    return (digitalRead(USER_BUTTON_A) == LOW ? 1 : 0);
}

int getButtonBState(void)
{
    return (digitalRead(USER_BUTTON_B) == LOW ? 1 : 0);
}

int transmitIrDA(unsigned char *data, int size, int timeout)
{
    return IrdaSensor->IRDATransmit(data, size, timeout);
}

static void _blinkRGBLED(uint64_t msNow)
{
    if (blinkRGBLEDTime <= 0)
    {
        return;
    }

    uint64_t ms = msNow - blinkRGBLEDTimeStart;
    if (ms >= 500)
    {
        // Change the color
        colorRGBLED = (colorRGBLED + 1) % (sizeof(_rgb) / sizeof(struct _tagRGB));
        rgbLed.setColor(_rgb[colorRGBLED].red, _rgb[colorRGBLED].green, _rgb[colorRGBLED].blue);
        
        blinkRGBLEDTime -= ms;
        if (blinkRGBLEDTime <= 0)
        {
            // End
            rgbLed.turnOff();
        }
        else
        {
            blinkRGBLEDTimeStart = msNow;
        }
    }
}

static void _blinkUserLED(uint64_t msNow)
{
    if (blinkUserLEDTime < 0)
    {
        return;
    }

    uint64_t ms = msNow - blinkUserLEDTimeStart;
    if (ms >= 500)
    {
        // Flip
        userLEDStat = !userLEDStat;
        digitalWrite(LED_USER, userLEDStat);

        blinkUserLEDTime -= ms;
        if (blinkRGBLEDTime <= 0)
        {
            // End
            digitalWrite(LED_USER, 0);
        }
        else
        {
            blinkUserLEDTimeStart = msNow;
        }
    }
}

void invokeDevKitPeripheral(void)
{
    uint64_t msNow = SystemTickCounterRead();

    _blinkRGBLED(msNow);

    _blinkUserLED(msNow);
}