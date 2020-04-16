#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

namespace reven {
namespace binresource {

///
/// Root metadata exception. Catch this exception to catch all exceptions related to metadata.
///
class MetadataError : public std::runtime_error {
public:
	MetadataError(const char* msg) : std::runtime_error(msg) {}
};

///
/// Exception that occurs when it is not possible to serialize the metadata.
///
class WriteMetadataError : public MetadataError {
public:
	WriteMetadataError(const char* msg) : MetadataError(msg) {}
};

///
/// Exception that occurs when it is not possible to deserialize the metadata.
///
class ReadMetadataError : public MetadataError {
public:
	ReadMetadataError(const char* msg) : MetadataError(msg) {}
};

constexpr std::size_t format_version_max_size = 512;
constexpr std::size_t tool_name_max_size = 512;
constexpr std::size_t tool_version_max_size = 512;
constexpr std::size_t tool_info_max_size = 2048;

///
/// Raw Metadata class that contains the metadata in the format stored and retrieved by the reader.
/// It is not to be used directly by clients, instead use converters that give a semantics to the metadata
///
class Metadata {
public:
	static Metadata deserialize(std::uint32_t metadata_version, std::istream& in);

public:
	/// Magic representing the resource type
	std::uint32_t type() const { return type_; }
	/// A version for the resource file format (should be of format "x.y.z-suffix" with an optional suffix)
	const std::string& format_version() const { return format_version_; }
	/// Name of the tool that generated the resource
	const std::string& tool_name() const { return tool_name_; }
	/// A version for the tool that generated the resource (should be of format "x.y.z-suffix" with an optional suffix)
	const std::string& tool_version() const { return tool_version_; }
	/// The version of the tool and possibly the version of the writer library used
	const std::string& tool_info() const { return tool_info_; }
	/// The date of the generation
	std::uint64_t generation_date() const { return generation_date_; }

	void serialize(std::ostream& out) const;

private:
	// General clients are not expected to be able to build Metadata
	Metadata() = default;

	std::uint32_t type_;
	std::string format_version_;
	std::string tool_name_;
	std::string tool_version_;
	std::string tool_info_;
	std::uint64_t generation_date_;

	// Special class that is allowed to build Metadata
	friend class MetadataWriter;
	// Special permission for Reader to build Metadata
	friend class Reader;
};

///
/// Special purpose class that is meant to be subclassed to create classes able to build Metadata.
/// Subclasses can call the static method `write` to build a Metadata.
///
/// Example:
///
/// ```cpp
/// class TestMDWriter : MetadataWriter {
/// public:
/// 	static constexpr std::uint32_t type = 42;
/// 	static constexpr char format_version[] = "1.0.0-dummy";
/// 	static constexpr char tool_name[] = "TestMetaDataWriter";
/// 	static constexpr char tool_version[] = "1.0.0-dummy";
/// 	static constexpr char tool_info[] = "Tests version 1.0.0";
/// 	static constexpr std::uint64_t generation_date = 42424242;
///
/// 	static Metadata dummy_md() {
/// 		return write(type, format_version, tool_name, tool_info, generation_date);
/// 	}
/// };
///
/// // clients can then call `TestMDWriter::dummy_md()` to build metadata.
/// ```
class MetadataWriter {
protected:
	static Metadata write(std::uint32_t type, std::string format_version,
	                      std::string tool_name, std::string tool_version, std::string tool_info,
	                      std::uint64_t generation_date) {
		if (format_version.size() > format_version_max_size) {
			throw WriteMetadataError(
				("Format version too long, max size is " + std::to_string(format_version_max_size)).c_str()
			);
		}

		if (tool_name.size() > tool_name_max_size) {
			throw WriteMetadataError(
				("Tool name too long, max size is " + std::to_string(tool_name_max_size)).c_str()
			);
		}

		if (tool_version.size() > tool_version_max_size) {
			throw WriteMetadataError(
				("Tool version too long, max size is " + std::to_string(tool_version_max_size)).c_str()
			);
		}

		if (tool_info.size() > tool_info_max_size) {
			throw WriteMetadataError(
				("Tool info too long, max size is " + std::to_string(tool_info_max_size)).c_str()
			);
		}

		Metadata md;
		md.type_ = type;
		md.format_version_ = std::move(format_version);
		md.tool_name_ = std::move(tool_name);
		md.tool_version_ = std::move(tool_version);
		md.tool_info_ = std::move(tool_info);
		md.generation_date_ = generation_date;
		return md;
	}
};

}} // namespace reven::binresource
