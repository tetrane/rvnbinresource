#define BOOST_TEST_MODULE RVN_BINRESOURCE_WRITER
#include <boost/test/unit_test.hpp>

#include <sstream>

#include "common.h"
#include "metadata.h"
#include "writer.h"

using MD = reven::binresource::Metadata;
using Writer = reven::binresource::Writer;

// Allows to write MD
class TestMDWriter : reven::binresource::MetadataWriter {
public:
	static MD dummy_md() {
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", "1.0.0", "Tests version 1.0.0", 42424242);
	}

	static MD dummy_md2() {
		return write(24, "1.2.0-dummy", "TestMetaDataWriter2", "1.2.0", "Tests version 1.2.0", 42424243);
	}
};

BOOST_AUTO_TEST_CASE(bad_stream)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	ss->seekg(50000);

	BOOST_CHECK_THROW(Writer::create(std::move(ss), TestMDWriter::dummy_md()), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(written_metadata)
{
	const auto md = TestMDWriter::dummy_md();
	auto writer = Writer::create(std::make_unique<std::stringstream>(), md);
	const auto md_size = writer.md_size();

	auto stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	stream->seekg(0);

	std::uint64_t written_magic = 0;
	stream->read(reinterpret_cast<char*>(&written_magic), sizeof(written_magic));

	BOOST_CHECK_EQUAL(written_magic, reven::binresource::magic);

	std::uint32_t written_metadata_version = 0;
	stream->read(reinterpret_cast<char*>(&written_metadata_version), sizeof(written_metadata_version));

	BOOST_CHECK_EQUAL(written_metadata_version, reven::binresource::metadata_version);

	const auto md2 = MD::deserialize(written_metadata_version, *stream);

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());

	BOOST_CHECK_EQUAL(stream->tellg(), md_size);
}

BOOST_AUTO_TEST_CASE(open_bad_stream)
{
	auto stream = std::make_unique<std::stringstream>();

	stream->seekg(50000);

	BOOST_CHECK_THROW(Writer::open(std::move(stream)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(open_bad_magic)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();

	const std::uint64_t bad_magic = 0x42424242424242;
	ss->write(reinterpret_cast<const char*>(&bad_magic), sizeof(bad_magic));

	BOOST_CHECK_THROW(Writer::open(std::move(ss)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(open_no_metadata_version)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));

	BOOST_CHECK_THROW(Writer::open(std::move(ss)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(metadata_version_in_the_past)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));

	const std::uint32_t metadata_version = 0;
	ss->write(reinterpret_cast<const char*>(&metadata_version), sizeof(metadata_version));

	BOOST_CHECK_THROW(Writer::open(std::move(ss)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(metadata_version_in_the_future)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));

	const std::uint32_t metadata_version = reven::binresource::metadata_version + 1;
	ss->write(reinterpret_cast<const char*>(&metadata_version), sizeof(metadata_version));

	BOOST_CHECK_THROW(Writer::open(std::move(ss)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(open_no_metadata)
{
	std::unique_ptr<std::stringstream> ss = std::make_unique<std::stringstream>();
	ss->write(reinterpret_cast<const char*>(&reven::binresource::magic), sizeof(reven::binresource::magic));
	ss->write(reinterpret_cast<const char*>(&reven::binresource::metadata_version), sizeof(reven::binresource::metadata_version));

	BOOST_CHECK_THROW(Writer::open(std::move(ss)), reven::binresource::WriterError);
}

BOOST_AUTO_TEST_CASE(open)
{
	auto writer = Writer::create(std::make_unique<std::stringstream>(), TestMDWriter::dummy_md());

	const std::uint64_t foo = 0x42424242424242;
	writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size() + sizeof(foo));

	auto stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));

	stream->seekg(writer.md_size());

	std::uint64_t written_foo = 0;
	stream->read(reinterpret_cast<char*>(&written_foo), sizeof(written_foo));

	BOOST_CHECK_EQUAL(foo, written_foo);

	writer = Writer::open(std::move(stream));
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size());

	const std::uint64_t foo2 = 0x42424242424242 + 1;
	writer.stream().write(reinterpret_cast<const char*>(&foo2), sizeof(foo2));
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size() + sizeof(foo2));

	stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));

	stream->seekg(writer.md_size());

	std::uint64_t written_foo2 = 0;
	stream->read(reinterpret_cast<char*>(&written_foo2), sizeof(written_foo2));

	BOOST_CHECK_EQUAL(foo2, written_foo2);
}

BOOST_AUTO_TEST_CASE(set_metadata)
{
	auto md = TestMDWriter::dummy_md();
	auto writer = Writer::create(std::make_unique<std::stringstream>(), md);
	const auto md_size = writer.md_size();

	auto stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));

	writer = Writer::open(std::move(stream));
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size());

	md = TestMDWriter::dummy_md2();
	writer.set_metadata(md);
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size());

	stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	stream->seekg(0);

	std::uint64_t written_magic = 0;
	stream->read(reinterpret_cast<char*>(&written_magic), sizeof(written_magic));

	BOOST_CHECK_EQUAL(written_magic, reven::binresource::magic);

	std::uint32_t written_metadata_version = 0;
	stream->read(reinterpret_cast<char*>(&written_metadata_version), sizeof(written_metadata_version));

	BOOST_CHECK_EQUAL(written_metadata_version, reven::binresource::metadata_version);

	const auto md2 = MD::deserialize(written_metadata_version, *stream);

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());

	BOOST_CHECK_EQUAL(stream->tellg(), md_size);
}

BOOST_AUTO_TEST_CASE(write_uint64)
{
	const auto md = TestMDWriter::dummy_md();
	auto writer = Writer::create(std::make_unique<std::stringstream>(), md);
	const auto md_size = writer.md_size();

	const std::uint64_t foo = 0x42424242424242;
	writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));
	BOOST_CHECK_EQUAL(writer.stream().tellp(), writer.md_size() + sizeof(foo));

	auto stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	stream->seekg(md_size);

	std::uint64_t written_foo = 0;
	stream->read(reinterpret_cast<char*>(&written_foo), sizeof(written_foo));

	BOOST_CHECK_EQUAL(foo, written_foo);
}
