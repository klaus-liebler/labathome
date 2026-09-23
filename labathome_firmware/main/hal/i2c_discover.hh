#include <esp_err.h>
#include <driver/i2c_master.h>

namespace i2c_discover{
    esp_err_t Discover(const i2c_master_bus_handle_t i2c_master_handle);
}
