// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.soundsystem;

import :listener;

pragma::audio::IListener::IListener(ISoundSystem &system) : m_soundSystem(system) {}

void pragma::audio::IListener::SetGain(float gain) { m_gain = gain; }
float pragma::audio::IListener::GetGain() const { return m_gain; }

void pragma::audio::IListener::SetPosition(const Vector3 &pos) { m_position = pos; }
const Vector3 &pragma::audio::IListener::GetPosition() const { return m_position; }

void pragma::audio::IListener::SetVelocity(const Vector3 &vel) { m_velocity = vel; }
const Vector3 &pragma::audio::IListener::GetVelocity() const { return m_velocity; }

void pragma::audio::IListener::SetOrientation(const Vector3 &at, const Vector3 &up) { m_orientation = {at, up}; }
const std::pair<Vector3, Vector3> &pragma::audio::IListener::GetOrientation() const { return m_orientation; }

float pragma::audio::IListener::GetMetersPerUnit() const { return m_metersPerUnit; }
void pragma::audio::IListener::SetMetersPerUnit(float mu) { m_metersPerUnit = mu; }
