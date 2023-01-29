/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsound_coordinate_system.hpp"

static auto s_worldScale = 1.f;
void al::set_world_scale(float scale) { s_worldScale = scale; }

Vector3 al::to_audio_direction(const Vector3 &v) { return Vector3 {v.x, v.y, v.z}; }

Vector3 al::to_audio_position(const Vector3 &v) { return to_audio_direction(v * s_worldScale); }
Vector3 al::to_audio_scale(const Vector3 &v) { return Vector3 {v.x * s_worldScale, v.y * s_worldScale, v.z * s_worldScale}; }

Vector3 al::to_game_direction(const Vector3 &v) { return Vector3 {v.x, v.y, v.z}; }
Vector3 al::to_game_position(const Vector3 &v) { return to_game_direction(v) / s_worldScale; }
Vector3 al::to_game_scale(const Vector3 &v) { return Vector3 {v.x, v.y, v.z} / s_worldScale; }

float al::to_audio_distance(float r) { return r * s_worldScale; }
float al::to_game_distance(float r) { return r / s_worldScale; }
