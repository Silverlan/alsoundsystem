// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:coordinate_system;

export import pragma.util;

export {
	namespace al {
		DLLALSYS void set_world_scale(float scale);
		DLLALSYS Vector3 to_audio_direction(const Vector3 &v);
		DLLALSYS Vector3 to_audio_position(const Vector3 &v);
		DLLALSYS Vector3 to_audio_scale(const Vector3 &v);

		DLLALSYS Vector3 to_game_direction(const Vector3 &v);
		DLLALSYS Vector3 to_game_position(const Vector3 &v);
		DLLALSYS Vector3 to_game_scale(const Vector3 &v);

		template<class T>
		T to_custom_vector(const Vector3 &v);
		template<class T>
		Vector3 to_game_vector(const T &v);

		DLLALSYS float to_audio_distance(float r);
		DLLALSYS float to_game_distance(float r);
	};

	template<class T>
	T al::to_custom_vector(const Vector3 &v)
	{
		return T {v.x, v.y, v.z};
	}
	template<class T>
	Vector3 al::to_game_vector(const T &v)
	{
		return Vector3 {v.x, v.y, v.z};
	}
}
