#ifndef _BQ3060_H_
#define _BQ3060_H_

#include "utils_hal.h"

/* BQ3060_ManufacturerAccess commands
BQ3060_DeviceType = 0x01,
BQ3060_FirmwareVersion = 0x02,
BQ3060_HardwareVersion = 0x03,
BQ3060_DFChecksum = 0x04,
BQ3060_PendingEDVThrecholdVoltage = 0x05,
BQ3060_ManufacturerStatus = 0x06,
BQ3060_ChemistryID = 0x08,
BQ3060_Shutdown = 0x10,
BQ3060_Sleep = 0x11,
BQ3060_SealDevice = 0x20,
BQ3060_LTFPEnable = 0x21,
BQ3060_FUSEActivation = 0x30,
BQ3060_FUSEClear = 0x31,
BQ3060_CalibrationMode = 0x40,
BQ3060_Reset = 0x41,
BQ3060_BootRom = 0xf00,
*/
typedef enum {
	/* SBS Commands */
	BQ3060_ManufacturerAccess = 0x00,
	BQ3060_RemainingCapacityAlarm = 0x01,
	BQ3060_RemainingTimeAlarm = 0x02,
	BQ3060_BatteryMode = 0x03,
	BQ3060_AtRate = 0x04,
	BQ3060_AtRateTimeToFull = 0x05,
	BQ3060_AtRateTimeToEmpty = 0x06,
	BQ3060_AtRateOK = 0x07,
	BQ3060_Temperature = 0x08,
	BQ3060_Voltage = 0x09,
	BQ3060_Current = 0x0a,
	BQ3060_AverageCurrent = 0x0b,
	BQ3060_MaxError = 0x0c,
	BQ3060_RelativeStateOfCharge = 0x0d,
	BQ3060_AbsoluteStateOfCharge = 0x0e,
	BQ3060_RemainingCapacity = 0x0f,
	BQ3060_FullChargeCapacity = 0x10,
	BQ3060_RunTimeToEmpty = 0x11,
	BQ3060_AverageTimeToEmpty = 0x12,
	BQ3060_AverageTimeToFull = 0x13,
	BQ3060_ChargingCurrent = 0x14,
	BQ3060_ChargingVoltage = 0x15,
	BQ3060_BatteryStatus = 0x16,
	BQ3060_CycleCount = 0x17,
	BQ3060_DesignCapacity = 0x18,
	BQ3060_DesignVoltage = 0x19,
	BQ3060_Specificationinfo = 0x1a,
	BQ3060_ManufactureDate = 0x1b,
	BQ3060_SerialNumber = 0x1c,
	BQ3060_ManufacturerName = 0x20,
	BQ3060_DeviceName = 0x21,
	BQ3060_DeviceChemistry = 0x22,
	BQ3060_ManufacturerData = 0x23,
	BQ3060_Authenticate = 0x2f,
	BQ3060_CellVoltage4 = 0x3c,
	BQ3060_CellVoltage3 = 0x3d,
	BQ3060_CellVoltage2 = 0x3e,
	BQ3060_CellVoltage1 = 0x3f,
	/*Extended SBS commands */
	BQ3060_AFEData = 0x45,
	BQ3060_FETControl = 0x46,
	BQ3060_PendingEDV = 0x47,
	BQ3060_StateOfHealth = 0x4f,
	BQ3060_SafetyAlert = 0x50,
	BQ3060_SafetyStatus = 0x51,
	BQ3060_PFAlert = 0x52,
	BQ3060_PFStatus = 0x53,
	BQ3060_OpertationStatus = 0x54,
	BQ3060_ChargingStatus = 0x55,
	BQ3060_FETStatus = 0x56,
	BQ3060_ResetData = 0x57,
	BQ3060_WDResetData = 0x58,
	BQ3060_PackVoltage = 0x5a,
	BQ3060_AverageVoltage = 0x5d,
	BQ3060_TS1Temperature = 0x5e,
	BQ3060_TS2Temperature = 0x5f,
	BQ3060_UnSealKey = 0x60,
	BQ3060_FullAccessKey = 0x61,
	BQ3060_PFKey = 0x62,
	BQ3060_AuthenKey3 = 0x63,
	BQ3060_AuthenKey2 = 0x64,
	BQ3060_AuthenKey1 = 0x65,
	BQ3060_AuthenKey0 = 0x66,
	BQ3060_ManufacturerInfo = 0x70,
	BQ3060_SenseResistor = 0x71,
	BQ3060_TempRange = 0x72,
	BQ3060_DataFlashSubClassID = 0x77,
	BQ3060_DataFlashSubClassPage1 = 0x78,
	BQ3060_DataFlashSubClassPage2 = 0x79,
	BQ3060_DataFlashSubClassPage3 = 0x7a,
	BQ3060_DataFlashSubClassPage4 = 0x7b,
	BQ3060_DataFlashSubClassPage5 = 0x7c,
	BQ3060_DataFlashSubClassPage6 = 0x7d,
	BQ3060_DataFlashSubClassPage7 = 0x7e,
	BQ3060_DataFlashSubClassPage8 = 0x7f,

} BQ3060_reg;

struct BQ3060Data{
	uint16_t Temperature;
	int16_t TS1Temperature;
	int16_t TS2Temperature;
	uint16_t TempRange;
	uint16_t Voltage;
	int16_t Current;
	int16_t AverageCurrent;
	uint16_t CellVoltage1;
	uint16_t CellVoltage2;
	uint16_t CellVoltage3;
	uint16_t CellVoltage4;
	uint16_t PackVoltage;
	uint16_t AverageVoltage;
};

struct BQ3060Config{
	I2CDriver *I2CD;
	I2CPins   *I2CP;
};

void BQ3060Start(struct BQ3060Config * conf);
int BQ3060Set(uint8_t register_id, uint16_t data);
int BQ3060Get(uint8_t register_id, uint16_t* data);

extern EventSource BQ3060_data_ready;
extern EventSource BQ3060_battery_fault;
extern uint16_t crntAlarms[3];
void BQ3060_get_data(struct BQ3060Data * data);

#endif /* _BQ3060_H_ */
