/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"

import pragma.audio.util;

bool al::get_sound_duration(const std::string path, float &duration) { return pragma::audio::util::get_duration(path, duration); }
