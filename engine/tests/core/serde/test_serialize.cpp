#include <SkrTestFramework/framework.hpp>
#include "./test_serde_types.hpp"
#include <SkrCore/serialize/json_archive.hpp>
#include <SkrCore/serialize/binary_archive.hpp>
#include <SkrCore/exec_static.hpp>
#include "./benchmark_utils.hpp"

TEST_CASE("serialize bench mark")
{
    using namespace skr;
    using namespace std::chrono_literals;

    // SKR_LOG_INFO(u8"Test start");

    static constexpr uint64_t ITERATION = 1;
    static constexpr uint64_t DATA_AMOUNT = 10;
    auto _calc_throughput = +[](uint64_t per_iter_data, double total_ns) {
        return (double)per_iter_data * (double)ITERATION / (double)total_ns * 1e9 / (1024.0 * 1024.0);
    };

    std::this_thread::sleep_for(2s);

    SUBCASE("binary serialize")
    {
        // prepare bench mark data
        BenchMarkTable bench_tbl;
        bench_tbl.add_column_real("Time (ms)", EBenchMarkCompare::LessIsBetter);
        bench_tbl.add_column_real("Throughput (mb/s)", EBenchMarkCompare::MoreIsBetter);
        bench_tbl.add_column_real("FileSize (mb)", EBenchMarkCompare::LessIsBetter);
        BenchMarkClock clock;

        // prepare data
        test_serde::TestComplexType test_data;
        test_data.fill_some_data(DATA_AMOUNT);

        // collect buffer count
        uint64_t serialize_total_size = 0;
        {
            ArWriteBin writer;
            writer.value<test_serde::TestComplexType>(test_data);
            serialize_total_size = writer.buffer().size();

            test_serde::TestComplexType read_back;
            ArReadBin reader;
            reader.use_buffer(writer.buffer());
            // reader.buffer.resize_unsafe(reader.buffer.size() / 2);
            reader.value<test_serde::TestComplexType>(read_back);

            if (reader.error_tracker().any_error())
            {
                SKR_LOG_FMT_ERROR(u8"Read Failed: {}", reader.error_tracker().checkpoint_trace_message());
            }

            REQUIRE(test_data == read_back);
        }

        // write
        {
            // bench mark uniform bin write
            clock.reset();
            {
                ArWriteBin writer;
                writer.buffer().reserve(serialize_total_size);

                clock << [&]() {
                    SkrZoneScopedN("ArWriteBin");
                    for (uint64_t i = 0; i < ITERATION; i++)
                    {
                        writer.value<test_serde::TestComplexType>(test_data);
                        REQUIRE(writer.is_succeeded());
                        writer.buffer().clear();
                    }
                };

                bench_tbl.next_suite("ArWriteBin");
                bench_tbl.item_real(clock.total_ms());
                bench_tbl.item_real(_calc_throughput(serialize_total_size, clock.total_ns()));
                bench_tbl.item_real((double)serialize_total_size / (1024.0 * 1024.0));
            }

            // dump bench mark result
            bench_tbl.dump();
        }

        bench_tbl.clear_suites();

        // read
        {
            // bench mark uniform bin read
            clock.reset();
            {
                ArWriteBin write_archive;
                write_archive.value<test_serde::TestComplexType>(test_data);

                test_serde::TestComplexType read_back;
                ArReadBin read_archive;
                read_archive.use_buffer(write_archive.buffer());

                // read back once to avoid first time cost
                read_archive.value<test_serde::TestComplexType>(read_back);
                REQUIRE(read_archive.is_succeeded());
                read_archive.reset();

                clock << [&]() {
                    SkrZoneScopedN("ArReadBin");
                    for (uint64_t i = 0; i < ITERATION; i++)
                    {
                        read_archive.value<test_serde::TestComplexType>(read_back);
                        REQUIRE(read_archive.is_succeeded());
                        read_archive.reset();
                    }
                };

                // assert same
                REQUIRE(test_data == read_back);

                bench_tbl.next_suite("ArReadBin");
                bench_tbl.item_real(clock.total_ms());
                bench_tbl.item_real(_calc_throughput(serialize_total_size, clock.total_ns()));
                bench_tbl.item_real((double)serialize_total_size / (1024.0 * 1024.0));
            }

            // dump bench mark result
            bench_tbl.dump();
        }
    }

    SUBCASE("json serialize")
    {
        // prepare bench mark data
        BenchMarkTable bench_tbl;
        bench_tbl.add_column_real("Time (ms)", EBenchMarkCompare::LessIsBetter);
        bench_tbl.add_column_real("Throughput (mb/s)", EBenchMarkCompare::MoreIsBetter);
        bench_tbl.add_column_real("FileSize (mb)", EBenchMarkCompare::LessIsBetter);
        BenchMarkClock clock;

        // prepare data
        test_serde::TestComplexType test_data;
        test_data.fill_some_data(DATA_AMOUNT);

        // test archive correctness
        uint64_t serialize_total_size = 0;
        uint64_t inplace_total_size = 0;
        { // use dom write
            auto writer = skr::ArWriteJson::Create();
            writer.value<test_serde::TestComplexType>(test_data);
            writer.assume_succeeded_or_dump_error();
            REQUIRE(writer.is_succeeded());
            
            String json_str;
            writer.write_to_string(json_str);
            serialize_total_size = json_str.size();

            // read back
            test_serde::TestComplexType read_back;
            auto reader = skr::ArReadJson::ReadBuffer(json_str.data(), json_str.size());
            reader.value<test_serde::TestComplexType>(read_back);
            reader.assume_succeeded_or_dump_error();
            REQUIRE(reader.is_succeeded());

            REQUIRE(test_data == read_back);
        }
        { // use fast write
            skr::ArWriteJsonInplace writer;
            writer.value<test_serde::TestComplexType>(test_data);
            writer.assume_succeeded_or_dump_error();
            REQUIRE(writer.is_succeeded());

            String& json_str = writer.result;
            inplace_total_size = json_str.size();
            // SKR_LOG_FMT_INFO(u8"json str: {}", json_str);

            // read back
            test_serde::TestComplexType read_back;
            auto reader = skr::ArReadJson::ReadBuffer(json_str.data(), json_str.size());
            reader.value<test_serde::TestComplexType>(read_back);
            reader.assume_succeeded_or_dump_error();
            REQUIRE(reader.is_succeeded());

            REQUIRE(test_data == read_back);
        }

        // write
        {
            // bench mark for uniform json write
            clock.reset();
            {
                clock << [&]() {
                    SkrZoneScopedN("ArWriteJson");
                    for (uint64_t i = 0; i < ITERATION; i++)
                    {
                        auto writer = skr::ArWriteJson::Create();
                        writer.value<test_serde::TestComplexType>(test_data);
                    }
                };

                bench_tbl.next_suite("ArWriteJson");
                bench_tbl.item_real(clock.total_ms());
                bench_tbl.item_real(_calc_throughput(serialize_total_size, clock.total_ns()));
                bench_tbl.item_real((double)serialize_total_size / (1024.0 * 1024.0));
            }

            // bench mark for fast json write
            clock.reset();
            {
                clock << [&]() {
                    SkrZoneScopedN("ArWriteJsonInplace");
                    for (uint64_t i = 0; i < ITERATION; i++)
                    {
                        skr::ArWriteJsonInplace writer;
                        writer.result.reserve(inplace_total_size);
                        writer.value<test_serde::TestComplexType>(test_data);
                        String& json_str = writer.result;
                    }
                };

                bench_tbl.next_suite("ArWriteJsonInplace");
                bench_tbl.item_real(clock.total_ms());
                bench_tbl.item_real(_calc_throughput(serialize_total_size, clock.total_ns()));
                bench_tbl.item_real((double)serialize_total_size / (1024.0 * 1024.0));
            }

            // dump bench mark result
            bench_tbl.dump();
        }

        bench_tbl.clear_suites();

        // read
        {
            // bench mark for uniform json read
            clock.reset();
            {
                // dump reference string
                String json_str;
                test_serde::TestComplexType read_back;
                {
                    auto writer = skr::ArWriteJson::Create();
                    writer.value<test_serde::TestComplexType>(test_data);
                    writer.write_to_string(json_str);
                }

                // bench mark
                clock << [&]() {
                    SkrZoneScopedN("ArReadJson");
                    for (uint64_t i = 0; i < ITERATION; i++)
                    {
                        auto reader = skr::ArReadJson::ReadBuffer(json_str.data(), json_str.size());
                        reader.value<test_serde::TestComplexType>(read_back);
                    }
                };

                REQUIRE(test_data == read_back);

                bench_tbl.next_suite("ArReadJson");
                bench_tbl.item_real(clock.total_ms());
                bench_tbl.item_real(_calc_throughput(serialize_total_size, clock.total_ns()));
                bench_tbl.item_real((double)serialize_total_size / (1024.0 * 1024.0));
            }

            // dump bench mark result
            bench_tbl.dump();
        }
    }

    // std::this_thread::sleep_for(1s);
}