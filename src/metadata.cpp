#include "metadata.h"

#include <istream>
#include <ostream>

namespace reven {
namespace binresource {

void Metadata::serialize(std::ostream& out) const {
	const char padding[std::max(std::max(std::max(format_version_max_size, tool_name_max_size), tool_version_max_size), tool_info_max_size)] = {'\0'};

	try {
		out.write(reinterpret_cast<const char*>(&type_), sizeof(type_));

		const std::size_t format_version_size = format_version_.size();
		out.write(reinterpret_cast<const char*>(&format_version_size), sizeof(format_version_size));

		out.write(format_version_.c_str(), format_version_size);

		if (format_version_size < format_version_max_size) {
			out.write(padding, format_version_max_size - format_version_size);
		}

		const std::size_t tool_name_size = tool_name().size();
		out.write(reinterpret_cast<const char*>(&tool_name_size), sizeof(tool_name_size));

		out.write(tool_name_.c_str(), tool_name_size);

		if (tool_name_size < tool_name_max_size) {
			out.write(padding, tool_name_max_size - tool_name_size);
		}

		const std::size_t tool_version_size = tool_version_.size();
		out.write(reinterpret_cast<const char*>(&tool_version_size), sizeof(tool_version_size));

		out.write(tool_version_.c_str(), tool_version_size);

		if (tool_version_size < tool_version_max_size) {
			out.write(padding, tool_version_max_size - tool_version_size);
		}

		const std::size_t tool_info_size = tool_info_.size();
		out.write(reinterpret_cast<const char*>(&tool_info_size), sizeof(tool_info_size));

		out.write(tool_info_.c_str(), tool_info_size);

		if (tool_info_size < tool_info_max_size) {
			out.write(padding, tool_info_max_size - tool_info_size);
		}

		out.write(reinterpret_cast<const char*>(&generation_date_), sizeof(generation_date_));
	} catch(const std::ios_base::failure& e) {
		throw WriteMetadataError((std::string("IO error: ") + e.what()).c_str());
	}
}

Metadata Metadata::deserialize(std::uint32_t metadata_version, std::istream& in) {
	char padding[std::max(std::max(std::max(format_version_max_size, tool_name_max_size), tool_version_max_size), tool_info_max_size)] = {'\0'};

	Metadata md;

	try {
		in.read(reinterpret_cast<char*>(&md.type_), sizeof(md.type_));

		if (in.gcount() != sizeof(md.type_)) {
			throw ReadMetadataError("Can't read enough data for the type");
		}

		std::size_t format_version_size = 0;
		in.read(reinterpret_cast<char*>(&format_version_size), sizeof(format_version_size));

		if (in.gcount() != sizeof(format_version_size)) {
			throw ReadMetadataError("Can't read enough data for the format version size");
		}

		if (format_version_size > format_version_max_size) {
			throw ReadMetadataError("Format version size is greater than the maximum value");
		}

		md.format_version_ = std::string(format_version_size, '\0');
		in.read(&md.format_version_[0], format_version_size);

		if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != format_version_size) {
			throw ReadMetadataError("Can't read enough data for the format version");
		}

		if (format_version_size < format_version_max_size) {
			const std::size_t padding_size = format_version_max_size - format_version_size;
			in.read(padding, padding_size);

			if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != padding_size) {
				throw ReadMetadataError("Can't read enough data for the padding of the format version");
			}
		}

		std::size_t tool_name_size = 0;
		in.read(reinterpret_cast<char*>(&tool_name_size), sizeof(tool_name_size));

		if (in.gcount() != sizeof(tool_name_size)) {
			throw ReadMetadataError("Can't read enough data for the tool name size");
		}

		if (tool_name_size > tool_name_max_size) {
			throw ReadMetadataError("Tool name size is greater than the maximum value");
		}

		md.tool_name_ = std::string(tool_name_size, '\0');
		in.read(&md.tool_name_[0], tool_name_size);

		if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != tool_name_size) {
			throw ReadMetadataError("Can't read enough data for the tool name");
		}

		if (tool_name_size < tool_name_max_size) {
			const std::size_t padding_size = tool_name_max_size - tool_name_size;
			in.read(padding, padding_size);

			if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != padding_size) {
				throw ReadMetadataError("Can't read enough data for the padding of the tool name");
			}
		}

		if (metadata_version >= 1) {
			std::size_t tool_version_size = 0;
			in.read(reinterpret_cast<char*>(&tool_version_size), sizeof(tool_version_size));

			if (in.gcount() != sizeof(tool_version_size)) {
				throw ReadMetadataError("Can't read enough data for the tool version size");
			}

			if (tool_version_size > tool_version_max_size) {
				throw ReadMetadataError("Tool version size is greater than the maximum value");
			}

			md.tool_version_ = std::string(tool_version_size, '\0');
			in.read(&md.tool_version_[0], tool_version_size);

			if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != tool_version_size) {
				throw ReadMetadataError("Can't read enough data for the tool version");
			}

			if (tool_version_size < tool_version_max_size) {
				const std::size_t padding_size = tool_version_max_size - tool_version_size;
				in.read(padding, padding_size);

				if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != padding_size) {
					throw ReadMetadataError("Can't read enough data for the padding of the tool version");
				}
			}
		} else {
			md.tool_version_ = "1.0.0-prerelease";
		}

		std::size_t tool_info_size = 0;
		in.read(reinterpret_cast<char*>(&tool_info_size), sizeof(tool_info_size));

		if (in.gcount() != sizeof(tool_info_size)) {
			throw ReadMetadataError("Can't read enough data for the tool info size");
		}

		if (tool_info_size > tool_info_max_size) {
			throw ReadMetadataError("Tool info size is greater than the maximum value");
		}

		md.tool_info_ = std::string(tool_info_size, '\0');
		in.read(&md.tool_info_[0], tool_info_size);

		if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != tool_info_size) {
			throw ReadMetadataError("Can't read enough data for the tool info");
		}

		if (tool_info_size < tool_info_max_size) {
			const std::size_t padding_size = tool_info_max_size - tool_info_size;
			in.read(padding, padding_size);

			if (in.gcount() <= 0 || static_cast<std::size_t>(in.gcount()) != padding_size) {
				throw ReadMetadataError("Can't read enough data for the padding of the tool info");
			}
		}

		in.read(reinterpret_cast<char*>(&md.generation_date_), sizeof(md.generation_date_));

		if (in.gcount() != sizeof(md.generation_date_)) {
			throw ReadMetadataError("Can't read enough data for the generation date");
		}
	} catch(const std::ios_base::failure& e) {
		throw ReadMetadataError((std::string("IO error: ") + e.what()).c_str());
	}

	return md;
}

}} // namespace reven::binresource
