#pragma once

#include <istream>
#include <memory>

#include "metadata.h"

namespace reven {
namespace binresource {

///
/// Exception that occurs when there is an error in the reading
///
class ReaderError : public std::runtime_error {
public:
	ReaderError(const char* msg) : std::runtime_error(msg) {}
};

///
/// Reader class used kinda like a std::istream but with the abstraction of the metadata
/// The user could use independently a std::istream and this class without caring about the offset
/// because the offset 0 will be the position right after the metadata in the stream
///
class Reader {
public:
	///
	/// \brief open Open a resource from the filename passed in parameter
	/// \param filename The filename of the resource to open
	/// \throws ReaderError if an error occurs during the reading of the file
	static Reader open(const char* filename);

	///
	/// \brief open Open a resource from a stream passed in parameter
	/// \param stream The stream to read
	/// \throws ReaderError if an error occurs during the reading of the stream
	static Reader open(std::unique_ptr<std::istream>&& stream);

public:
	//! Return the stream used
	std::istream& stream() {
		return *stream_;
	}

	//! The size of the metadata (the offset from the beginning of the file to the position 0 for the user)
	std::size_t md_size() const {
		return md_size_;
	}

	//! Returns the metadata read at the opening
	const Metadata& metadata() const { return md_; }

private:
	Reader(std::unique_ptr<std::istream>&& stream) : stream_{std::move(stream)} {
		stream_->seekg(0);
	}

	Metadata read_metadata(std::uint32_t metadata_version);

private:
	//! Stored in a pointer because ostream itself is not movable
	std::unique_ptr<std::istream> stream_;

	Metadata md_;
	std::size_t md_size_;
};

}} // namespace reven::binresource
