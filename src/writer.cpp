#include "writer.h"
#include "common.h"

#include <fstream>

namespace reven {
namespace binresource {

Writer Writer::create(const char* filename, const Metadata& md) {
	return Writer::create(std::make_unique<std::ofstream>(filename, std::ios::binary | std::ios::trunc), md);
}

Writer Writer::create(std::unique_ptr<std::ostream>&& stream, const Metadata& md) {
	Writer writer(std::move(stream));

	if (!*writer.stream_) {
		throw WriterError("Bad stream");
	}

	writer.stream_->write(reinterpret_cast<const char*>(&magic), sizeof(magic));
	writer.stream_->write(reinterpret_cast<const char*>(&metadata_version), sizeof(metadata_version));
	writer.write_metadata(md);

	writer.md_size_ = writer.stream_->tellp();

	return writer;
}

Writer Writer::open(const char* filename) {
	return Writer::open(std::make_unique<std::fstream>(filename, std::ios::binary | std::ios::in | std::ios::out));
}

Writer Writer::open(std::unique_ptr<std::iostream>&& stream) {
	if (!*stream) {
		throw WriterError("Bad stream");
	}

	stream->seekg(0);

	std::uint64_t magic = 0;
	stream->read(reinterpret_cast<char*>(&magic), sizeof(magic));

	std::uint32_t metadata_version = 0;
	if (magic == ::reven::binresource::magic) {
		stream->read(reinterpret_cast<char*>(&metadata_version), sizeof(metadata_version));

		if (stream->gcount() != sizeof(metadata_version)) {
			throw WriterError("Can't read enough data for the metadata version");
		}
	}
	// Before having the metadata_version in the file we used to have this magic and because we couldn't simply add
	// a new field and preserve the compatibility with the previous version we changed the magic to the new one
	else if (magic == 0x7262696e72737263) {
		metadata_version = 0;
	} else {
		throw WriterError("Wrong magic");
	}

	if (metadata_version != ::reven::binresource::metadata_version) {
		throw WriterError("Writer can't open resource with different metadata version than the current");
	}

	try {
		const auto md = Metadata::deserialize(metadata_version, *stream);
	} catch (const MetadataError& e) {
		throw WriterError((std::string("While reading metadata: ") + e.what()).c_str());
	}

	const std::size_t md_size = stream->tellg();

	Writer writer(std::move(stream));

	writer.md_size_ = md_size;
	writer.stream_->seekp(md_size);

	return writer;
}

void Writer::set_metadata(const Metadata& md) {
	const auto previous_pos = stream_->tellp();

	stream_->seekp(sizeof(magic) + sizeof(metadata_version));
	write_metadata(md);

	stream_->seekp(previous_pos);
}

void Writer::write_metadata(const Metadata& md) {
	try {
		return md.serialize(*stream_);
	} catch (const MetadataError& e) {
		throw WriterError((std::string("While writing metadata: ") + e.what()).c_str());
	}
}

}} // namespace reven::binresource
