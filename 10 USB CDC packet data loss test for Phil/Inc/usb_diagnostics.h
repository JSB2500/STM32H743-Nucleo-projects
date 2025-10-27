/*
 * diagnostics.h
 *
 *  Created on: 11 Sep 2019
 *      Author: Phil
 */

#ifndef USB_DIAGNOSTICS_H_
#define USB_DIAGNOSTICS_H_

#include "stdint.h" // Added by JSB.
#include "go.h"

typedef uint8_t bool; // Added by JSB.
#define false (0)
#define true (1)

// Added by JSB:
typedef enum
{
  IR_ERR_NONE = 0,
  IR_ERR_NOT_READY = 1,
  IR_ERR_BUFFER_FULL = 2
} IR_ERR;

//JSB!!! #define USB_BUFFER_SIZE (32768)
#define USB_BUFFER_SIZE (1024)

typedef struct
{
	const uint8_t bytes[16];
} GUID_t_PJB;

enum TValue
{
	VAR_ENC_MOTOR = 1,
	VAR_ENC_OUTPUT = 2,
	VAR_CURRENT = 3,
	VAR_JOYSTICK = 4,
	FUN_CALIBRATE_MOTOR = 5,
	FUN_ENABLE_DRIVE = 6,
	VAR_MOTOR_VELOCITY = 7,
	VAR_VELOCITY_KP = 8,
	VAR_VELOCITY_KI = 9,
	VAR_VELOCITY_KD = 10,
	VAR_VELOCITY_SATURATION = 11,
	VAR_SYS_STATE = 12,
	FUN_DISABLE_DRIVE = 13,
	VAR_CONTROL_METHOD = 14,
	VAR_JOYSTICK_RANGE = 15,
	VAR_TARGET_VELOCITY = 16,
	VAR_TARGET_TORQUE = 17,
	FUN_SAVE_EEPROM = 18,
	FUN_READ_PARAMS = 19,
	VAR_TIME = 20,
	VAR_POLE_PAIRS = 21,
	VAR_CURRENT_KP = 22,
	VAR_CURRENT_KI = 23,
	VAR_ERROR_CODE = 24,
	FUN_FETCH_ERRORS = 25,
	VAR_THERMAL_MODEL_OUTPUT = 26,
	TOK_HANDSHAKE_SYN = 27,
	TOK_HANDSHAKE_ACK = 28,
	TOK_HANDSHAKE_SYNACK = 29,
	FUN_FLUSH = 30,
	FUN_ZERO_JOYSTICK = 31,
	FUN_START_ICMU_CALIBRATION = 32,
	FUN_UPDATE_ICMU_ANALOG = 33,
	FUN_UPDATE_ICMU_NONIUS = 34,
	FUN_STOP_ICMU_CALIBRATION = 35,
	VAR_POSITION_VELOCITY_FEEDBACK_GAIN = 36,
	VAR_POSITION_VELOCITY_FEEDFORWARD_GAIN = 37,
	VAR_VELOCITY_FILTER_DECAY = 38
};

enum USB_State
{
	USB_STATE_DISCONNECTED,
	USB_STATE_HANDSHAKE,
	USB_STATE_CONNECTED
};

typedef struct
{
	uint8_t data[USB_BUFFER_SIZE];
	uint16_t write_index;
	uint16_t write_handles;
	bool sending;
} USB_BufferTypeDef;

typedef union
{
	float f;
	uint8_t b[4];
} Float;

typedef union
{
	uint32_t i;
	uint8_t b[4];
} Int32;

void diagnostics_init();
void usb_swap_buffers();
void usb_handle_data(uint8_t* Buf, uint32_t *Len);
bool usb_flush(); // Added by JSB.

#endif /* USB_DIAGNOSTICS_H_ */
