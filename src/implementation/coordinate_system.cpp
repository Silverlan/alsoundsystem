// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.soundsystem;

import :coordinate_system;

static auto s_worldScale = 1.f;
void pragma::audio::set_world_scale(float scale) { s_worldScale = scale; }

Vector3 pragma::audio::to_audio_direction(const Vector3 &v) { return Vector3 {v.x, v.y, v.z}; }

Vector3 pragma::audio::to_audio_position(const Vector3 &v) { return to_audio_direction(v * s_worldScale); }
Vector3 pragma::audio::to_audio_scale(const Vector3 &v) { return Vector3 {v.x * s_worldScale, v.y * s_worldScale, v.z * s_worldScale}; }

Vector3 pragma::audio::to_game_direction(const Vector3 &v) { return Vector3 {v.x, v.y, v.z}; }
Vector3 pragma::audio::to_game_position(const Vector3 &v) { return to_game_direction(v) / s_worldScale; }
Vector3 pragma::audio::to_game_scale(const Vector3 &v) { return Vector3 {v.x, v.y, v.z} / s_worldScale; }

float pragma::audio::to_audio_distance(float r) { return r * s_worldScale; }
float pragma::audio::to_game_distance(float r) { return r / s_worldScale; }
