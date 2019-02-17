/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_LISTENER_HPP__
#define __ALSOUND_LISTENER_HPP__

#include "alsound_definitions.hpp"
#include <mathutil/uvec.h>

#pragma warning(push)
#pragma warning(disable:4251)
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure {class Listener;};
#endif
namespace al
{
	class SoundSystem;
	class DLLALSYS Listener
	{
	public:
		~Listener()=default;

		void SetGain(float gain);
		float GetGain() const;

		void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition() const;

		void SetVelocity(const Vector3 &vel);
		const Vector3 &GetVelocity() const;

		void SetOrientation(const Vector3 &at,const Vector3 &up);
		const std::pair<Vector3,Vector3> &GetOrientation() const;

		float GetMetersPerUnit() const;
		void SetMetersPerUnit(float mu);
	private:
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
		Listener(alure::Listener &listener);
		alure::Listener &m_listener;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
		Listener(al::SoundSystem &system);
		al::SoundSystem &m_soundSystem;
#endif

		float m_gain = 1.f;
		Vector3 m_position = {};
		Vector3 m_velocity = {};
		std::pair<Vector3,Vector3> m_orientation = {};
		float m_metersPerUnit = 1.f;

		friend SoundSystem;
	};
};
#pragma warning(pop)

#endif
