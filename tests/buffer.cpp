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
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_EQ(reader.decode<std::int8_t>(), 1);
    ASSERT_EQ(reader.decode<std::int16_t>(), 2);
    ASSERT_EQ(reader.decode<std::int32_t>(), 3);
    ASSERT_EQ(reader.decode<std::int64_t>(), 4);
    ASSERT_EQ(reader.decode<bool>(), true);
    ASSERT_EQ(reader.decode<bool>(), false);
    ASSERT_EQ(reader.decode<std::uint8_t>(), 5);
    ASSERT_EQ(reader.decode<std::uint16_t>(), 6);
    ASSERT_EQ(reader.decode<std::uint32_t>(), 7);
    ASSERT_EQ(reader.decode<std::uint64_t>(), 8);
}

TEST_F(BufferTest, floats)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, (float)1.1);
    tmp.append({}, (double)2.2);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_EQ(reader.decode<float>(), 1.1f);
    ASSERT_EQ(reader.decode<double>(), 2.2);
}

TEST_F(BufferTest, string)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_EQ(reader.decode<std::string_view>(), "qwerty");
}

TEST_F(BufferTest, vector)
{
    Internal::ExecuteBufferWriter tmp;
    std::vector<int> tmp_vec = {1, 2, 3, 4, 5};
    tmp.append_span_helper({}, tmp_vec);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    std::vector<int> res;
    reader.decode_span_helper<int>([&](size_t, size_t, int v) {
        res.push_back(v);
    });
    ASSERT_EQ(res, tmp_vec);
}

TEST_F(BufferTest, invalid_opcode1)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, (float)1.1);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_THROW(reader.decode<Internal::ExecuteBufferReader::Op>(), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode2)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, true);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_THROW(reader.decode<std::uint8_t>(), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode3)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_THROW(reader.decode<Internal::ExecuteBufferReader::Op>(), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode4)
{
    Internal::ExecuteBufferWriter tmp;

    tmp.append({}, "qwerty");

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_THROW(reader.decode<std::uint32_t>(), std::runtime_error);    
}

TEST_F(BufferTest, invalid_opcode5)
{
    Internal::ExecuteBufferWriter tmp;
    std::vector<int> tmp_vec = {1, 2, 3, 4, 5};
    tmp.append_span_helper({}, tmp_vec);

    auto [ buf, deb, lines ] = std::move(tmp).take();
    OwcaCode code{ "filename", std::move(buf), std::move(deb), std::move(lines), nullptr, {} };
    auto reader = Internal::ExecuteBufferReader{ code, 0 };

    ASSERT_THROW(reader.decode<Internal::ExecuteBufferReader::Op>(), std::runtime_error);    
}

