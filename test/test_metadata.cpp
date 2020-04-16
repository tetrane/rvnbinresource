#define BOOST_TEST_MODULE RVN_BINRESOURCE_METADATA
#include <boost/test/unit_test.hpp>

#include <sstream>
#include <experimental/string_view>

#include "common.h"
#include "metadata.h"

using MD = reven::binresource::Metadata;

// Allows to write MD
class TestMDWriter : reven::binresource::MetadataWriter {
public:
	static MD format_version_too_long() {
		const std::string format_version(reven::binresource::format_version_max_size + 1, '\0');
		return write(42, format_version, "TestMetaDataWriter", "1.0.0", "Tests version 1.0.0", 42424242);
	}

	static MD tool_name_too_long() {
		const std::string tool_name(reven::binresource::tool_name_max_size + 1, '\0');
		return write(42, "1.0.0-dummy", tool_name, "1.0.0", "Tests version 1.0.0", 42424242);
	}

	static MD tool_version_too_long() {
		const std::string tool_version(reven::binresource::tool_version_max_size + 1, '\0');
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", tool_version, "Tests version 1.0.0", 42424242);
	}

	static MD tool_info_too_long() {
		const std::string tool_info(reven::binresource::tool_info_max_size + 1, '\0');
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", "1.0.0", tool_info, 42424242);
	}

	static MD dummy_md() {
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", "1.0.0", "Tests version 1.0.0", 42424242);
	}
};

BOOST_AUTO_TEST_CASE(serialize_deserialize)
{
	const auto md = TestMDWriter::dummy_md();

	std::stringstream stream;
	md.serialize(stream);

	const auto md2 = MD::deserialize(reven::binresource::metadata_version, stream);

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());
}

BOOST_AUTO_TEST_CASE(serialize_failed_stream)
{
	std::stringstream stream;

	// Fail the stream and allow it to send an exception when using it
	stream.seekg(50000);
	try {
		stream.exceptions(std::ios_base::failbit);
	} catch(...) {}

	const auto md = TestMDWriter::dummy_md();
	BOOST_CHECK_THROW(md.serialize(stream), reven::binresource::WriteMetadataError);
}

BOOST_AUTO_TEST_CASE(deserialize_fake_stream)
{
	std::stringstream stream;

	// Fail the stream and allow it to send an exception when using it
	stream.seekg(50000);
	try {
		stream.exceptions(std::ios_base::failbit);
	} catch(...) {}

	BOOST_CHECK_THROW(MD::deserialize(reven::binresource::metadata_version, stream), reven::binresource::ReadMetadataError);
}

BOOST_AUTO_TEST_CASE(deserialize_empty)
{
	std::stringstream stream;
	BOOST_CHECK_THROW(MD::deserialize(reven::binresource::metadata_version, stream), reven::binresource::ReadMetadataError);
}

void test_bad_format(std::uint32_t metadata_version) {
	std::stringstream stream;

	///////////////////////////
	// TYPE
	///////////////////////////

	const std::uint32_t type = 42;
	stream.write(reinterpret_cast<const char*>(&type), sizeof(type));

	///////////////////////////
	// FORMAT VERSION
	///////////////////////////

	// Can't read enough data for the format version size
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	std::size_t format_version_size = reven::binresource::format_version_max_size + 1;
	stream.write(reinterpret_cast<const char*>(&format_version_size), sizeof(format_version_size));

	// Format version size is greater than the maximum value
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	format_version_size = reven::binresource::format_version_max_size - 20;

	stream.seekp(-sizeof(format_version_size), std::ios_base::cur);
	stream.write(reinterpret_cast<const char*>(&format_version_size), sizeof(format_version_size));

	// Can't read enough data for the format version
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	const char format_version[reven::binresource::format_version_max_size] = {'\0'};
	stream.write(format_version, format_version_size);

	// Can't read enough data for the padding of the format version
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	stream.write(format_version, reven::binresource::format_version_max_size - format_version_size);

	///////////////////////////
	// TOOL NAME
	///////////////////////////

	// Can't read enough data for the tool name size
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	std::size_t tool_name_size = reven::binresource::tool_name_max_size + 1;
	stream.write(reinterpret_cast<const char*>(&tool_name_size), sizeof(tool_name_size));

	// Tool name size is greater than the maximum value
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	tool_name_size = reven::binresource::tool_name_max_size - 20;

	stream.seekp(-sizeof(tool_name_size), std::ios_base::cur);
	stream.write(reinterpret_cast<const char*>(&tool_name_size), sizeof(tool_name_size));

	// Can't read enough data for the tool name
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	const char tool_name[reven::binresource::tool_name_max_size] = {'\10'};
	stream.write(tool_name, tool_name_size);

	// Can't read enough data for the padding of the tool name
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	stream.write(tool_name, reven::binresource::tool_name_max_size - tool_name_size);

	///////////////////////////
	// TOOL VERSION
	///////////////////////////

	std::size_t tool_version_size = reven::binresource::tool_version_max_size + 1;
	const char tool_version[reven::binresource::tool_version_max_size] = {'\10'};

	if (metadata_version >= 1) {
		// Can't read enough data for the tool version size
		BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
		stream.clear();
		stream.seekg(0, std::ios_base::beg);
		stream.seekp(0, std::ios_base::end);

		stream.write(reinterpret_cast<const char*>(&tool_version_size), sizeof(tool_version_size));

		// Tool version size is greater than the maximum value
		BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
		stream.clear();
		stream.seekg(0, std::ios_base::beg);
		stream.seekp(0, std::ios_base::end);

		tool_version_size = reven::binresource::tool_version_max_size - 20;

		stream.seekp(-sizeof(tool_version_size), std::ios_base::cur);
		stream.write(reinterpret_cast<const char*>(&tool_version_size), sizeof(tool_version_size));

		// Can't read enough data for the tool version
		BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
		stream.clear();
		stream.seekg(0, std::ios_base::beg);
		stream.seekp(0, std::ios_base::end);

		stream.write(tool_version, tool_version_size);

		// Can't read enough data for the padding of the tool version
		BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
		stream.clear();
		stream.seekg(0, std::ios_base::beg);
		stream.seekp(0, std::ios_base::end);

		stream.write(tool_version, reven::binresource::tool_version_max_size - tool_version_size);
	}

	///////////////////////////
	// TOOL INFO
	///////////////////////////

	// Can't read enough data for the tool info size
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	std::size_t tool_info_size = reven::binresource::tool_info_max_size + 1;
	stream.write(reinterpret_cast<const char*>(&tool_info_size), sizeof(tool_info_size));

	// Tool info size is greater than the maximum value
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	tool_info_size = reven::binresource::tool_info_max_size - 20;

	stream.seekp(-sizeof(tool_info_size), std::ios_base::cur);
	stream.write(reinterpret_cast<const char*>(&tool_info_size), sizeof(tool_info_size));

	// Can't read enough data for the tool info
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	const char tool_info[reven::binresource::tool_info_max_size] = {'\20'};
	stream.write(tool_info, tool_info_size);

	// Can't read enough data for the padding of the tool info
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	stream.write(tool_info, reven::binresource::tool_info_max_size - tool_info_size);

	///////////////////////////
	// GENERATION DATE
	///////////////////////////

	// Can't read enough data for the generation date
	BOOST_CHECK_THROW(MD::deserialize(metadata_version, stream), reven::binresource::ReadMetadataError);
	stream.clear();
	stream.seekg(0, std::ios_base::beg);
	stream.seekp(0, std::ios_base::end);

	const std::uint64_t generation_date = 0x42424242424242;
	stream.write(reinterpret_cast<const char*>(&generation_date), sizeof(generation_date));

	auto md = MD::deserialize(metadata_version, stream);

	BOOST_CHECK_EQUAL(md.type(), type);
	BOOST_CHECK_EQUAL(md.format_version(), std::experimental::string_view(format_version, format_version_size));
	BOOST_CHECK_EQUAL(md.tool_name(), std::experimental::string_view(tool_name, tool_name_size));

	if (metadata_version >= 1) {
		BOOST_CHECK_EQUAL(md.tool_version(), std::experimental::string_view(tool_version, tool_version_size));
	} else {
		BOOST_CHECK_EQUAL(md.tool_version(), "1.0.0-prerelease");
	}

	BOOST_CHECK_EQUAL(md.tool_info(), std::experimental::string_view(tool_info, tool_info_size));
	BOOST_CHECK_EQUAL(md.generation_date(), generation_date);
}

BOOST_AUTO_TEST_CASE(deserialize_bad_format_v0)
{
	test_bad_format(0);
}

BOOST_AUTO_TEST_CASE(deserialize_bad_format_v1)
{
	test_bad_format(1);
}

BOOST_AUTO_TEST_CASE(format_version_too_long)
{
	BOOST_CHECK_THROW(TestMDWriter::format_version_too_long(), reven::binresource::WriteMetadataError);
}

BOOST_AUTO_TEST_CASE(tool_name_too_long)
{
	BOOST_CHECK_THROW(TestMDWriter::tool_name_too_long(), reven::binresource::WriteMetadataError);
}

BOOST_AUTO_TEST_CASE(tool_version_too_long)
{
	BOOST_CHECK_THROW(TestMDWriter::tool_version_too_long(), reven::binresource::WriteMetadataError);
}

BOOST_AUTO_TEST_CASE(tool_info_too_long)
{
	BOOST_CHECK_THROW(TestMDWriter::tool_info_too_long(), reven::binresource::WriteMetadataError);
}
