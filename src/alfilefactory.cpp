/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "alsoundsystem.hpp"
#include "alsoundsystem_filefactory.hpp"
#include <fsys/filesystem_stream.hpp>

#if ALSYS_LIBRARY_TYPE == ALSYS_LIBRARY_ALURE
namespace al
{
	class StreamBuf
		: public fsys::BaseStreamBuf
	{
	public:
		StreamBuf()=default;
		virtual ~StreamBuf() override final=default;
		virtual int_type underflow() override final;
		virtual pos_type seekoff(off_type offset,std::ios_base::seekdir whence,std::ios_base::openmode mode) override final;
		virtual pos_type seekpos(pos_type pos,std::ios_base::openmode mode) override final;
	private:
		using BufferArrayT = std::array<traits_type::char_type,4096>;
		BufferArrayT mBuffer;
	};
};

std::streambuf::pos_type al::StreamBuf::seekoff(off_type offset,std::ios_base::seekdir whence,std::ios_base::openmode mode)
{
	if(!mFile || (mode &std::ios_base::out) || !(mode &std::ios_base::in))
		return traits_type::eof();

	// PhysFS only seeks using absolute offsets, so we have to convert cur-
	// and end-relative offsets.
	uint64_t fpos;
	switch(whence)
	{
		case std::ios_base::beg:
			break;
		case std::ios_base::cur:
			// Need to offset for the read pointers with std::ios_base::cur
			// regardless
			offset -= off_type(egptr() -gptr());
			if((fpos = mFile->Tell()) == -1)
				return traits_type::eof();
			offset += fpos;
			break;
		case std::ios_base::end:
			if((fpos = mFile->GetSize()) == -1)
				return traits_type::eof();
			offset += fpos;
			break;
		default:
			return traits_type::eof();
	}
	// Range check - absolute offsets cannot be less than 0, nor be greater
	// than PhysFS's offset type.
	if(offset < 0 || offset >= std::numeric_limits<decltype(fpos)>::max())
		return traits_type::eof();
	mFile->Seek(offset);
	if(mFile->Eof() == true)
		return traits_type::eof();
	// Clear read pointers so underflow() gets called on the next read
	// attempt.
	setg(0,0,0);
	return offset;
}

std::streambuf::pos_type al::StreamBuf::seekpos(pos_type pos,std::ios_base::openmode mode)
{
	// Simplified version of seekoff
	if(!mFile || (mode&std::ios_base::out) || !(mode&std::ios_base::in))
		return traits_type::eof();

	if(pos < 0)
		return traits_type::eof();
	mFile->Seek(pos);
	if(mFile->Eof() == true)
		return traits_type::eof();
	setg(0,0,0);
	return pos;
}

std::streambuf::int_type al::StreamBuf::underflow()
{
	if(mFile && gptr() == egptr())
	{
		// Read in the next chunk of data, and set the read pointers on
		// success
		auto got = mFile->Read(mBuffer.data(),sizeof(BufferArrayT::value_type),mBuffer.size());
		if(got != std::numeric_limits<decltype(got)>::max())
			setg(mBuffer.data(),mBuffer.data(),mBuffer.data() +got);
	}
	if(gptr() == egptr())
		return traits_type::eof();
	return traits_type::to_int_type(*gptr());
}

///////////////////

alure::UniquePtr<std::istream> al::FileFactory::openFile(const alure::String &name)
{
	auto stream = alure::MakeUnique<fsys::Stream>(name.c_str(),new al::StreamBuf());
	if(stream->fail())
		stream = nullptr;
	return std::move(stream);
}
#endif
