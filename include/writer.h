#pragma once

#include <ostream>
#include <memory>

#include "metadata.h"

namespace reven {
namespace binresource {

///
/// Exception that occurs when there is an error in the writing
///
class WriterError : public std::runtime_error {
public:
	WriterError(const char* msg) : std::runtime_error(msg) {}
};

///
/// Writer class used kinda like a std::ostream but with the abstraction of the metadata
/// The user could use independently a std::ostream and this class without caring about the offset
/// because the offset 0 will be the position right after the metadata in the stream
///
class Writer {
public:
	///
	/// \brief create Create a resource with the metadata and filename passed in parameter
	/// \param filename The filename of the resource to open
	/// \param md The metadata to write in the file
	/// \throws WriterError if an error occurs during the writing of the file
	static Writer create(const char* filename, const Metadata& md);

	///
	/// \brief create Create a resource with the metadata and stream passed in parameter
	/// \param stream The stream to write
	/// \param md The metadata to write in the file
	/// \throws WriterError if an error occurs during the writing of the stream
	static Writer create(std::unique_ptr<std::ostream>&& stream, const Metadata& md);

	///
	/// \brief open Open an already versioned resource with the filename passed in parameter
	/// \param filename The filename of the resource to open
	/// \throws WriterError if an error occurs during the reading of the file
	static Writer open(const char* filename);

	///
	/// \brief open Open an already versioned resource with the stream passed in parameter
	/// \param stream The stream to write
	/// \throws WriterError if an error occurs during the reading of the stream
	static Writer open(std::unique_ptr<std::iostream>&& stream);

public:
	//! Return the stream used
	std::ostream& stream() {
		return *stream_;
	}

	//! Retrieve the stream in case someone want to access it after the end of the writing
	std::unique_ptr<std::ostream>&& finalize() && {
		return std::move(stream_);
	}

	//! The size of the metadata (the offset from the beginning of the file to the position 0 for the user)
	std::size_t md_size() const {
		return md_size_;
	}

	///
	/// \brief set_metadata Update the metadata of an already existing resource
	/// \param md The metadata to write in the resource
	/// \throws WriterError if an error occurs during the writing of the resource
	void set_metadata(const Metadata& md);

private:
	Writer(std::unique_ptr<std::ostream>&& stream) : stream_{std::move(stream)} {
		stream_->seekp(0);
	}

	void write_metadata(const Metadata& md);

private:
	//! Stored in a pointer because ostream itself is not movable
	std::unique_ptr<std::ostream> stream_;

	std::size_t md_size_;
};

}} // namespace reven::binresource
