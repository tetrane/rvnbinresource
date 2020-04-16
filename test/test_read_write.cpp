#define BOOST_TEST_MODULE RVN_BINRESOURCE_READ_WRITE
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

#include "common.h"
#include "metadata.h"
#include "reader.h"
#include "writer.h"

using MD = reven::binresource::Metadata;
using Reader = reven::binresource::Reader;
using Writer = reven::binresource::Writer;

class TestMDWriter : reven::binresource::MetadataWriter {
public:
	static MD dummy_md() {
		return write(42, "1.0.0-dummy", "TestMetaDataWriter", "1.0.0", "Tests version 1.0.0", 42424242);
	}

	static MD dummy_md2() {
		return write(24, "1.2.0-dummy", "TestMetaDataWriter2", "1.2.0", "Tests version 1.2.0", 42424243);
	}
};

struct transient_directory {
	//! Path of created directory.
	boost::filesystem::path path;

	//! Create a uniquely named temporary directory in base_dir.
	//! A suffix is generated and appended to the given prefix to ensure the directory name is unique.
	//! Throw if directory cannot be created.
	transient_directory(const boost::filesystem::path& base_dir = boost::filesystem::temp_directory_path(),
	                    std::string prefix = {}) {
		boost::filesystem::path tmp_path = boost::filesystem::unique_path(prefix + "%%%%-%%%%-%%%%-%%%%");
		tmp_path = base_dir / tmp_path;

		if (!boost::filesystem::create_directories(tmp_path)) {
			throw std::runtime_error(("Can't create the directory " + tmp_path.native()).c_str());
		}

		this->path = tmp_path;
	}

	//! Delete created directory.
	~transient_directory() {
		boost::filesystem::remove_all(this->path);
	}
};

constexpr std::uint64_t foo = 0x42424242424242;

BOOST_AUTO_TEST_CASE(read_write_stringstream)
{
	const auto md = TestMDWriter::dummy_md();
	auto writer = Writer::create(std::make_unique<std::stringstream>(), md);

	writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));

	auto stream = std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	stream->seekg(0);
	stream->seekp(0);

	auto reader = Reader::open(std::move(stream));

	std::uint64_t bar = 0;
	reader.stream().read(reinterpret_cast<char*>(&bar), sizeof(bar));

	BOOST_CHECK_EQUAL(reader.stream().gcount(), sizeof(bar));
	BOOST_CHECK_EQUAL(foo, bar);

	const auto md2 = reader.metadata();

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());
}

BOOST_AUTO_TEST_CASE(read_write_file)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "foo.bin";

	const auto md = TestMDWriter::dummy_md();

	{
		auto writer = Writer::create(tmp_file.c_str(), md);

		writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));
	}

	auto reader = Reader::open(tmp_file.c_str());

	std::uint64_t bar = 0;
	reader.stream().read(reinterpret_cast<char*>(&bar), sizeof(bar));

	BOOST_CHECK_EQUAL(reader.stream().gcount(), sizeof(bar));
	BOOST_CHECK_EQUAL(foo, bar);

	const auto md2 = reader.metadata();

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());
}

BOOST_AUTO_TEST_CASE(read_write_set_metadata_stringstream)
{
	auto md = TestMDWriter::dummy_md();

	auto stream = ([&md]() {
		auto writer = Writer::create(std::make_unique<std::stringstream>(), md);

		writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));

		return std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	}());

	stream = ([&stream, &md]() {
		md = TestMDWriter::dummy_md2();

		auto writer = Writer::open(std::move(stream));
		writer.set_metadata(md);

		return std::unique_ptr<std::stringstream>(static_cast<std::stringstream*>(std::move(writer).finalize().release()));
	}());

	stream->seekg(0);
	stream->seekp(0);

	auto reader = Reader::open(std::move(stream));

	std::uint64_t bar = 0;
	reader.stream().read(reinterpret_cast<char*>(&bar), sizeof(bar));

	BOOST_CHECK_EQUAL(reader.stream().gcount(), sizeof(bar));
	BOOST_CHECK_EQUAL(foo, bar);

	const auto md2 = reader.metadata();

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());
}

BOOST_AUTO_TEST_CASE(read_write_set_metadata_file)
{
	transient_directory tmp_dir{};

	const auto tmp_file = tmp_dir.path / "foo.bin";

	auto md = TestMDWriter::dummy_md();

	{
		auto writer = Writer::create(tmp_file.c_str(), md);

		writer.stream().write(reinterpret_cast<const char*>(&foo), sizeof(foo));
	}

	{
		md = TestMDWriter::dummy_md2();

		auto writer = Writer::open(tmp_file.c_str());
		writer.set_metadata(md);
	}

	auto reader = Reader::open(tmp_file.c_str());

	std::uint64_t bar = 0;
	reader.stream().read(reinterpret_cast<char*>(&bar), sizeof(bar));

	BOOST_CHECK_EQUAL(reader.stream().gcount(), sizeof(bar));
	BOOST_CHECK_EQUAL(foo, bar);

	const auto md2 = reader.metadata();

	BOOST_CHECK_EQUAL(md.type(), md2.type());
	BOOST_CHECK_EQUAL(md.format_version(), md2.format_version());
	BOOST_CHECK_EQUAL(md.tool_name(), md2.tool_name());
	BOOST_CHECK_EQUAL(md.tool_version(), md2.tool_version());
	BOOST_CHECK_EQUAL(md.tool_info(), md2.tool_info());
	BOOST_CHECK_EQUAL(md.generation_date(), md2.generation_date());
}
