/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef __ALSOUNDSYSTEM_FILEFACTORY_HPP__
#define __ALSOUNDSYSTEM_FILEFACTORY_HPP__

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
#include <AL/alure2.h>

// Inherit from alure::FileIOFactory to use our custom istream
namespace al
{
	class FileFactory : public alure::FileIOFactory
	{
	public:
		FileFactory()=default;
		virtual ~FileFactory() override final=default;
		virtual alure::UniquePtr<std::istream> openFile(const alure::String &name) override final;
	};
};
#endif

#endif
