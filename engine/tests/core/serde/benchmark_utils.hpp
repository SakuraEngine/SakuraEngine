#pragma once
#include <cassert>
#include <chrono>
#include <string>
#include <vector>

struct BenchMarkClock
{
    template <typename F>
    inline BenchMarkClock operator<<(F&& func)
    {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        _total_ns += std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        return *this;
    }

    inline void reset() { _total_ns = 0; }

    inline double total_ns() const { return (double)_total_ns; }
    inline double total_ms() const { return _total_ns / 1e6; }
    inline double total_s() const { return _total_ns / 1e9; }

private:
    uint64_t _total_ns = 0;
};

enum class EBenchMarkItemKind
{
    UInt,
    Int,
    Real,
};
enum class EBenchMarkCompare
{
    LessIsBetter,
    MoreIsBetter,
};

struct BenchMarkItem
{
    enum class EKind
    {
    };
    enum class ECompare
    {
        LessIsBetter,
        MoreIsBetter
    };

    // ctors
    inline BenchMarkItem(uint64_t v)
        : _kind(EBenchMarkItemKind::UInt)
        , _value_uint(v)
    {
    }
    inline BenchMarkItem(int64_t v)
        : _kind(EBenchMarkItemKind::Int)
        , _value_int(v)
    {
    }
    inline BenchMarkItem(double v)
        : _kind(EBenchMarkItemKind::Real)
        , _value_real(v)
    {
    }

    // factory
    inline static BenchMarkItem UInt(uint64_t v) { return BenchMarkItem(v); }
    inline static BenchMarkItem Int(int64_t v) { return BenchMarkItem(v); }
    inline static BenchMarkItem Real(double v) { return BenchMarkItem(v); }

    // getters
    inline EBenchMarkItemKind kind() const { return _kind; }
    inline bool is_uint() const { return _kind == EBenchMarkItemKind::UInt; }
    inline bool is_int() const { return _kind == EBenchMarkItemKind::Int; }
    inline bool is_real() const { return _kind == EBenchMarkItemKind::Real; }
    inline uint64_t as_uint() const
    {
        assert(_kind == EBenchMarkItemKind::UInt);
        return _value_uint;
    }
    inline int64_t as_int() const
    {
        assert(_kind == EBenchMarkItemKind::Int);
        return _value_int;
    }
    inline double as_real() const
    {
        assert(_kind == EBenchMarkItemKind::Real);
        return _value_real;
    }

    // equal
    inline bool operator==(const BenchMarkItem& rhs) const
    {
        if (_kind != rhs._kind) return false;
        switch (_kind)
        {
        case EBenchMarkItemKind::UInt:
            return _value_uint == rhs._value_uint;
        case EBenchMarkItemKind::Int:
            return _value_int == rhs._value_int;
        case EBenchMarkItemKind::Real:
            return _value_real == rhs._value_real;
        }
        assert(false && "unreachable");
        return false;
    }
    inline bool operator!=(const BenchMarkItem& rhs) const { return !(*this == rhs); }

    // is better
    inline bool is_better_than(const BenchMarkItem& rhs, EBenchMarkCompare compare) const
    {
        assert(_kind == rhs._kind);
        switch (compare)
        {
        case EBenchMarkCompare::LessIsBetter:
            switch (_kind)
            {
            case EBenchMarkItemKind::UInt:
                return _value_uint < rhs._value_uint;
            case EBenchMarkItemKind::Int:
                return _value_int < rhs._value_int;
            case EBenchMarkItemKind::Real:
                return _value_real < rhs._value_real;
            }
            break;
        case EBenchMarkCompare::MoreIsBetter:
            switch (_kind)
            {
            case EBenchMarkItemKind::UInt:
                return _value_uint > rhs._value_uint;
            case EBenchMarkItemKind::Int:
                return _value_int > rhs._value_int;
            case EBenchMarkItemKind::Real:
                return _value_real > rhs._value_real;
            }
            break;
        }
        assert(false && "unreachable");
        return false;
    }
    inline double ratio_to(const BenchMarkItem& baseline, EBenchMarkCompare compare) const
    {
        assert(_kind == baseline._kind);
        assert(*this == baseline || baseline.is_better_than(*this, compare));
        switch (_kind)
        {
        case EBenchMarkItemKind::UInt:
            return (double)_value_uint / (double)baseline._value_uint;
        case EBenchMarkItemKind::Int:
            return (double)_value_int / (double)baseline._value_int;
        case EBenchMarkItemKind::Real:
            return _value_real / baseline._value_real;
        };
    }

    // format
    inline std::string format(int precision = 2) const
    {
        switch (_kind)
        {
        case EBenchMarkItemKind::UInt:
            return std::format("{}", _value_uint);
        case EBenchMarkItemKind::Int:
            return std::format("{}", _value_int);
        case EBenchMarkItemKind::Real:
            return std::format("{:.{}f}", _value_real, precision);
        }
        assert(false && "unreachable");
        return "";
    }

private:
    EBenchMarkItemKind _kind;
    union
    {
        uint64_t _value_uint;
        int64_t _value_int;
        double _value_real;
    };
};

struct BenchMarkTable
{
    // setup column info
    inline void add_column(std::string name, EBenchMarkItemKind kind, EBenchMarkCompare compare)
    {
        assert(!_current_column && _suites.empty() && "cannot add item when building");
        _column_settings.push_back({ std::move(name), kind, compare });
    }
    inline void add_column_uint(std::string name, EBenchMarkCompare compare)
    {
        add_column(std::move(name), EBenchMarkItemKind::UInt, compare);
    }
    inline void add_column_int(std::string name, EBenchMarkCompare compare)
    {
        add_column(std::move(name), EBenchMarkItemKind::Int, compare);
    }
    inline void add_column_real(std::string name, EBenchMarkCompare compare)
    {
        add_column(std::move(name), EBenchMarkItemKind::Real, compare);
    }

    // build
    inline void next_suite(std::string name)
    {
        // add current column if exists
        if (_current_column)
        {
            _suites.push_back(std::move(*_current_column));
            _current_column.reset();
            _current_column_index = 0;
        }

        // setup current column
        _current_column = ColumnData();
        _current_column->name = std::move(name);
        _current_column->items.resize(_column_settings.size(), std::nullopt);
    }
    inline void item(BenchMarkItem item)
    {
        assert(_current_column && "must call next_suite first");
        assert(_current_column_index < _column_settings.size() && "all items added");
        assert(item.kind() == _column_settings[_current_column_index].kind && "item kind mismatch");

        _current_column->items[_current_column_index] = item;
        _current_column_index++;

        // all items added, push to columns
        if (_current_column_index == _column_settings.size())
        {
            _suites.push_back(std::move(*_current_column));
            _current_column.reset();
            _current_column_index = 0;
        }
    }
    inline void item_uint(uint64_t v) { item(BenchMarkItem::UInt(v)); }
    inline void item_int(int64_t v) { item(BenchMarkItem::Int(v)); }
    inline void item_real(double v) { item(BenchMarkItem::Real(v)); }

    // clear suites
    inline void clear_suites()
    {
        _suites.clear();
        _current_column.reset();
        _current_column_index = 0;
    }

    // compare and dump
    inline void dump()
    {
        // compare best item in each column
        std::vector<int> best_indices;
        best_indices.resize(_column_settings.size(), -1);
        for (size_t i = 0; i < _column_settings.size(); i++)
        {
            const auto& setting = _column_settings[i];
            auto compare = setting.compare;
            int best_index = -1;
            for (size_t j = 0; j < _suites.size(); j++)
            {
                const auto& suite = _suites[j];
                if (!suite.items[i].has_value()) continue;
                if (best_index == -1)
                {
                    best_index = (int)j;
                    continue;
                }
                auto last_best = _suites[best_index].items[i].value();
                if (suite.items[i]->is_better_than(last_best, compare))
                {
                    best_index = (int)j;
                }
            }
            best_indices[i] = best_index;
        }

        // calculate ratio to best item
        std::vector<double> ratios;
        ratios.resize(_column_settings.size() * _suites.size(), -1.0);
        for (size_t i = 0; i < _column_settings.size(); i++)
        {
            const auto& setting = _column_settings[i];
            auto compare = setting.compare;
            int best_index = best_indices[i];
            if (best_index == -1) continue;
            const auto& best_item = _suites[best_index].items[i].value();
            for (size_t j = 0; j < _suites.size(); j++)
            {
                const auto& column = _suites[j];
                if (!column.items[i].has_value()) continue;
                auto& ratio = ratios[j * _column_settings.size() + i];
                ratio = column.items[i]->ratio_to(best_item, compare);
            }
        }

        // solve column header string
        std::vector<std::string> column_header_str;
        column_header_str.resize(_column_settings.size());
        for (size_t i = 0; i < _column_settings.size(); i++)
        {
            auto& setting = _column_settings[i];
            std::string content = _column_settings[i].name;
            switch (setting.compare)
            {
            case EBenchMarkCompare::LessIsBetter:
                content += " (Less is Better)";
                break;
            case EBenchMarkCompare::MoreIsBetter:
                content += " (More is Better)";
                break;
            }
            column_header_str[i] = content;
        }

        // solve all item string
        std::vector<std::string> items_str;
        items_str.resize(_suites.size() * _column_settings.size());
        for (size_t i = 0; i < _suites.size(); i++)
        {
            const auto& column = _suites[i];
            for (size_t j = 0; j < _column_settings.size(); j++)
            {
                auto& str = items_str[i * _column_settings.size() + j];
                // append suite value
                if (column.items[j].has_value())
                {
                    str = column.items[j]->format(2);
                }
                else
                {
                    str = "-";
                }

                // append ratio if exists
                if (ratios[i * _column_settings.size() + j] > 0.0)
                {
                    std::format_to(
                        std::back_inserter(str),
                        " ({:.2f}x)",
                        ratios[i * _column_settings.size() + j]
                    );
                }
            }
        }

        // solve column width
        std::vector<size_t> column_width;
        column_width.resize(_column_settings.size());
        for (size_t i = 0; i < _column_settings.size(); i++)
        {
            size_t width = column_header_str[i].size();
            for (size_t j = 0; j < _suites.size(); j++)
            {
                width = std::max(width, items_str[j * _column_settings.size() + i].size());
            }
            column_width[i] = width;
        }

        // solve max suite name width
        size_t max_suite_name_width = 5; // "Suite"
        for (const auto& column : _suites)
        {
            max_suite_name_width = std::max(max_suite_name_width, column.name.size());
        }

        // dump, make value center and print best use bold green
        std::string dump_str;
        { // print up border
            // print left border
            dump_str += "+";

            // print suite spliter
            dump_str += std::string(max_suite_name_width + 2, '-');
            dump_str += "+";

            // print column spliter
            for (size_t i = 0; i < _column_settings.size(); i++)
            {
                dump_str += std::string(column_width[i] + 2, '-');
                dump_str += "+";
            }

            dump_str += "\n";
        }
        { // print header
            // print left border
            dump_str += "|";

            // pad 'suite' into center
            size_t left_pad = (max_suite_name_width - 5) / 2;
            size_t right_pad = (max_suite_name_width - 5) - left_pad;
            std::format_to(
                std::back_inserter(dump_str),
                " {}{}{} |",
                std::string(left_pad, ' '),
                "Suite",
                std::string(right_pad, ' ')
            );

            // print column header
            for (size_t i = 0; i < _column_settings.size(); i++)
            {
                const auto& setting = _column_settings[i];
                // pad column header into center
                size_t left_pad = (column_width[i] - column_header_str[i].size()) / 2;
                size_t right_pad = column_width[i] - column_header_str[i].size() - left_pad;
                std::format_to(
                    std::back_inserter(dump_str),
                    " {}{}{} |",
                    std::string(left_pad, ' '),
                    column_header_str[i],
                    std::string(right_pad, ' ')
                );
            }
            dump_str += "\n";
        }
        { // print spliter
            // print left border
            dump_str += "+";

            // write suite spliter
            dump_str += "-";
            dump_str += std::string(max_suite_name_width, '-');
            dump_str += "-+";

            // write column spliter
            for (size_t i = 0; i < _column_settings.size(); i++)
            {
                dump_str += "-";
                dump_str += std::string(column_width[i], '-');
                dump_str += "-+";
            }
            dump_str += "\n";
        }
        { // print columns
            for (size_t i = 0; i < _suites.size(); i++)
            {
                const auto& column = _suites[i];
                // print left border
                dump_str += "|";

                // pad suite name into left
                {
                    size_t right_pad = max_suite_name_width - column.name.size();
                    std::format_to(
                        std::back_inserter(dump_str),
                        " {}{} |",
                        column.name,
                        std::string(right_pad, ' ')
                    );
                }

                // print items
                for (size_t j = 0; j < _column_settings.size(); j++)
                {
                    const auto& setting = _column_settings[j];
                    const auto& item_str = items_str[i * _column_settings.size() + j];
                    // pad item into center
                    size_t left_pad = (column_width[j] - item_str.size()) / 2;
                    size_t right_pad = column_width[j] - item_str.size() - left_pad;
                    dump_str += " ";
                    dump_str += left_pad > 0 ? std::string(left_pad, ' ') : "";
                    if (best_indices[j] == (int)i) dump_str += "\x1b[1;32m"; // bold green
                    dump_str += item_str;
                    if (best_indices[j] == (int)i) dump_str += "\x1b[0m"; // reset
                    dump_str += right_pad > 0 ? std::string(right_pad, ' ') : "";
                    dump_str += " |";
                }
                dump_str += "\n";
            }
        }
        { // print down border
            dump_str += "+";

            // write suite spliter
            dump_str += std::string(max_suite_name_width + 2, '-');
            dump_str += "+";

            // write column spliter
            for (size_t i = 0; i < _column_settings.size(); i++)
            {
                dump_str += std::string(column_width[i] + 2, '-');
                dump_str += "+";
            }
            dump_str += "\n";
        }

        dump_str += "\n";
        printf(dump_str.c_str());
    }

private:
    struct ColumnSettings
    {
        std::string name;
        EBenchMarkItemKind kind;
        EBenchMarkCompare compare;
    };
    struct ColumnData
    {
        std::string name;
        std::vector<std::optional<BenchMarkItem>> items;
    };

    // data
    std::vector<ColumnSettings> _column_settings;
    std::vector<ColumnData> _suites;

    // builder
    std::optional<ColumnData> _current_column;
    uint64_t _current_column_index = 0;
};
