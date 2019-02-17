/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_listener.hpp"
#include "alsoundsystem.hpp"
#include "alsound_settings.hpp"
#include "alsound_coordinate_system.hpp"
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
#include <fmod_studio.hpp>
#endif

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
al::Listener::Listener(alure::Listener &listener)
	: m_listener(listener)
{}
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
al::Listener::Listener(al::SoundSystem &system)
	: m_soundSystem(system)
{}
#endif

void al::Listener::SetGain(float gain)
{
	m_gain = gain;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_listener.setGain(gain);
#endif
}
float al::Listener::GetGain() const {return m_gain;}

void al::Listener::SetPosition(const Vector3 &pos)
{
	m_position = pos;
	auto posAudio = al::to_audio_position(pos);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_listener.setPosition(&posAudio[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	auto &fmodSys = m_soundSystem.GetFMODSystem();
	FMOD_3D_ATTRIBUTES attributes;
	al::fmod::check_result(fmodSys.getListenerAttributes(0,&attributes));
	attributes.position = {posAudio.x,posAudio.y,posAudio.z};
	al::fmod::check_result(fmodSys.setListenerAttributes(0,&attributes));
#endif
}
const Vector3 &al::Listener::GetPosition() const {return m_position;}

void al::Listener::SetVelocity(const Vector3 &vel)
{
	m_velocity = vel;
	auto velAudio = al::to_audio_position(vel);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_listener.setVelocity(&velAudio[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	auto &fmodSys = m_soundSystem.GetFMODSystem();
	FMOD_3D_ATTRIBUTES attributes;
	al::fmod::check_result(fmodSys.getListenerAttributes(0,&attributes));
	attributes.velocity = {velAudio.x,velAudio.y,velAudio.z};
	al::fmod::check_result(fmodSys.setListenerAttributes(0,&attributes));
#endif
}
const Vector3 &al::Listener::GetVelocity() const {return m_velocity;}

void al::Listener::SetOrientation(const Vector3 &at,const Vector3 &up)
{
	m_orientation = {at,up};
	auto atAudio = al::to_audio_direction(at);
	auto atUp = al::to_audio_direction(up);
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_listener.setOrientation(&atAudio[0],&atUp[0]);
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	auto &fmodSys = m_soundSystem.GetFMODSystem();
	FMOD_3D_ATTRIBUTES attributes;
	al::fmod::check_result(fmodSys.getListenerAttributes(0,&attributes));
	attributes.forward = {atAudio.x,atAudio.y,atAudio.z};
	attributes.up = {atUp.x,atUp.y,atUp.z};
	al::fmod::check_result(fmodSys.setListenerAttributes(0,&attributes));
#endif
}
const std::pair<Vector3,Vector3> &al::Listener::GetOrientation() const {return m_orientation;}

float al::Listener::GetMetersPerUnit() const {return m_metersPerUnit;}
void al::Listener::SetMetersPerUnit(float mu)
{
	m_metersPerUnit = mu;
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	m_listener.setMetersPerUnit(m_metersPerUnit);
#endif
}
