#include <benchmark/benchmark.h>
#include <mapbox/vector_tile.hpp>
#include <mapbox/vector_tile/builder.hpp>

#include <limits>
#include <iostream>
#include <fstream>
#include <sstream>

static std::string open_tile(std::string const& path)
{
    std::ifstream stream(path.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!stream.is_open())
    {
        throw std::runtime_error("could not open: '" + path + "'");
    }
    std::string message(std::istreambuf_iterator<char>(stream.rdbuf()), (std::istreambuf_iterator<char>()));
    stream.close();
    return message;
}

template <typename CoordinateType>
static void BM_decode_polygon_fixture(benchmark::State& state) // NOLINT google-runtime-references
{
    std::string buffer = open_tile("test/mvt-fixtures/fixtures/valid/Feature-single-polygon.mvt");
    vtzero::vector_tile tile(buffer);
    auto layer = tile.next_layer();
    auto feature = layer.next_feature();

    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(mapbox::vector_tile::extract_geometry<CoordinateType>(feature));
    }
}

template <typename CoordinateType>
static void BM_decode_polygon(benchmark::State& state) // NOLINT google-runtime-references
{
    std::string buffer;
    std::size_t num_points = 0;
    {
        vtzero::tile_builder tbuilder;
        std::uint32_t extent = 4096;
        std::int64_t range = state.range(0);
        vtzero::layer_builder lbuilder{tbuilder, "my_layer_name", 2, extent};
        
        mapbox::geometry::polygon<CoordinateType> poly;
        poly.emplace_back();
        mapbox::geometry::linear_ring<CoordinateType> & lr = poly.back();
        
        CoordinateType x = static_cast<CoordinateType>(extent / 2) - static_cast<CoordinateType>(range / 2);
        CoordinateType y = static_cast<CoordinateType>(extent / 2) - static_cast<CoordinateType>(range / 2);
        for (std::int64_t i = 0; i < range; ++i) {
            ++num_points;
            lr.emplace_back(x, y);
            ++y;
        }
        for (std::int64_t i = 0; i < range; ++i) {
            ++num_points;
            lr.emplace_back(x, y);
            ++x;
        }
        for (std::int64_t i = 0; i < range; ++i) {
            ++num_points;
            lr.emplace_back(x, y);
            --y;
        }
        for (std::int64_t i = 0; i < range; ++i) {
            ++num_points;
            lr.emplace_back(x, y);
            --x;
        }
        ++num_points;
        lr.emplace_back(x, y);

        mapbox::geometry::geometry<CoordinateType> geom(std::move(poly));
        mapbox::feature::identifier id;
        mapbox::feature::property_map prop;
        mapbox::feature::feature<CoordinateType> feat{geom, prop, id};
        mapbox::vector_tile::encode_feature(lbuilder, feat);
        buffer = tbuilder.serialize();
    }

    vtzero::vector_tile tile(buffer);
    auto layer = tile.next_layer();
    auto feature = layer.next_feature();

    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(mapbox::vector_tile::extract_geometry<CoordinateType>(feature));
    }
    // sets a simple counter
    state.counters["Points"] = num_points;
    state.counters["PointRate"] = benchmark::Counter(num_points, benchmark::Counter::kIsRate);
}

BENCHMARK_TEMPLATE(BM_decode_polygon_fixture, int64_t);
BENCHMARK_TEMPLATE(BM_decode_polygon_fixture, int32_t);
BENCHMARK_TEMPLATE(BM_decode_polygon_fixture, int16_t);
BENCHMARK_TEMPLATE(BM_decode_polygon, int64_t)->RangeMultiplier(2)->Range(1, 1<<12);
BENCHMARK_TEMPLATE(BM_decode_polygon, int32_t)->RangeMultiplier(2)->Range(1, 1<<12);
BENCHMARK_TEMPLATE(BM_decode_polygon, int16_t)->RangeMultiplier(2)->Range(1, 1<<12);

BENCHMARK_MAIN();
