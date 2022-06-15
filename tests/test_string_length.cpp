#include <cat/allocators>
#include <cat/string>
#include <cat/utility>

void meow() {
    char const* p_string_1 = "Hello!";
    char const* const p_string_2 = "Hello!";

    char const* const p_string_3 = "Hello!";
    char const* p_string_4 = "Hello!";

    char const* const p_string_5 = "Hello!";
    char const* const p_string_6 = "Hello!";

    char const* p_string_7 = "/tmp/temp.sock";

    ssize len_1 = cat::string_length(p_string_1);
    ssize len_2 = cat::string_length(p_string_2);
    ssize len_3 = cat::string_length(p_string_3);
    ssize len_4 = cat::string_length(p_string_4);
    ssize len_5 = cat::string_length(p_string_5);
    ssize len_6 = cat::string_length(p_string_6);
    ssize len_7 = cat::string_length(p_string_7);

    Result(len_1 == len_2).or_panic();
    Result(len_1 == 7).or_panic();
    Result(len_3 == len_4).or_panic();
    Result(len_5 == len_6).or_panic();
    Result(len_7 == 15).or_panic();

    // Test `String`s.
    cat::String string_1 = p_string_1;
    Result(string_1.size() == len_1).or_panic();
    Result(string_1.subspan(1, 4).size() == 3).or_panic();
    Result(string_1.first(4).size() == 4).or_panic();
    Result(string_1.last(3).size() == 3).or_panic();
    Result(cat::String("Hello!").size() == len_1).or_panic();

    // TODO: Remove this and put it in another string unit test.
    char chars[5] = "foo\0";
    cat::Span<char> span = {chars, 4};
    span[0] = 'a';
    auto foo = cat::unconst(span).begin();
    *foo = 'a';
    for (char& c : span) {
        c = 'a';
    }

    // TODO: Put this in another string unit test.
    cat::String find_string = "abcdefabcdefabcdefabcdefabcdefabcdef";
    ssize c = find_string.find('c').or_panic();
    Result(c == 2).or_panic();

    ssize a = find_string.find('a').or_panic();
    Result(a == 0).or_panic();

    ssize f = find_string.find('f').or_panic();
    Result(f == 5).or_panic();

    // `z` is not inside of a 32-byte chunk.
    cat::String find_string_2 =
        "abcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcdefabcd"
        "efz";
    ssize z = find_string_2.find('z').or_panic();
    Result(z == 72).or_panic();

    // Make allocator for string formatting.
    cat::PageAllocator pager;
    void* p_page = pager.p_malloc(4_ki).value();
    cat::LinearAllocator allocator{p_page, 4_ki};

    // Test `int4` conversion.
    cat::String int_string = cat::to_chars(allocator, 10);
    Result(cat::compare_strings(int_string, "10")).or_panic();
    Result(int_string.size() == 3).or_panic();

    // TODO: Make this work.
    // constexpr cat::String const_int_string = cat::to_chars(10);

    cat::exit();
}