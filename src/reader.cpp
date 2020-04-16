#include "reader.h"
#include "common.h"

#include <cassert>
#include <fstream>

namespace reven {
namespace binresource {

Reader Reader::open(const char* filename) {
	return Reader::open(std::make_unique<std::ifstream>(filename, std::ios::binary | std::ios::in));
}

Reader Reader::open(std::unique_ptr<std::istream>&& stream) {
	Reader reader(std::move(stream));

	if (!*reader.stream_) {
		throw ReaderError("Bad stream");
	}

	std::uint64_t magic = 0;
	reader.stream_->read(reinterpret_cast<char*>(&magic), sizeof(magic));

	std::uint32_t metadata_version = 0;
	if (magic == ::reven::binresource::magic) {
		reader.stream_->read(reinterpret_cast<char*>(&metadata_version), sizeof(metadata_version));

		if (reader.stream_->gcount() != sizeof(metadata_version)) {
			throw ReaderError("Can't read enough data for the metadata version");
		}

		if (metadata_version > ::reven::binresource::metadata_version) {
			throw ReaderError("Metadata version in the future");
		}
	}
	// Before having the metadata_version in the file we used to have this magic and because we couldn't simply add
	// a new field and preserve the compatibility with the previous version we changed the magic to the new one
	else if (magic == 0x7262696e72737263) {
		metadata_version = 0;
	} else {
		throw ReaderError("Wrong magic");
	}

	reader.md_ = reader.read_metadata(metadata_version);
	reader.md_size_ = reader.stream_->tellg();

	return reader;
}

Metadata Reader::read_metadata(std::uint32_t metadata_version) {
	try {
		return Metadata::deserialize(metadata_version, *stream_);
	} catch (const MetadataError& e) {
		throw ReaderError((std::string("While reading metadata: ") + e.what()).c_str());
	}
}

}} // namespace reven::binresource
