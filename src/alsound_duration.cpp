// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "alsoundsystem.hpp"

import pragma.audio.util;

bool al::get_sound_duration(const std::string path, float &duration) { return pragma::audio::util::get_duration(path, duration); }
