#include "bme280.h"
#include "bme280_defs.h"
#include <inttypes.h>
#include <driver/i2c.h>

enum class BME280_ADRESS : uint8_t
{
    PRIM = UINT8_C(0x76),
    SEC = (0x77),
};

struct BME280_I2C
{
    uint8_t addr;
    i2c_port_t i2c_port;
};

class BME280
{
private:
    BME280_I2C intf_ptr;
    struct bme280_dev dev;

public:
    BME280(i2c_port_t i2c_num, BME280_ADRESS adress);
    ~BME280();
    esp_err_t Init(uint32_t *calculatedDelay);
    esp_err_t GetDataAndTriggerNextMeasurement(float *tempDegCel, float *pressurePa, float *relHumidityPercent);
    esp_err_t TriggerNextMeasurement();
};