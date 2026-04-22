#include "test.h"
#include "owca-script/exec_buffer.h"

using namespace OwcaScript;

class BufferTest : public SimpleTest {

};

TEST_F(BufferTest, ints)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, (std::int8_t)1);
    tmp.append({}, (std::int16_t)2);
    tmp.append({}, (std::int32_t)3);
    tmp.append({}, (std::int64_t)4);
    tmp.append({}, (bool)true);
    tmp.append({}, (bool)false);
    tmp.append({}, (std::uint8_t)5);
    tmp.append({}, (std::uint16_t)6);
    tmp.append({}, (std::uint32_t)7);
    tmp.append({}, (std::uint64_t)8);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };

    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::int8_t>(code_start, code_pos, {}), 1);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::int16_t>(code_start, code_pos, {}), 2);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::int32_t>(code_start, code_pos, {}), 3);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::int64_t>(code_start, code_pos, {}), 4);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<bool>(code_start, code_pos, {}), true);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<bool>(code_start, code_pos, {}), false);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::uint8_t>(code_start, code_pos, {}), 5);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::uint16_t>(code_start, code_pos, {}), 6);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::uint32_t>(code_start, code_pos, {}), 7);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::uint64_t>(code_start, code_pos, {}), 8);
}

TEST_F(BufferTest, floats)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, (float)1.1);
    tmp.append({}, (double)2.2);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<float>(code_start, code_pos, {}), 1.1f);
    ASSERT_EQ(Internal::ExecuteBufferReader::decode<double>(code_start, code_pos, {}), 2.2);
}

TEST_F(BufferTest, string)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };

    ASSERT_EQ(Internal::ExecuteBufferReader::decode<std::string_view>(code_start, code_pos, {}), "qwerty");
}

#ifdef DEBUG
TEST_F(BufferTest, invalid_opcode1)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, (float)1.1);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };

    ASSERT_THROW(Internal::ExecuteBufferReader::decode<Internal::ExecuteBufferReader::Op>(code_start, code_pos, code.data_kinds()), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode2)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, true);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };

    ASSERT_THROW(Internal::ExecuteBufferReader::decode<std::uint8_t>(code_start, code_pos, code.data_kinds()), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode3)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };

    ASSERT_THROW(Internal::ExecuteBufferReader::decode<Internal::ExecuteBufferReader::Op>(code_start, code_pos, code.data_kinds()), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode4)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto code_start = Internal::ExecuteBufferReader::StartOfCode{ code.code() };
    auto code_pos = Internal::ExecuteBufferReader::Position{ 0 };

    ASSERT_THROW(Internal::ExecuteBufferReader::decode<std::uint32_t>(code_start, code_pos, code.data_kinds()), std::runtime_error);    
}
#endif
