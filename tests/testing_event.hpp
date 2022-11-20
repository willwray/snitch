#include <algorithm>
#include <vector>

struct event_deep_copy {
    enum class type {
        unknown,
        test_case_started,
        test_case_ended,
        test_case_skipped,
        assertion_failed
    };

    type event_type = type::unknown;

    snatch::small_string<snatch::max_test_name_length> test_id_name;
    snatch::small_string<snatch::max_test_name_length> test_id_tags;
    snatch::small_string<snatch::max_test_name_length> test_id_type;

    snatch::small_string<snatch::max_message_length> location_file;
    std::size_t                                      location_line = 0u;

    snatch::small_string<snatch::max_message_length> message;
    snatch::small_vector<snatch::small_string<snatch::max_message_length>, snatch::max_captures>
        captures;
    snatch::
        small_vector<snatch::small_string<snatch::max_message_length>, snatch::max_nested_sections>
            sections;
};

event_deep_copy deep_copy(const snatch::event::data& e);

struct mock_framework {
    snatch::registry registry;

    snatch::impl::test_case test_case{
        .id    = {"mock_test", "[mock_tag]", "mock_type"},
        .func  = nullptr,
        .state = snatch::impl::test_state::not_run};

    std::vector<event_deep_copy> events;

    mock_framework();

    void report(const snatch::registry&, const snatch::event::data& e) noexcept;

    void run_test();

    std::optional<event_deep_copy> get_failure_event(std::size_t id = 0) const;

    std::optional<event_deep_copy> get_skip_event() const;

    std::size_t get_num_runs() const;
    std::size_t get_num_failures() const;
    std::size_t get_num_skips() const;
};

#define CHECK_EVENT_TEST_ID(ACTUAL, EXPECTED)                                                      \
    CHECK(ACTUAL.test_id_name == EXPECTED.name);                                                   \
    CHECK(ACTUAL.test_id_tags == EXPECTED.tags);                                                   \
    CHECK(ACTUAL.test_id_type == EXPECTED.type)

#define CHECK_EVENT_LOCATION(ACTUAL, FILE, LINE)                                                   \
    CHECK(ACTUAL.location_file == std::string_view(FILE));                                         \
    CHECK(ACTUAL.location_line == LINE)

#define CHECK_CAPTURES_FOR_FAILURE(FAILURE_ID, ...)                                                \
    do {                                                                                           \
        auto failure = framework.get_failure_event(FAILURE_ID);                                    \
        REQUIRE(failure.has_value());                                                              \
        const char* EXPECTED_CAPTURES[] = {__VA_ARGS__};                                           \
        REQUIRE(                                                                                   \
            failure.value().captures.size() == sizeof(EXPECTED_CAPTURES) / sizeof(const char*));   \
        std::size_t CAPTURE_INDEX = 0;                                                             \
        for (std::string_view CAPTURED_VALUE : EXPECTED_CAPTURES) {                                \
            CHECK(failure.value().captures[CAPTURE_INDEX] == CAPTURED_VALUE);                      \
            ++CAPTURE_INDEX;                                                                       \
        }                                                                                          \
    } while (0)

#define CHECK_CAPTURES(...) CHECK_CAPTURES_FOR_FAILURE(0u, __VA_ARGS__)

#define CHECK_NO_CAPTURE_FOR_FAILURE(FAILURE_ID)                                                   \
    do {                                                                                           \
        auto failure = framework.get_failure_event(FAILURE_ID);                                    \
        REQUIRE(failure.has_value());                                                              \
        CHECK(failure.value().captures.empty());                                                   \
    } while (0)

#define CHECK_NO_CAPTURE CHECK_NO_CAPTURE_FOR_FAILURE(0u)

#define CHECK_SECTIONS_FOR_FAILURE(FAILURE_ID, ...)                                                \
    do {                                                                                           \
        auto failure = framework.get_failure_event(FAILURE_ID);                                    \
        REQUIRE(failure.has_value());                                                              \
        const char* EXPECTED_SECTIONS[] = {__VA_ARGS__};                                           \
        REQUIRE(                                                                                   \
            failure.value().sections.size() == sizeof(EXPECTED_SECTIONS) / sizeof(const char*));   \
        std::size_t SECTION_INDEX = 0;                                                             \
        for (std::string_view SECTION_NAME : EXPECTED_SECTIONS) {                                  \
            CHECK(failure.value().sections[SECTION_INDEX] == SECTION_NAME);                        \
            ++SECTION_INDEX;                                                                       \
        }                                                                                          \
    } while (0)

#define CHECK_SECTIONS(...) CHECK_SECTIONS_FOR_FAILURE(0u, __VA_ARGS__)

#define CHECK_NO_SECTION_FOR_FAILURE(FAILURE_ID)                                                   \
    do {                                                                                           \
        auto failure = framework.get_failure_event(FAILURE_ID);                                    \
        REQUIRE(failure.has_value());                                                              \
        CHECK(failure.value().sections.empty());                                                   \
    } while (0)

#define CHECK_NO_SECTION CHECK_NO_SECTION_FOR_FAILURE(0u)