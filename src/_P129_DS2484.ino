// #######################################################################################################
// #################################### Plugin 129: Dallas DS18B20 DS2484      ###########################
// #######################################################################################################

//	This is a modification to Plugin_004

#include <DS2482_OneWire.h>
#include <DallasTemperature.h>
#include "ESPEasy_Log.h"
#include "ESPEasy-Globals.h"

#define PLUGIN_129
#define PLUGIN_ID_129         129
#define PLUGIN_NAME_129       "Environment - ds2482-100(I2C)ds18b20"
#define PLUGIN_VALUENAME1_129 "Temperature"

OneWire oneWire;
DallasTemperature sensors(&oneWire);


uint8_t 		numberOfDevices;
#define DS2482_I2C_ADDRESS      0x18 // I2C address for the sensor


void getDeviceCount() {
    addLog(LOG_LEVEL_ERROR,F("getDeviceCount"));
    sensors.begin();
    numberOfDevices = sensors.getDeviceCount();
}

void applyDeviceCount() {
    getDeviceCount();
    switch(numberOfDevices){
        case 0:
            Device[deviceCount].VType              = SENSOR_TYPE_NONE;
            break;
        case 1:
            Device[deviceCount].VType              = SENSOR_TYPE_SINGLE;
            break;
        case 2:
            Device[deviceCount].VType              = SENSOR_TYPE_DUAL;
            break;
        case 3:
            Device[deviceCount].VType              = SENSOR_TYPE_TRIPLE;
            break;
        default:
            Device[deviceCount].VType              = SENSOR_TYPE_QUAD;

    }
    Device[deviceCount].ValueCount         = numberOfDevices;
}

boolean Plugin_129(byte function, struct EventStruct * event, String& string)
{
    boolean success = false;

    switch (function)
    {
        case PLUGIN_DEVICE_ADD:
           {
            addLog(LOG_LEVEL_ERROR,F("PLUGIN_DEVICE_ADD"));
            Device[++deviceCount].Number           = PLUGIN_ID_129;
            Device[deviceCount].Type               = DEVICE_TYPE_I2C;
            Device[deviceCount].Ports              = 0;
            Device[deviceCount].PullUpOption       = false;
            Device[deviceCount].InverseLogicOption = false;
            Device[deviceCount].FormulaOption = false;
            Device[deviceCount].DecimalsOnly = true;
            applyDeviceCount();
            Device[deviceCount].SendDataOption     = true;
            Device[deviceCount].TimerOption        = true;
            Device[deviceCount].GlobalSyncOption   = true;
            break;
        }

        case PLUGIN_GET_DEVICENAME:
        {
            addLog(LOG_LEVEL_ERROR,F("PLUGIN_GET_DEVICENAME"));
            string = F(PLUGIN_NAME_129);
            break;
        }

        case PLUGIN_GET_DEVICEVALUENAMES:
        {
            addLog(LOG_LEVEL_ERROR,F("getDeviceValueNames"));
            uint8_t tmpAddress[8];
            byte count = 0;
            for (int i=0; i<numberOfDevices; i++)
            {
                addLog(LOG_LEVEL_ERROR,F("GetAddress"));
                sensors.getAddress(tmpAddress, i);
                String option = "";
                for (byte j = 0; j < 8; j++)
                {
                    option += String(tmpAddress[j], HEX);
                    if (j < 7) option += F("_");
                }
                count ++;
                ExtraTaskSettings.TaskDeviceValueDecimals[i]=2;
                strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[i], option.c_str());
            }

            break;
        }

        case PLUGIN_WEBFORM_LOAD:
        {
            addLog(LOG_LEVEL_ERROR,F("WebFormLoad"));
            uint8_t savedAddress[8];
            byte resolutionChoice = 0;
            addLog(LOG_LEVEL_ERROR,F("Sensors.begin"));

            // Scan the onewire bus and fill dropdown list with devicecount on this GPIO.
            //Plugin_129_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];

            // get currently saved address
            for (byte i = 0; i < 8; i++) {
                savedAddress[i] = ExtraTaskSettings.TaskDevicePluginConfigLong[i];
            }

            // find all suitable devices
            addRowLabel(F("Device Address"));
            addSelector_Head(F("plugin_129_dev"), false);
            addSelector_Item("", -1, false, false, F(""));

			/*
            while (Plugin_129_DS_search(tmpAddress))
            {
                String option = "";
                for (byte j = 0; j < 8; j++)
                {
                    option += String(tmpAddress[j], HEX);
                    if (j < 7) option += F("-");
                }
                bool selected = (memcmp(tmpAddress, savedAddress, 8) == 0) ? true : false;
                addSelector_Item(string, option, count, selected, false, F(""));
                count ++;
            }
			*/
            addSelector_Foot();

            // Device Resolution select
            if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0)
                resolutionChoice = Plugin_129_DS_getResolution(savedAddress);
            else
                resolutionChoice = 11;
            String resultsOptions[4] = { "9", "10", "11", "12" };
            int resultsOptionValues[4] = { 9, 10, 11, 12 };
            addFormSelector(F("Device Resolution"), F("plugin_129_res"), 4, resultsOptions, resultsOptionValues, resolutionChoice);
            addHtml(F(" Bit"));

            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SAVE:
        {
            addLog(LOG_LEVEL_ERROR,F("WebFormSave"));
            uint8_t addr[8] = {0,0,0,0,0,0,0,0};

            // save the address for selected device and store into extra tasksettings
            // Plugin_129_DallasPin = Settings.TaskDevicePin1[event->TaskIndex];
            // byte devCount =
            Plugin_129_DS_scan(getFormItemInt(F("plugin_129_dev")), addr);
            for (byte x = 0; x < 8; x++)
                ExtraTaskSettings.TaskDevicePluginConfigLong[x] = addr[x];

            Plugin_129_DS_setResolution(addr, getFormItemInt(F("plugin_129_res")));

            success = true;
            break;
        }

        case PLUGIN_WEBFORM_SHOW_CONFIG:
        {
            for (byte x = 0; x < 8; x++)
            {
                if (x != 0)
                    string += "-";
                string += String(ExtraTaskSettings.TaskDevicePluginConfigLong[x], HEX);
            }
            success = true;
            break;
        }

        case PLUGIN_READ:
        {
            float value[8];
            if (ExtraTaskSettings.TaskDevicePluginConfigLong[0] != 0){
                addLog(LOG_LEVEL_ERROR,F("PLUGIN_READ"));
                applyDeviceCount();
                // Load ROM address from tasksettings
                LoadTaskSettings(event->TaskIndex);
                uint8_t tmpAddress[8];
                for (int i=0; i<numberOfDevices; i++)
                {
                    addLog(LOG_LEVEL_ERROR,F("GetAddress"));
                    sensors.getAddress(tmpAddress, i);
                    String option = "";
                    addLog(LOG_LEVEL_ERROR,F("Form option-address"));
                    for (byte j = 0; j < 8; j++)
                    {
                        option += String(tmpAddress[j], HEX);
                        if (j < 7) option += F("_");
                    }
                    addLog(LOG_LEVEL_ERROR,F("send TaskDeviceValueNames"));
                    strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[i], option.c_str());
                    addLog(LOG_LEVEL_ERROR,F("set decimals"));
                    ExtraTaskSettings.TaskDeviceValueDecimals[i]=2;
                    option+= F(" : Temperature: ");
                    addLog(LOG_LEVEL_ERROR,F("Read Value"));
                    if (Plugin_129_DS_readTemp(tmpAddress, &value[i]))
                    {
                        option    += value[i];
                    }
                    else
                    {
                        value[i] = NAN;
                        option += F("Error!");
                    }
                    addLog(LOG_LEVEL_INFO, option);
                    UserVar[event->BaseVarIndex+i] = value[i];
                }
                success = true;
            }
            break;
        }
    }
    return success;
}

/*********************************************************************************************\
   Dallas Scan bus
  \*********************************************************************************************/
byte Plugin_129_DS_scan(byte getDeviceROM, uint8_t* ROM)
{
  byte tmpaddr[8];

  sensors.getAddress(tmpaddr, getDeviceROM);
  for (byte  i = 0; i < 8; i++)
        ROM[i] = tmpaddr[i];

  return getDeviceROM;
}

/*********************************************************************************************\
*  Dallas Read temperature
\*********************************************************************************************/
boolean Plugin_129_DS_readTemp(uint8_t ROM[8], float * value)
{
    addLog(LOG_LEVEL_ERROR,F("sensors.requestTemperaturesByAddress(ROM)"));
  sensors.requestTemperaturesByAddress(ROM);
    addLog(LOG_LEVEL_ERROR,F("value = sensors.getTempC(ROM);"));
	*value = sensors.getTempC(ROM);

    return true;
} // Plugin_129_DS_readTemp

/*********************************************************************************************\
* Dallas Get Resolution
\*********************************************************************************************/
int Plugin_129_DS_getResolution(uint8_t ROM[8])
{
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == 0x10)
	{
		return 12;
	}
	else
	{
		return sensors.getResolution(ROM);
	}
}

/*********************************************************************************************\
* Dallas Set Resolution
\*********************************************************************************************/
boolean Plugin_129_DS_setResolution(uint8_t ROM[8], byte res)
{
    // DS1820 and DS18S20 have no resolution configuration register
    if (ROM[0] == 0x10) return true;

	sensors.setResolution(ROM, res, false);

	return true;

}

/*********************************************************************************************\
*  Dallas Search bus
\*********************************************************************************************/
uint8_t Plugin_129_DS_search(uint8_t * newAddr)
{

  uint8_t search_result   = 0;

	search_result = sensors.isConnected(newAddr);

    return search_result;
}



/*********************************************************************************************\
*  Dallas Write byte
\*********************************************************************************************/


/*********************************************************************************************\
*  Dallas Read bit
\*********************************************************************************************/


/*********************************************************************************************\
*  Dallas Write bit
\*********************************************************************************************/


uint8_t Plugin_129_DS_crc8(uint8_t * addr, uint8_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}
