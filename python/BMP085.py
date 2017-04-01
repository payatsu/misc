#!/usr/bin/python3
# -*- coding: utf-8 -*-

import smbus
import time

dev_address = 0x77
control_register_address = 0xF4
control_register_value_temperature = 0x2E
control_register_value_pressures = [0x34, 0x74, 0xB4, 0xF4]
pressure_measure_mode = 0

calibration_data_register_address    = 0xAA
number_of_calibration_data_registers = 22
measured_data_register_address = 0xF6

def initialize():
	calibration_data = smbus.SMBus(1).read_i2c_block_data(dev_address, calibration_data_register_address, number_of_calibration_data_registers)
	global AC1, AC2, AC3, AC4, AC5, AC6, B1, B2, MB, MC, MD
	AC1 = calibration_data[ 0] << 8 | calibration_data[ 1]
	AC2 = calibration_data[ 2] << 8 | calibration_data[ 3]
	AC3 = calibration_data[ 4] << 8 | calibration_data[ 5]
	AC4 = calibration_data[ 6] << 8 | calibration_data[ 7]
	AC5 = calibration_data[ 8] << 8 | calibration_data[ 9]
	AC6 = calibration_data[10] << 8 | calibration_data[11]
	B1  = calibration_data[12] << 8 | calibration_data[13]
	B2  = calibration_data[14] << 8 | calibration_data[15]
	MB  = calibration_data[16] << 8 | calibration_data[17]
	MC  = calibration_data[18] << 8 | calibration_data[19]
	MD  = calibration_data[20] << 8 | calibration_data[21]

def measure_temperature():
	smbus.SMBus(1).write_byte_data(dev_address, control_register_address, control_register_value_temperature)
	time.sleep(0.0045)
	measured_data = smbus.SMBus(1).read_i2c_block_data(dev_address, measured_data_register_address, 2)
	temperature = measured_data[0] << 8 | measured_data[1]
	x1 = (temperature - AC6) * AC5 >> 15
	x2 = (MC << 11) // (x1 + MD)
	B5 = x1 + x2
	t = (B5 + 8) >> 4
	print(t)

def measure_pressure():
	smbus.SMBus(1).write_byte_data(dev_address, control_register_address, control_register_value_pressures[pressure_measure_mode])
	time.sleep(0.0045)
	measured_data = smbus.SMBus(1).read_i2c_block_data(dev_address, measured_data_register_address, 3)
	pressure = (measured_data[0] << 16 | measured_data[1] << 8 | measured_data[2]) >> (8 - pressure_measure_mode)

initialize()
measure_temperature()
measure_pressure()
