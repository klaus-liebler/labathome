#pragma once
#include <i2c_sensor.hh>
#include <common-esp32.hh>
#define TAG "IP5306"
#include <esp_log.h>


namespace IP5306
{
    constexpr uint8_t ADDRESS{0x75};
    namespace R
    {
        constexpr uint8_t SYS_CTL0{0x00};
        constexpr uint8_t SYS_CTL1{0x01};
        constexpr uint8_t SYS_CTL2{0x02};

        constexpr uint8_t Charger_CTL0{0x20};
        constexpr uint8_t Charger_CTL1{0x21};
        constexpr uint8_t Charger_CTL2{0x22};
        constexpr uint8_t Charger_CTL3{0x23};
        constexpr uint8_t Charger_CTL4{0x24};

        constexpr uint8_t READ0{0x70};
        constexpr uint8_t READ1{0x71};
        constexpr uint8_t READ2{0x72};
        constexpr uint8_t READ3{0x77};
        constexpr uint8_t BAT_LEVEL{0x78};

        struct SYS_CTL0_t{
            union
            {
                struct
                {
                    bool BUTTON_SHUTDOWN : 1;
                    bool SET_BOOST_OUTPUT_ENABLE : 1;
                    bool POWER_ON_LOAD : 1;
                    bool RSVD : 1;
                    bool CHARGER_ENABLE : 1;
                    bool BOOST_ENABLE : 1;
                    bool RSVD2 : 2;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct SYS_CTL1_t{
            union
            {
                struct
                {
                    bool LOW_BATTERY_SHUTDOWN_ENABLE : 1;
                    uint8_t RSVD : 1;
                    bool BOOST_AFTER_VIN : 1;
                    uint8_t RSVD2 : 2;
                    bool SHORT_PRESS_BOOST_SWITCH_ENABLE : 1;
                    bool FLASHLIGHT_CTRL_SIGNAL_SELECTION : 1;
                    bool BOOST_CTRL_SIGNAL_SELECTION : 1;
                } bits;

                uint8_t reg_byte;
            };
        };

        enum class eSHUTDOWN_TIME
        {
            T8s = 0,
            T32s = 1,
            T16s = 2,
            T64s = 3,
        };

        enum class eKEY_LONG_PRESS_TIME
        {
            T2s = 0,
            T3s = 1,
        };

        struct SYS_CTL2_t
        {
            union
            {
                struct
                {
                    uint8_t RSVD : 2;
                    eSHUTDOWN_TIME LIGHT_LOAD_SHUTDOWN_TIME : 2;
                    eKEY_LONG_PRESS_TIME LONG_PRESS_TIME : 1;
                    uint8_t RSVD2 : 3;
                } bits;

                uint8_t reg_byte;
            };
        };

        enum class eCUT_OFF_VOLTAGE{
            /*CUT-OFF VOLTAGE RANGE*/
            CUT_OFF_VOLTAGE_0 = 0, // 4.14/4.26/4.305/4.35  V
            CUT_OFF_VOLTAGE_1 = 1, // 4.17/4.275/4.32/4.365 V
            CUT_OFF_VOLTAGE_2 = 2, // 4.185/4.29/4.335/4.38 V
            CUT_OFF_VOLTAGE_3 = 3, // 4.2/4.305/4.35/4.395  V
        };       

        struct Charger_CTL0_t{
            union{
                struct{
                    eCUT_OFF_VOLTAGE CHARGING_FULL_STOP_VOLTAGE : 2;
                    uint8_t RSVD : 6;
                } bits;
                uint8_t reg_byte;
            };
        };

        enum class eCHARGING_VOLTAGE
        {
            VOUT_4_45 = 0, // 4.45
            VOUT_4_50 = 1, // 4.5
            VOUT_4_55 = 2, // 4.55
            VOUT_4_60 = 3, // 4.6
            VOUT_4_65 = 4, // 4.65
            VOUT_4_70 = 5, // 4.7
            VOUT_4_75 = 6, // 4.75
            VOUT_4_80 = 7, // 4.8
        };

        enum class eBATTERY_STOP_CHARGING_CURRENT
        {
            CURRENT_200 = 0,
            CURRENT_400 = 1,
            CURRENT_500 = 2,
            CURRENT_600 = 3,
        };

        struct Charger_CTL1_t
        {
            union
            {
                struct
                {
                    uint8_t RSVD : 2;
                    eCHARGING_VOLTAGE CHARGE_UNDER_VOLTAGE_LOOP : 3;
                    uint8_t RSVD2 : 1;
                    eBATTERY_STOP_CHARGING_CURRENT END_CHARGE_CURRENT_DETECTION : 2;
                } bits;

                uint8_t reg_byte;
            };
        };

        enum class eBATTERY_VOLTAGE{
            V4_40 = 3, // 4.4
            V4_35 = 2, // 4.35
            V4_30 = 1, // 4.3
            V4_20 = 0, // 4.2
        };

        enum class eVOLTAGE_PRESSURE{
            /*Voltage Pressure setting*/
            P42 = 3,
            P28 = 2,
            P14 = 1,
            P0 = 0,
        };

        struct Charger_CTL2_t{
            union{
                struct{
                    eVOLTAGE_PRESSURE VOLTAGE_PRESSURE : 2;
                    eBATTERY_VOLTAGE BATTERY_VOLTAGE : 2;
                    uint8_t RSVD : 4;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct Charger_CTL3_t{
            union{
                struct{
                    uint8_t RSVD : 5;
                    bool CHARGE_CC_LOOP : 1;
                    uint8_t RSVD2 : 2;
                } bits;
                uint8_t reg_byte;
            };
        };

        struct Charger_CTL4_t{
            union
            {
                struct
                {
                    uint8_t VIN_CURRENT : 5;
                    uint8_t RSVD : 3;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct READ0_t
        {
            union
            {
                struct
                {
                    uint8_t RSVD : 3;
                    bool CHARGE_ENABLE : 1;
                    uint8_t RSVD2 : 4;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct READ1_t
        {
            union
            {
                struct
                {
                    uint8_t RSVD : 3;
                    bool BATTERY_STATUS : 1;
                    uint8_t RSVD2 : 4;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct READ2_t
        {
            union
            {
                struct
                {
                    uint8_t RSVD : 2;
                    bool LOAD_LEVEL : 1;
                    uint8_t RSVD2 : 5;
                } bits;

                uint8_t reg_byte;
            };
        };

        struct READ3_t
        {
            union
            {
                struct
                {
                    bool SHORT_PRESS_DETECT : 1;
                    bool LONG_PRESS_DETECT : 1;
                    bool DOUBLE_PRESS_DETECT : 1;
                    uint8_t RSVD : 5;
                } bits;

                uint8_t reg_byte;
            };
        };
    }

    class M : public I2CSensor
    {
    private:
        IP5306::R::SYS_CTL0_t SYS_CTL0;
        IP5306::R::SYS_CTL1_t SYS_CTL1;
        IP5306::R::SYS_CTL2_t SYS_CTL2;
        IP5306::R::Charger_CTL0_t Charger_CTL0;
        IP5306::R::Charger_CTL1_t Charger_CTL1;
        IP5306::R::Charger_CTL2_t Charger_CTL2;
        IP5306::R::Charger_CTL3_t Charger_CTL3;
        IP5306::R::Charger_CTL4_t Charger_CTL4;
        R::READ0_t R0;
        R::READ1_t R1;
        R::READ2_t R2;
        R::READ3_t R3;
        uint8_t batLevel;

        
        void SynchronizeAllRegisters(){
            this->ReadRegs8(R::SYS_CTL0, &SYS_CTL0.reg_byte, 1);
            this->ReadRegs8(R::SYS_CTL1, &SYS_CTL1.reg_byte, 1);
            this->ReadRegs8(R::SYS_CTL2, &SYS_CTL2.reg_byte, 1);

            this->ReadRegs8(R::Charger_CTL0, &Charger_CTL0.reg_byte, 1);
            this->ReadRegs8(R::Charger_CTL1, &Charger_CTL1.reg_byte, 1);
            this->ReadRegs8(R::Charger_CTL2, &Charger_CTL2.reg_byte, 1);
            this->ReadRegs8(R::Charger_CTL3, &Charger_CTL3.reg_byte, 1);
            this->ReadRegs8(R::Charger_CTL4, &Charger_CTL4.reg_byte, 1);
            this->ReadRegs8(R::READ0, &R0.reg_byte, 1);
            this->ReadRegs8(R::READ1, &R1.reg_byte, 1);
            this->ReadRegs8(R::READ2, &R2.reg_byte, 1);
            this->ReadRegs8(R::READ3, &R3.reg_byte, 1);
            this->ReadRegs8(R::BAT_LEVEL, &batLevel, 1);
        }
        

    public:
        M(i2c_master_bus_handle_t bus_handle) : I2CSensor(bus_handle, ADDRESS)
        {
        }

        ErrorCode Initialize(int64_t &waitTillFirstTrigger) override
        {
            /*initialize register data*/
            SynchronizeAllRegisters();
            return ErrorCode::OK;
        }

        size_t FormatJSON(char* buffer, size_t maxLen){
            size_t used=0;
            used += snprintf(buffer, maxLen-used, "{\"percentage\":%d,\"status\":\"%s\"}",
                batLevel2Percentage(),
                R1.bits.BATTERY_STATUS?"Already Full":"Still charging"
            );
            return used;
        }

        void LogSettings()
        {
            SynchronizeAllRegisters();
            ESP_LOGI(TAG, "IP5306 Settings (now updated):");
            ESP_LOGI(TAG, "  CHARGE_ENABLE: %s", R0.bits.CHARGE_ENABLE?"Yes":"No"); 
            ESP_LOGI(TAG, "  Is Battery Full: %s", R1.bits.BATTERY_STATUS?"Already Full":"Still charging"); 
            ESP_LOGI(TAG, "  Light Load: %s", R2.bits.LOAD_LEVEL?"Light Load":"Heavy Load");
            ESP_LOGI(TAG, "  Battery Percentage: %d", batLevel2Percentage()); 
            ESP_LOGI(TAG, "  KeyOff: %s", SYS_CTL0.bits.BUTTON_SHUTDOWN?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  BoostOutput: %s",SYS_CTL0.bits.SET_BOOST_OUTPUT_ENABLE?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  PowerOnLoad: %s", SYS_CTL0.bits.POWER_ON_LOAD?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  Charger: %s", SYS_CTL0.bits.CHARGER_ENABLE?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  Boost: %s", SYS_CTL0.bits.BOOST_ENABLE?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  LowBatShutdown: %s", SYS_CTL1.bits.LOW_BATTERY_SHUTDOWN_ENABLE?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  ShortPressBoostSwitch: %s", SYS_CTL1.bits.SHORT_PRESS_BOOST_SWITCH_ENABLE?"Enabled":"Disabled");
            ESP_LOGI(TAG, "  FlashlightClicks: %s", SYS_CTL1.bits.FLASHLIGHT_CTRL_SIGNAL_SELECTION?"Long Press":"Double Press");
            ESP_LOGI(TAG, "  BoostOffClicks: %s", SYS_CTL1.bits.BOOST_CTRL_SIGNAL_SELECTION?"Double Press":"Long Press");
            ESP_LOGI(TAG, "  BoostAfterVin: %s", SYS_CTL1.bits.BOOST_AFTER_VIN?"Open":"Not Open");
            ESP_LOGI(TAG, "  LongPressTime: %s", SYS_CTL2.bits.LONG_PRESS_TIME==R::eKEY_LONG_PRESS_TIME::T3s?"3s":"2s");
            ESP_LOGI(TAG, "  ChargeUnderVoltageLoop: %.2fV", 4.45 + (uint8_t)Charger_CTL1.bits.CHARGE_UNDER_VOLTAGE_LOOP * 0.05);
            ESP_LOGI(TAG, "  ChargeCCLoop: %s", Charger_CTL3.bits.CHARGE_CC_LOOP?"Vin":"Bat");
            ESP_LOGI(TAG, "  VinCurrent: %dmA", (Charger_CTL4.bits.VIN_CURRENT * 100) + 50);
            ESP_LOGI(TAG, "  VoltagePressure: %dmV", (int)Charger_CTL2.bits.VOLTAGE_PRESSURE*14);
            ESP_LOGI(TAG, "  ChargingFullStopVoltage: %u", (int)Charger_CTL0.bits.CHARGING_FULL_STOP_VOLTAGE);
            ESP_LOGI(TAG, "  LightLoadShutdownTime: %u", (int)SYS_CTL2.bits.LIGHT_LOAD_SHUTDOWN_TIME);
            ESP_LOGI(TAG, "  EndChargeCurrentDetection: %u", (int)Charger_CTL1.bits.END_CHARGE_CURRENT_DETECTION);
            ESP_LOGI(TAG, "  ChargeCutoffVoltage: %u", (int)Charger_CTL2.bits.BATTERY_VOLTAGE);
        }
        ErrorCode Trigger(int64_t &waitTillReadout) override
        {
            waitTillReadout = 2000;
            return ErrorCode::OK;
        }
        ErrorCode Readout(int64_t &waitTillNextTrigger) override
        {
            this->ReadRegs8(R::BAT_LEVEL, &this->batLevel, 1);
            this->ReadRegs8(R::READ1, &this->R1.reg_byte, 1);
            
            waitTillNextTrigger = 2000;
            return ErrorCode::OK;
        }
        /*@brief  select boost mode
              @param  enable or disable bit
              @retval None
              @note   Default value - enable
            */
        void boost_mode(bool boost_en)
        {
            SYS_CTL0.bits.BOOST_ENABLE = boost_en;

            this->WriteReg8(R::SYS_CTL0, SYS_CTL0.reg_byte);
        }

        /*@brief  select charger mode
          @param  enable or disable bit
          @retval None
          @note   Default value - enable
        */
        ErrorCode charger_mode(bool charger_en)
        {
            SYS_CTL0.bits.CHARGER_ENABLE = charger_en;
            return this->WriteReg8(R::SYS_CTL0, SYS_CTL0.reg_byte);
        }

        /*@brief  select auto power on once load detected
          @param  enable or disable bit
          @retval None
          @note   Default value - enable
        */
        ErrorCode power_on_load(bool power_on_en)
        {
            SYS_CTL0.bits.POWER_ON_LOAD = power_on_en;
            return this->WriteReg8(R::SYS_CTL0, SYS_CTL0.reg_byte);
        }

        /*@brief  boost o/p normally open function
          @param  enable or disable bit
          @retval None
          @note   Default value - enable
        */
        ErrorCode boost_output(bool output_val)
        {
            SYS_CTL0.bits.SET_BOOST_OUTPUT_ENABLE = output_val;
            return this->WriteReg8(R::SYS_CTL0, SYS_CTL0.reg_byte);
        }

        /*@brief  enter shutdown mode using button
          @param  enable or disable bit
          @retval None
          @note   Default value - disable
        */
        ErrorCode button_shutdown(bool shutdown_val)
        {
            SYS_CTL0.bits.BUTTON_SHUTDOWN = shutdown_val;
            return this->WriteReg8(R::SYS_CTL0, SYS_CTL0.reg_byte);
        }

        /*@brief  boost control mode using button
          @param  enable or disable bit
          @retval None
          @note   Default value - disable
        */
        ErrorCode boost_ctrl_signal(bool press_val)
        {
            SYS_CTL1.bits.BOOST_CTRL_SIGNAL_SELECTION = press_val;
            return this->WriteReg8(R::SYS_CTL1, SYS_CTL1.reg_byte);
        }

        /*@brief  keep boost mode on after input supply removal
          @param  enable or disable bit
          @retval None
          @note   Default value - enable
        */
        ErrorCode boost_after_vin(bool val)
        {
            SYS_CTL1.bits.BOOST_AFTER_VIN = val;

            return this->WriteReg8(R::SYS_CTL1, SYS_CTL1.reg_byte);
        }

        /*@brief  shutdown if battery voltage raches 3V
          @param  enable or disable bit
          @retval None
          @note   Default value - enable
        */
        ErrorCode set_low_battery_shutdown(bool shutdown_en)
        {
            SYS_CTL1.bits.LOW_BATTERY_SHUTDOWN_ENABLE = shutdown_en;

            return this->WriteReg8(R::SYS_CTL1, SYS_CTL1.reg_byte);
        }

        /*@brief  set light load shutdown timing
          @param  shutdown time value - 0 for 8s, 1 for 32s, 2 for 16s and 3 for 64s
          @retval None
          @note   Default value - disable
        */
        ErrorCode set_light_load_shutdown_time(R::eSHUTDOWN_TIME shutdown_time)
        {
            SYS_CTL2.bits.LIGHT_LOAD_SHUTDOWN_TIME = shutdown_time;

            return this->WriteReg8(R::SYS_CTL2, SYS_CTL2.reg_byte);
        }

        /*@brief  set charging cutoff voltage range for battery
          @param  voltage range value - 0 for 4.14/4.26/4.305/4.35  V
                                        1 for 4.17/4.275/4.32/4.365 V
                                        2 for 4.185/4.29/4.335/4.38 V
                                        3 for 4.2/4.305/4.35/4.395  V
          @retval None
          @note   Default value - 2
        */
        ErrorCode set_charging_stop_voltage(R::eCUT_OFF_VOLTAGE voltage_val)
        {
            Charger_CTL0.bits.CHARGING_FULL_STOP_VOLTAGE = voltage_val;

            return this->WriteReg8(R::Charger_CTL0, Charger_CTL0.reg_byte);
        }

        /*@brief  set charging complete current detection
          @param  current value - 3 : 600mA
                                  2 : 500mA
                                  1 : 400mA
                                  0 : 200mA

          @retval None
          @note   Default value - 1
        */
        ErrorCode end_charge_current(R::eBATTERY_STOP_CHARGING_CURRENT current_val)
        {
            Charger_CTL1.bits.END_CHARGE_CURRENT_DETECTION = current_val;

            return this->WriteReg8(R::Charger_CTL1, Charger_CTL1.reg_byte);
        }

        /*@brief  set voltage Vout for charging
          @param  voltage value - 111:4.8
                                  110:4.75
                                  101:4.7
                                  100:4.65
                                  011:4.6
                                  010:4.55
                                  001:4.5
                                  000:4.45
          @retval None
          @note   Default value - 101
        */
        ErrorCode charger_under_voltage(R::eCHARGING_VOLTAGE voltage_val)
        {
            Charger_CTL1.bits.CHARGE_UNDER_VOLTAGE_LOOP = voltage_val;

            return this->WriteReg8(R::Charger_CTL1, Charger_CTL1.reg_byte);
        }

        /*@brief  set battery voltage
          @param  voltage value - 00:4.2
                                  01:4.3
                                  02:4.35
                                  03:4.4
          @retval None
          @note   Default value - 00
        */
        ErrorCode set_battery_voltage(R::eBATTERY_VOLTAGE voltage_val)
        {
            Charger_CTL2.bits.BATTERY_VOLTAGE = voltage_val;

            return this->WriteReg8(R::Charger_CTL2, Charger_CTL2.reg_byte);
        }

        /*@brief  set constant voltage charging setting
          @param  voltage value - 11: Pressurized 42mV
                                  10: Pressurized 28mV
                                  01: Pressurized 14mV
                                  00: Not pressurized
          @retval None
          @note   Default value - 01
          @note :4.30V/4.35V/4.4V It is recommended to pressurize 14mV
                 4.2V It is recommended to pressurize 28mV
        */
        ErrorCode set_voltage_pressure(R::eVOLTAGE_PRESSURE voltage_val)
        {
            Charger_CTL2.bits.VOLTAGE_PRESSURE = voltage_val;

            return this->WriteReg8(R::Charger_CTL2, Charger_CTL2.reg_byte);
        }

        /*@brief  set constant current charging setting
          @param  value - 1:VIN end CC Constant current
                          0:BAT end CC Constant current
          @retval None
          @note   Default value - 1
        */
        ErrorCode set_cc_loop(bool current_val)
        {
            Charger_CTL3.bits.CHARGE_CC_LOOP = current_val;

            return this->WriteReg8(R::Charger_CTL3, Charger_CTL3.reg_byte);
        }

        /*@brief  get current charging status
          @param  None
          @retval integer representation of charging status(1 - charging is on and 0 if  charging is off)
        */
        bool is_charging(void)
        {
            this->ReadRegs8(R::READ0, &R0.reg_byte, 1);
            return R0.bits.CHARGE_ENABLE;
        }

        /*@brief  get current battery status
          @param  None
          @retval integer representation of battery status(1 - fully charged and 0 if still charging)
        */
        bool is_battery_fully_charged(void)
        {
            this->ReadRegs8(R::READ1, &R1.reg_byte, 1);
            return R1.bits.BATTERY_STATUS;
        }

        /*@brief  get output load level
          @param  None
          @retval integer representation of load status(1 - light load and 0 if heavy load)
        */
        bool is_light_load(void)
        {
            this->ReadRegs8(R::READ2, &R2.reg_byte, 1);
            return R2.bits.LOAD_LEVEL;
        }

        uint8_t batLevel2Percentage(){
            switch (this->batLevel & 0xF0)
            {
            case 0xE0:
                return 25;
            case 0xC0:
                return 50;
            case 0x80:
                return 75;
            case 0x00:
                return 100;
            default:
                return 0;
            }
        }

        uint8_t getBatteryLevel()
        {

            this->ReadRegs8(R::BAT_LEVEL, &this->batLevel, 1);
            return batLevel2Percentage();
            
        }
    };
}
#undef TAG