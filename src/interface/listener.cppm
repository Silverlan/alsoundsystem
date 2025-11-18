// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.soundsystem:listener;

export import pragma.math;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace al {
	class ISoundSystem;
	class DLLALSYS IListener {
	  public:
		~IListener() = default;

		virtual void SetGain(float gain);
		float GetGain() const;

		virtual void SetPosition(const Vector3 &pos);
		const Vector3 &GetPosition() const;

		virtual void SetVelocity(const Vector3 &vel);
		const Vector3 &GetVelocity() const;

		virtual void SetOrientation(const Vector3 &at, const Vector3 &up);
		const std::pair<Vector3, Vector3> &GetOrientation() const;

		float GetMetersPerUnit() const;
		void SetMetersPerUnit(float mu);
	  protected:
		IListener(al::ISoundSystem &system);
		virtual void DoSetMetersPerUnit(float mu) = 0;

		al::ISoundSystem &m_soundSystem;
		float m_gain = 1.f;
		Vector3 m_position = {};
		Vector3 m_velocity = {};
		std::pair<Vector3, Vector3> m_orientation = {};
		float m_metersPerUnit = 1.f;

		friend ISoundSystem;
	};
};
#pragma warning(pop)
