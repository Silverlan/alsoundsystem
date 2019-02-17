/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUND_SETTINGS_HPP__
#define __ALSOUND_SETTINGS_HPP__

#include "alsound_definitions.hpp"

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace alure
{
	class Source;
};
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
namespace FMOD
{
	class Channel;
};
#endif

namespace al
{
#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
	using InternalSource = alure::Source;
#elif ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_FMOD
	using InternalSource = FMOD::Channel;
	namespace fmod
	{
		DLLALSYS void check_result(uint32_t r);
	};
#endif
};

#endif
