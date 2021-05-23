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
	class ISoundSystem;
	class DLLALSYS IListener
	{
	public:
		~IListener()=default;

		virtual void SetGain(float gain);
		float GetGain() const;

		virtual void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition() const;

		virtual void SetVelocity(const Vector3 &vel);
		const Vector3 &GetVelocity() const;

		virtual void SetOrientation(const Vector3 &at,const Vector3 &up);
		const std::pair<Vector3,Vector3> &GetOrientation() const;

		float GetMetersPerUnit() const;
		void SetMetersPerUnit(float mu);
	protected:
		IListener(al::ISoundSystem &system);
		virtual void DoSetMetersPerUnit(float mu)=0;
		
		al::ISoundSystem &m_soundSystem;
		float m_gain = 1.f;
		Vector3 m_position = {};
		Vector3 m_velocity = {};
		std::pair<Vector3,Vector3> m_orientation = {};
		float m_metersPerUnit = 1.f;

		friend ISoundSystem;
	};
};
#pragma warning(pop)

#endif
