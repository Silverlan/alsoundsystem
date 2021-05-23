/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_listener.hpp"
#include "alsoundsystem.hpp"
#include "alsound_settings.hpp"
#include "alsound_coordinate_system.hpp"

al::IListener::IListener(al::ISoundSystem &system)
	: m_soundSystem(system)
{}

void al::IListener::SetGain(float gain)
{
	m_gain = gain;
}
float al::IListener::GetGain() const {return m_gain;}

void al::IListener::SetPosition(const Vector3 &pos)
{
	m_position = pos;
}
const Vector3 &al::IListener::GetPosition() const {return m_position;}

void al::IListener::SetVelocity(const Vector3 &vel)
{
	m_velocity = vel;
}
const Vector3 &al::IListener::GetVelocity() const {return m_velocity;}

void al::IListener::SetOrientation(const Vector3 &at,const Vector3 &up)
{
	m_orientation = {at,up};
}
const std::pair<Vector3,Vector3> &al::IListener::GetOrientation() const {return m_orientation;}

float al::IListener::GetMetersPerUnit() const {return m_metersPerUnit;}
void al::IListener::SetMetersPerUnit(float mu)
{
	m_metersPerUnit = mu;
}
