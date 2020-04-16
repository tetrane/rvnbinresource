#define BOOST_TEST_MODULE RVN_BINRESOURCE_READER
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "common.h"
#include "metadata.h"
#include "reader.h"

using MD = reven::binresource::Metadata;
using Reader = reven::binresource::Reader;

// Allows to write MD
class TestMDWriter : reven::binresource::MetadataWriter {
public:
	static MD dummy_md() {
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", "1.0.0", "Tests version 1.0.0", 42424242);
	}
};

BOOST_AUTO_TEST_CASE(bad_stream)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	ss->seekg(50000);

	BOOST_CHECK_THROW(Reader::open(std::move(ss)), reven::binresource::ReaderError);
}

BOOST_AUTO_TEST_CASE(bad_magic)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	const std::uint64_t bad_magic = 0x42424242424242;
	ss->write(reinterpret_cast<const char*>(&bad_magic), sizeof(bad_magic));

	BOOST_CHECK_THROW(Reader::open(std::move(ss)), reven::binresource::ReaderError);
}

BOOST_AUTO_TEST_CASE(no_metadata_version)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));

	BOOST_CHECK_THROW(Reader::open(std::move(ss)), reven::binresource::ReaderError);
}

BOOST_AUTO_TEST_CASE(metadata_version_in_the_future)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));

	const std::uint32_t metadata_version = reven::binresource::metadata_version + 1;
	ss->write(reinterpret_cast<const char*>(&metadata_version), sizeof(metadata_version));

	BOOST_CHECK_THROW(Reader::open(std::move(ss)), reven::binresource::ReaderError);
}

BOOST_AUTO_TEST_CASE(no_metadata)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));
	ss->write(reinterpret_cast<const char*>(&reven::binresource::metadata_version), sizeof(reven::binresource::metadata_version));

	BOOST_CHECK_THROW(Reader::open(std::move(ss)), reven::binresource::ReaderError);
}

BOOST_AUTO_TEST_CASE(check_metadata)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));
	ss->write(reinterpret_cast<const char*>(&reven::binresource::metadata_version), sizeof(reven::binresource::metadata_version));

	const auto md = TestMDWriter::dummy_md();
	md.serialize(*ss);

	auto reader = Reader::open(std::move(ss));

	BOOST_CHECK_EQUAL(md.type(), reader.metadata().type());
	BOOST_CHECK_EQUAL(md.format_version(), reader.metadata().format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), reader.metadata().tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), reader.metadata().tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), reader.metadata().tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), reader.metadata().generation_date());
}

BOOST_AUTO_TEST_CASE(read_uint64)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));
	ss->write(reinterpret_cast<const char*>(&reven::binresource::metadata_version), sizeof(reven::binresource::metadata_version));

	const auto md = TestMDWriter::dummy_md();
	md.serialize(*ss);

	const std::uint64_t foo = 0x42424242424242;
	ss->write(reinterpret_cast<const char*>(&foo), sizeof(foo));

	auto reader = Reader::open(std::move(ss));

	BOOST_CHECK_EQUAL(reader.stream().tellg(), reader.md_size());

	std::uint64_t bar = 0;
	reader.stream().read(reinterpret_cast<char*>(&bar), sizeof(bar));
	BOOST_CHECK_EQUAL(foo, bar);
	BOOST_CHECK_EQUAL(reader.stream().gcount(), sizeof(bar));
	BOOST_CHECK_EQUAL(reader.stream().tellg(), reader.md_size() + sizeof(bar));
}
