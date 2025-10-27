/*
 * diagnostics.c
 *
 *  Created on: 11 Sep 2019
 *      Author: Phil
 */

#include "usb_diagnostics.h"
#include "go.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include <string.h>

extern const uint8_t VersionString[]; // TODO use sys struct

__attribute__((section(".usb_buffer"))) static USB_BufferTypeDef buffer1;
__attribute__((section(".usb_buffer"))) static USB_BufferTypeDef buffer2;
static USB_BufferTypeDef* active_buffer;
static USB_BufferTypeDef* transmit_buffer;

//JSB: static const uint32_t SYNC_PATTERN = 0x4885BCF7;

extern USBD_HandleTypeDef hUsbDeviceFS;

static volatile enum USB_State state = USB_STATE_DISCONNECTED;

static void usb_disconnect()
{
	state = USB_STATE_DISCONNECTED;
}

void usb_clear_buffer(USB_BufferTypeDef* buffer)
{
	GUID_t_PJB format = {{0xB0,0x85,0xBE,0x8F,0xE7,0x9A,0x18,0x4D,0xB8,0x0B,0x34,0x0F,0x9E,0xEC,0x59,0x5D}}; // {8FBE85B0-9AE7-4D18-B80B-340F9EEC595D}

	buffer->write_handles = 0;
	buffer->sending = false;

	memcpy(buffer->data, (uint8_t *) &format, sizeof(GUID_t_PJB));
	buffer->write_index = sizeof(GUID_t_PJB) + 4;
}

void usb_select_inital_buffer(USB_BufferTypeDef* buffer)
{
	active_buffer = buffer;

	if (buffer == &buffer1)
		transmit_buffer = &buffer2;
	else
		transmit_buffer = &buffer1;

	transmit_buffer->sending = true;
}

void usb_clear_buffers()
{
	usb_clear_buffer(&buffer1);
	usb_clear_buffer(&buffer2);
}

void diagnostics_init()
{
	usb_clear_buffers();
	usb_select_inital_buffer(&buffer1);

	// FIXME remove:
	for (uint32_t i = 0; i < (USB_BUFFER_SIZE - 128) / 5; i++) // JSB: Bodge re upper limit.
	{
		transmit_buffer->data[transmit_buffer->write_index++] = 20;
		transmit_buffer->data[transmit_buffer->write_index++] = 0x01;
		transmit_buffer->data[transmit_buffer->write_index++] = 0x02;
		transmit_buffer->data[transmit_buffer->write_index++] = 0x03;
		transmit_buffer->data[transmit_buffer->write_index++] = 0x04;
	}
	transmit_buffer->sending = false;
	*((uint16_t*) (transmit_buffer->data + sizeof(GUID_t_PJB) + 4)) = transmit_buffer->write_index;
}

uint16_t transmitted_sizes[100];
uint8_t transmitted_sizes_index = 0;
uint32_t sent_buffers = 0;
void usb_swap_buffers()
{
	__disable_irq();
	USB_BufferTypeDef* temp = transmit_buffer;
	transmit_buffer = active_buffer;
	active_buffer = temp;
	active_buffer->write_index = sizeof(GUID_t_PJB) + 4;
	__enable_irq();

	transmit_buffer->sending = false;

	*((uint16_t*) (transmit_buffer->data + sizeof(GUID_t_PJB) + 4)) = transmit_buffer->write_index;
}

bool usb_flush()
{
	/** Check if USB is able to take new data set **/
	if (hUsbDeviceFS.pClassData == NULL)
		return false; // TODO bad

	if (((USBD_CDC_HandleTypeDef *) hUsbDeviceFS.pClassData)->TxState != 0)
		return false; // BUSY

	// FIXME uncomment
//	if (transmit_buffer->sending)
//		usb_swap_buffers();

	if (transmit_buffer->write_handles > 0)
		return false;

	// Added by JSB:
  *((uint32_t*) (transmit_buffer->data + sizeof(GUID_t_PJB))) = sent_buffers;

	if (!transmit_buffer->sending && (CDC_Transmit_FS(transmit_buffer->data, transmit_buffer->write_index) == USBD_OK))
	{
		transmitted_sizes[transmitted_sizes_index++] = transmit_buffer->write_index;
		if (transmitted_sizes_index == 100)
			transmitted_sizes_index = 0;
		sent_buffers++;

		//transmit_buffer->sending = true; TODO uncomment
		return true;
	}
	return false;
}

void usb_release_reserved_buffer_space(USB_BufferTypeDef* pAllocated_buffer)
{
	__atomic_fetch_sub(&pAllocated_buffer->write_handles, 1, __ATOMIC_RELEASE);
}

/**
 * Attempts to allocate a portion of the currently active USB buffer to write to. If successful, returns memory address of allocated memory. Otherwise returns NULL.
 *
 * If successful, usb_release_reserved_buffer_space must be used to free your handle on the buffer once you have finished writing to the buffer.
 */
void* usb_reserve_buffer_space(uint16_t requested_size, USB_BufferTypeDef** ppAllocated_buffer)
{
	USB_BufferTypeDef* pBuffer = active_buffer;
	__atomic_fetch_add(&pBuffer->write_handles, 1, __ATOMIC_ACQUIRE);
	uint16_t buffer_index = __atomic_fetch_add(&pBuffer->write_index, requested_size, __ATOMIC_ACQUIRE);
	if (buffer_index + requested_size >= USB_BUFFER_SIZE)
	{
		__atomic_fetch_sub(&pBuffer->write_index, requested_size, __ATOMIC_RELEASE);
		usb_release_reserved_buffer_space(pBuffer);
		return NULL;
	}
	*ppAllocated_buffer = pBuffer;
	return ((uint8_t *) pBuffer) + buffer_index;
}

IR_ERR log_value(enum TValue type, void *value)
{
	if (state != USB_STATE_CONNECTED)
		return IR_ERR_NOT_READY;

// FIXME uncomment all
//	static uint32_t last_time = 0;
//	if (sys.current_loop > last_time)
//	{
//		last_time = sys.current_loop;
//		log_value(VAR_TIME, &sys.current_loop);
//	}
//
//	struct SLogValue
//	{
//		uint8_t type;
//		uint8_t v[4];
//	};
//
//	USB_BufferTypeDef* allocated_buffer;
//	struct SLogValue* pBuffer = (struct SLogValue *) usb_reserve_buffer_space(sizeof(struct SLogValue), &allocated_buffer);
//	if (pBuffer == NULL)
//	{
//		//TODO do we need to be so strict?
//		usb_disconnect();
//		return IR_ERR_BUFFER_FULL;
//	}

// FIXME uncomment
//	pBuffer->type = type;

// FIXME uncomment all
//	switch (type)
//	{
//		case VAR_VELOCITY_KP:
//		case VAR_VELOCITY_KI:
//		case VAR_VELOCITY_KD:
//		case VAR_VELOCITY_SATURATION:
//		case VAR_CURRENT_KP:
//		case VAR_CURRENT_KI:
//#ifdef _PID_USE_DOUBLE
//		{
//			float f = (float) *((double *) value);
//			value = &f;
//		}
//#endif
//		// Fall through
//		case VAR_CURRENT:
//		case VAR_ENC_MOTOR:
//		case VAR_ENC_OUTPUT:
//		case VAR_JOYSTICK:
//		case VAR_MOTOR_VELOCITY:
//		case VAR_JOYSTICK_RANGE:
//		case VAR_TARGET_TORQUE:
//		case VAR_TARGET_VELOCITY:
//		case VAR_THERMAL_MODEL_OUTPUT:
//			*((float *) pBuffer->v) = *((float *) value);
//			break;
//		case VAR_TIME:
//		case VAR_SYS_STATE:
//		case VAR_CONTROL_METHOD:
//		case VAR_POLE_PAIRS:
//		case VAR_ERROR_CODE:
//			*((uint32_t *) pBuffer->v) = *((uint32_t *) value);
//			break;
//		default:
//			break;
//	}
//	usb_release_reserved_buffer_space(allocated_buffer);
	usb_flush();
	return IR_ERR_NONE;
}

IR_ERR log_error_alt(const char* file, uint32_t line, const char* message)
{
	if (state != USB_STATE_CONNECTED)
		return IR_ERR_NOT_READY;

	uint32_t file_length = strlen(file);
	uint32_t message_length = strlen(message);
	uint32_t total_length = file_length + message_length + 13;

	USB_BufferTypeDef* allocated_buffer;
	uint8_t* pBuffer = usb_reserve_buffer_space(total_length, &allocated_buffer);
	if (pBuffer == NULL)
		return IR_ERR_BUFFER_FULL;

	*pBuffer = VAR_ERROR_CODE;
	pBuffer += 1;
	*((uint32_t *) pBuffer) = *((uint32_t *) &file_length);
	pBuffer += 4;
	strcpy((char*) pBuffer, file);
	pBuffer += file_length;
	*((uint32_t *) pBuffer) = *((uint32_t *) &line);
	pBuffer += 4;
	*((uint32_t *) pBuffer) = *((uint32_t *) &message_length);
	pBuffer += 4;
	strcpy((char*) pBuffer, message);

	usb_release_reserved_buffer_space(allocated_buffer);

	return IR_ERR_NONE;
}

static inline float read_float(uint8_t* data, uint16_t* read_address)
{
	Float u;
	memcpy(u.b, data + *read_address, 4);
	*read_address += 4;
	return u.f;
}

static inline uint32_t read_int32(uint8_t* data, uint16_t* read_address)
{
	Int32 u;
	memcpy(u.b, data + *read_address, 4);
	*read_address += 4;
	return u.i;
}

uint32_t offset_add(uint32_t currentOffsetRaw, int32_t add)
{
	currentOffsetRaw &= 0x7F;

	int32_t offset;
	if (currentOffsetRaw >= 0x40)
		offset = 0x40 - currentOffsetRaw;
	else
		offset = currentOffsetRaw;

	int32_t result = offset + add;
	uint32_t resultingOffset;
	if (result >= 0)
	{
		if (result > 63)
			result = 63;
		resultingOffset = result;
	}
	else
	{
		result = -result;
		if (result > 63)
			result = 63;
		resultingOffset = result + 0x40;
	}

	return resultingOffset;
}

uint32_t phase_add(uint32_t currentPhase, int32_t add)
{
	return offset_add(currentPhase, add);
}

uint32_t gain_add(uint32_t currentGainRaw, int32_t add)
{
	currentGainRaw &= 0x7F;

	int32_t gain;
	if (currentGainRaw >= 0x40)
	{
		gain = currentGainRaw;
		gain -= 0x80;
	}
	else
		gain = currentGainRaw;

	int32_t result = gain + add;
	uint32_t resultingGain;
	if (result >= 0)
	{
		if (result > 63)
			result = 63;
		resultingGain = result;
	}
	else
	{
		result = -result;
		if (result > 63)
			result = 63;
		resultingGain = 0x80 - result;
	}

	return resultingGain;
}

//void usb_handle_data(uint8_t* data, uint32_t* length)
//{
//	uint16_t read_address = 0;
//	if (*length < 4)
//		return;
//	if (*((uint32_t *) data) != SYNC_PATTERN)
//		return;
//	read_address += 4;
//
//	// TODO maybe improve this
//	if (data[read_address] == TOK_HANDSHAKE_SYN)
//	{
////		state = USB_STATE_DISCONNECTED; // FIXME uncomment
//
//		state = USB_STATE_CONNECTED; // FIXME remove
//	}
//	return; // FIXME remove
//
//	switch (state)
//	{
//		case USB_STATE_DISCONNECTED:
//			if (*length != 9)
//				break;
//			if (data[read_address++] != TOK_HANDSHAKE_SYN)
//				break;
//			int32_t sync_token = *((int32_t *) (data + read_address));
//			read_address += 4;
//			sync_token++;
//
//			usb_clear_buffer(active_buffer);
////			usb_swap_buffers();
//
//			uint8_t* pBuffer;
//			USB_BufferTypeDef* allocated_buffer;
//
//			GUID_t_PJB handshake_format = {{0xCC, 0x16, 0x05, 0x23, 0x8F, 0x51, 0xF2, 0x40, 0xB0, 0xC5, 0xB7, 0x9F, 0x29, 0xE3, 0xC0, 0xD6}}; // {230516CC-518F-40F2-B0C5-B79F29E3C0D6}
//			pBuffer = usb_reserve_buffer_space(sizeof(GUID_t_PJB), &allocated_buffer);
//			memcpy(pBuffer, &handshake_format, sizeof(GUID_t_PJB));
//			usb_release_reserved_buffer_space(allocated_buffer);
//
//			pBuffer = usb_reserve_buffer_space(17, &allocated_buffer);
//			*pBuffer = TOK_HANDSHAKE_SYNACK;
//			pBuffer++;
//			*((int32_t *) pBuffer) = sync_token;
//			pBuffer += 4;
//			memcpy(pBuffer, (uint32_t *) UID_BASE, 12);
//			usb_release_reserved_buffer_space(allocated_buffer);
//
//			uint8_t version_string_length = strlen((const char*) VersionString);
//			pBuffer = usb_reserve_buffer_space(version_string_length + 1, &allocated_buffer);
//			*pBuffer = version_string_length;
//			pBuffer++;
//			memcpy(pBuffer, VersionString, version_string_length);
//			usb_release_reserved_buffer_space(allocated_buffer);
//
//			pBuffer = usb_reserve_buffer_space(19, &allocated_buffer);
//			memcpy(pBuffer, sys.status.build_date, 11);
//			pBuffer += 11;
//			memcpy(pBuffer, sys.status.build_time, 8);
//			usb_release_reserved_buffer_space(allocated_buffer);
//
////			usb_swap_buffers();
//			usb_flush();
//
//			state = USB_STATE_HANDSHAKE;
//			break;
//		case USB_STATE_HANDSHAKE:
//			if (*length != 5)
//			{
//				state = USB_STATE_DISCONNECTED;
//				break;
//			}
//			if (data[read_address++] != TOK_HANDSHAKE_ACK)
//			{
//				state = USB_STATE_DISCONNECTED;
//				break;
//			}
//			state = USB_STATE_CONNECTED;
//
////			log_error_alt(__FILE__, __LINE__, "Handshake done 1.");
////			log_error_alt(__FILE__, __LINE__, "Handshake done 2.");
////			log_error_alt(__FILE__, __LINE__, "Handshake done 3.");
////			log_error_alt(__FILE__, __LINE__, "Handshake done 4.");
////			*((uint16_t*) (active_buffer->data + 4)) = active_buffer->write_index;
////			CDC_Transmit_FS(active_buffer->data, active_buffer->write_index);
//			//usb_swap_buffers();
//			//usb_flush();
//
//			break;
//		case USB_STATE_CONNECTED:
//			break;
//	}
//
//	if (state != USB_STATE_CONNECTED)
//		return;
//
//	while (*length > read_address)
//	{
//		enum TValue var_type = data[read_address++];
//		float f;
//		switch (var_type)
//		{
//			case FUN_CALIBRATE_MOTOR:
//				sys.calibration_requested = 1;
//				break;
//			case FUN_ENABLE_DRIVE:
//				sys_request_state(SYS_STATE_ACTIVE);
//				break;
//			case FUN_DISABLE_DRIVE:
//				sys_request_state(SYS_STATE_READY);
//				break;
//			case VAR_VELOCITY_KP:
//				f = read_float(data, &read_address);
//				sys.pid_velocity.kp = f;
//				break;
//			case VAR_VELOCITY_KI:
//				f = read_float(data, &read_address);
//				sys.pid_velocity.ki = f;
//				break;
//			case VAR_VELOCITY_KD:
//				f = read_float(data, &read_address);
//				sys.pid_velocity.kd = f;
//				break;
//			case VAR_VELOCITY_SATURATION:
//				f = read_float(data, &read_address);
//				sys.pid_velocity.output_saturation = f;
//				break;
//			case VAR_CONTROL_METHOD:
//				pSettings->control_method = data[read_address++];
//				if (pSettings->control_method == CM_UI_TORQUE_CONTROL)
//				{
//					SPositionDemandBufferElement e;
//					e.Feedforward = 0.0;
//					e.OutputGain = 0.0;
//					e.Position = 0.0;
//					PositionDemandBuffer_Write(&sys.PositionDemandBuffer, &e);
//				}
//				break;
//			case VAR_JOYSTICK_RANGE:
//				f = read_float(data, &read_address);
//				pSettings->joystick_range = f;
//				break;
//			case VAR_TARGET_VELOCITY:
//				f = read_float(data, &read_address);
//				if (pSettings->control_method == CM_UI_VELOCITY_CONTROL || pSettings->control_method == CM_UI_OPEN_LOOP_VELOCITY)
//					sys.pid_velocity.setpoint = f;
//				break;
//			case VAR_TARGET_TORQUE:
//			{
//				f = read_float(data, &read_address);
//				SPositionDemandBufferElement e;
//				if (pSettings->control_method == CM_UI_TORQUE_CONTROL || pSettings->control_method == CM_UI_OPEN_LOOP_VELOCITY)
//				{
//					e.Feedforward = f;
//					e.OutputGain = 0.0;
//					e.Position = 0.0;
//					PositionDemandBuffer_Write(&sys.PositionDemandBuffer, &e);
//				}
//				else if (pSettings->control_method == CM_SERVO)
//				{
//					e.Feedforward = 0.0;
//					e.OutputGain = 1.0;
//					e.Position = f;
//					PositionDemandBuffer_Write(&sys.PositionDemandBuffer, &e);
//				}
//				break;
//			}
//			case VAR_POLE_PAIRS:
//				pSettings->pole_pairs = read_int32(data, &read_address);
//				break;
//			case FUN_SAVE_EEPROM:
//				save_settings();
//				break;
//			case FUN_READ_PARAMS:
//				log_value(VAR_CONTROL_METHOD, &pSettings->control_method);
//				log_value(VAR_JOYSTICK_RANGE, &pSettings->joystick_range);
//				log_value(VAR_TARGET_TORQUE, &sys.pid_position.feedforward); // Make this a separate parameter
//				log_value(VAR_TARGET_VELOCITY, &sys.pid_velocity.setpoint); // Make this a separate parameter
//				log_value(VAR_VELOCITY_KP, &sys.pid_velocity.kp);
//				log_value(VAR_VELOCITY_KI, &sys.pid_velocity.ki);
//				log_value(VAR_VELOCITY_KD, &sys.pid_velocity.kd);
//				log_value(VAR_VELOCITY_SATURATION, &sys.pid_velocity.output_saturation);
//				log_value(VAR_POLE_PAIRS, &pSettings->pole_pairs);
//				log_value(VAR_CURRENT_KP, &sys.pid_current.kp);
//				log_value(VAR_CURRENT_KI, &sys.pid_current.ki);
//				break;
//			case VAR_CURRENT_KP:
//				f = read_float(data, &read_address);
//				sys.pid_current.kp = f;
//				break;
//			case VAR_CURRENT_KI:
//				f = read_float(data, &read_address);
//				sys.pid_current.ki = f;
//				break;
//			case FUN_FETCH_ERRORS:
//			{
//				// TODO
//				sys.motor_param_test_requested = true;
//
//				break;
//			}
//			case TOK_HANDSHAKE_SYN:
//				state = USB_STATE_CONNECTED;
//				break;
//			case FUN_FLUSH:
//				// Do nothing
//				break;
//			case FUN_ZERO_JOYSTICK:
//				sys.joystick_zero_requested = true;
//				break;
//			case FUN_START_ICMU_CALIBRATION:
//			{
//				uint8_t encoder = data[read_address++];
//				if (encoder == 0)
//					biss_enc_m.calibration_start_requested = true;
//				else
//					biss_enc_o.calibration_start_requested = true;
//				break;
//			}
//			case FUN_STOP_ICMU_CALIBRATION:
//			{
//				uint8_t encoder = data[read_address++];
//				if (encoder == 0)
//					biss_enc_m.calibration_stop_requested = true;
//				else
//					biss_enc_o.calibration_stop_requested = true;
//				break;
//			}
//			case FUN_UPDATE_ICMU_ANALOG:
//			{
//				uint8_t encoder = data[read_address++];
//
//				ICMU_AnalogTunings* pAnalog_tunings;
//				if (encoder == 0)
//					pAnalog_tunings = &sys.encoder_motor.analog_tunings;
//				else
//					pAnalog_tunings = &sys.encoder_output.analog_tunings;
//
//				ICMU_AnalogTunings analog_adjustment;
//				analog_adjustment.masterGain = data[read_address++];
//				analog_adjustment.masterSinOffset = data[read_address++];
//				analog_adjustment.masterCosOffset = data[read_address++];
//				analog_adjustment.masterPhaseOffset = data[read_address++];
//				analog_adjustment.noniusGain = data[read_address++];
//				analog_adjustment.noniusSinOffset = data[read_address++];
//				analog_adjustment.noniusCosOffset = data[read_address++];
//				analog_adjustment.noniusPhaseOffset = data[read_address++];
//
//				pAnalog_tunings->masterGain = gain_add(pAnalog_tunings->masterGain, analog_adjustment.masterGain);
//				pAnalog_tunings->masterSinOffset = offset_add(pAnalog_tunings->masterSinOffset, analog_adjustment.masterSinOffset);
//				pAnalog_tunings->masterCosOffset = offset_add(pAnalog_tunings->masterCosOffset, analog_adjustment.masterCosOffset);
//				pAnalog_tunings->masterPhaseOffset = phase_add(pAnalog_tunings->masterPhaseOffset, analog_adjustment.masterPhaseOffset);
//				pAnalog_tunings->noniusGain = gain_add(pAnalog_tunings->noniusGain, analog_adjustment.noniusGain);
//				pAnalog_tunings->noniusSinOffset = offset_add(pAnalog_tunings->noniusSinOffset, analog_adjustment.noniusSinOffset);
//				pAnalog_tunings->noniusCosOffset = offset_add(pAnalog_tunings->noniusCosOffset, analog_adjustment.noniusCosOffset);
//				pAnalog_tunings->noniusPhaseOffset = phase_add(pAnalog_tunings->noniusPhaseOffset, analog_adjustment.noniusPhaseOffset);
//
//				pAnalog_tunings->changed = true;
//				break;
//			}
//			case FUN_UPDATE_ICMU_NONIUS:
//			{
//				uint8_t encoder = data[read_address++];
//
//				ICMU_NoniusTunings* pNonius_tunings;
//				if (encoder == 0)
//					pNonius_tunings = &sys.encoder_motor.nonius_tunings;
//				else
//					pNonius_tunings = &sys.encoder_output.nonius_tunings;
//
//				pNonius_tunings->base = data[read_address++];
//				for (int i = 0; i < 15; i++)
//					pNonius_tunings->SPOs[i] = data[read_address++];
//
//				pNonius_tunings->changed = true;
//				break;
//			}
//			case VAR_POSITION_VELOCITY_FEEDBACK_GAIN:
//				// TODO sys.pid_position.velocity_feedback_gain = read_float(data, &read_address);
//				break;
//			case VAR_POSITION_VELOCITY_FEEDFORWARD_GAIN:
//				// TODO sys.pid_position.velocity_feedforward_gain = read_float(data, &read_address);
//				break;
//			case VAR_VELOCITY_FILTER_DECAY:
//				sys.velocity_filter_decay_factor = read_float(data, &read_address);
//				break;
//			default:
//				break;
//		}
//	}
//}
