/****************************************************************************
 *
 *   Copyright (c) 2020 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#pragma once

#include <lib/conversion/rotation.h>
#include <lib/matrix/matrix/math.hpp>
#include <px4_platform_common/px4_config.h>
#include <px4_platform_common/log.h>
#include <uORB/Subscription.hpp>
#include <uORB/topics/actuator_controls.h>
#include <uORB/topics/battery_status.h>

namespace calibration
{
class Magnetometer
{
public:
	static constexpr int MAX_SENSOR_COUNT = 4;

	static constexpr uint8_t DEFAULT_PRIORITY = 50;
	static constexpr uint8_t DEFAULT_EXTERNAL_PRIORITY = 75;

	static constexpr const char *SensorString() { return "MAG"; }

	Magnetometer();
	explicit Magnetometer(uint32_t device_id);

	~Magnetometer() = default;

	void PrintStatus();

	const matrix::Vector3f &offset() const { return _offset; }
	matrix::Vector3f scale() const { return _scale.diag(); }

	void set_calibration_index(uint8_t calibration_index) { _calibration_index = calibration_index; }
	void set_device_id(uint32_t device_id);
	void set_external(bool external = true) { _external = external; }
	void set_offset(const matrix::Vector3f &offset) { _offset = offset; }

	void set_scale(const matrix::Vector3f &scale)
	{
		_scale(0, 0) = scale(0);
		_scale(1, 2) = scale(1);
		_scale(2, 2) = scale(2);
	}

	void set_offdiagonal(const matrix::Vector3f &offdiagonal)
	{
		_scale(0, 1) = offdiagonal(0);
		_scale(1, 0) = offdiagonal(0);

		_scale(0, 2) = offdiagonal(1);
		_scale(2, 0) = offdiagonal(1);

		_scale(1, 2) = offdiagonal(2);
		_scale(2, 1) = offdiagonal(2);

	}

	uint32_t device_id() const { return _device_id; }
	int32_t priority() const { return _priority; }
	bool enabled() const { return (_priority > 0); }
	bool external() const { return _external; }

	// apply offsets and scale
	// rotate corrected measurements from sensor to body frame
	inline matrix::Vector3f Correct(const matrix::Vector3f &data)
	{
		return _rotation * (_scale * ((data + _power * _power_compensation) - _offset));
	}

	bool ParametersSave();
	void ParametersUpdate();

	void UpdatePower(float power) { _power = power; }

	void Reset();

	const Rotation &rotation() { return _rotation_enum; }
	void set_rotation(Rotation rotation)
	{
		_rotation_enum = rotation;
		_rotation = get_rot_matrix(rotation);
	}

private:
	Rotation _rotation_enum;

	matrix::Dcmf _rotation;
	matrix::Vector3f _offset;
	matrix::Matrix3f _scale;
	matrix::Vector3f _power_compensation;
	float _power{0.f};

	int8_t _calibration_index{-1};
	uint32_t _device_id{0};
	int32_t _priority{DEFAULT_PRIORITY};

	bool _external{false};
};
} // namespace calibration