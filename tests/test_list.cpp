#include <cat/insert_iterators>
#include <cat/linear_allocator>
#include <cat/list>
#include <cat/page_allocator>

auto main() -> int {
    cat::PageAllocator page_allocator;
    cat::Byte* p_page = page_allocator.p_alloc_multi<cat::Byte>(4_ki).or_exit();
    defer(page_allocator.free_multi(p_page, 4_ki);)
    cat::LinearAllocator allocator = {p_page, 4_ki};

    // Test insert.
    cat::List<int4> list_1;
    _ = list_1.insert(allocator, list_1.begin(), 3).or_exit();
    _ = list_1.insert(allocator, list_1.begin(), 2).or_exit();
    _ = list_1.insert(allocator, list_1.begin(), 1).or_exit();
    verify(list_1.front() == 1);
    verify(list_1.back() == 3);

    // Test iteration.
    int i = 1;
    for (auto& node : list_1) {
        verify(node == i);
        ++i;
    }

    list_1.pop_front(allocator);
    verify(list_1.front() == 2);
    list_1.pop_back(allocator);
    verify(list_1.back() == 2);

    // Test push.
    cat::List<int4> list_2;
    _ = list_2.push_front(allocator, 0).or_exit();
    _ = list_2.push_back(allocator, 4).or_exit();
    verify(list_2.front() == 0);
    verify(list_2.back() == 4);
    _ = list_2.insert(allocator, ++list_2.begin(), 1).or_exit();
    verify(list_2.front() == 0);
    verify(*++list_2.begin() == 1);

    // Test iteration.
    for ([[maybe_unused]] int4 _ : list_2) {
    }

    // Test emplace.
    cat::List<int4> list_3;
    _ = list_3.emplace_front(allocator, 1).or_exit();
    _ = list_3.emplace_front(allocator, 2).or_exit();
    _ = list_3.emplace_back(allocator, 3).or_exit();
    _ = list_3.emplace(allocator, ++list_3.begin(), 4);
    verify(list_3.front() == 2);
    verify(list_3.back() == 3);
    verify((*(++list_3.begin())) == 4);

    for (auto& node : list_3) {
        // Iterate.
        node = node;  // NOLINT
    }

    // Test special iterators.
    _ = list_1.emplace(allocator, list_1.begin()++, 0);
    _ = list_1.cbegin();
    _ = list_1.cend();
    _ = list_1.rbegin();
    _ = list_1.rend();
    auto iter = list_1.crbegin();
    _ = list_1.crend();
    verify(*iter == 2);
    ++iter;
    verify(*iter == 0);

    // Test freeing nodes.
    list_1.erase(allocator, list_1.begin());
    for (int i = 0; i < 10; ++i) {
        list_1.pop_front(allocator);
    }

    list_2.clear(allocator);

    // Deep copy a `List`.
    _ = list_1.push_front(allocator, 3).or_exit();
    _ = list_1.push_front(allocator, 2).or_exit();
    _ = list_1.push_front(allocator, 1).or_exit();
    _ = list_1.push_front(allocator, 0).or_exit();
    cat::List list_5 = cat::List<int4>::cloned(allocator, list_1).or_exit();

    // Test that the copy was deep.
    list_1.clear(allocator);
    verify(*list_5.begin() == 0);
    verify(*(list_5.begin() + 1) == 1);
    verify(*(list_5.begin() + 2) == 2);
    verify(*(list_5.begin() + 3) == 3);

    // Test moving `List`.
    list_1.push_front(allocator, 2);
    list_1.push_front(allocator, 1);
    list_1.push_front(allocator, 0);
    cat::List<int4> list_4 = cat::move(list_1);
    verify(list_4.front() == 0);
    verify(*(list_4.begin() + 1) == 1);
    verify(*(list_4.begin() + 2) == 2);

    // Test initialized `List`.
    [[maybe_unused]] cat::List list_init_1 =
        cat::List<int4>::from(allocator, 1, 2, 3).or_exit();
    cat::List list_init_2 =
        cat::List<int4>::from(allocator, cat::value_list<int4, 0, 4>).or_exit();
    for (int4 i : list_init_2) {
        verify(i == 0);
    }

    // Test `ForwardList`.
    allocator.reset();
    cat::ForwardList<int4> forward_list_1;
    _ = forward_list_1.push_front(allocator, 0).or_exit();
    _ = forward_list_1.emplace_front(allocator, 1).or_exit();
    _ = forward_list_1.insert_after(allocator, forward_list_1.begin() + 1, 2)
            .or_exit();
    _ = forward_list_1.emplace_after(allocator, forward_list_1.end(), 3)
            .or_exit();

    verify(*forward_list_1.begin() == 1);
    verify(*(forward_list_1.begin() + 1) == 0);
    verify(*(forward_list_1.begin() + 2) == 2);
    verify(*(forward_list_1.begin() + 3) == 3);

    // Deep copy a `ForwardList`.
    cat::ForwardList<int4> forward_list_2;
    forward_list_2.clone(allocator, forward_list_1).or_exit();

    // Remove elements from `ForwardList`.
    forward_list_1.erase_after(allocator, forward_list_1.begin());
    verify(*(forward_list_1.begin() + 1) == 2);

    forward_list_1.pop_front(allocator);
    verify(*forward_list_1.begin() == 2);

    // Test that the copy was deep.
    verify(*forward_list_2.begin() == 1);
    verify(*(forward_list_2.begin() + 1) == 0);
    verify(*(forward_list_2.begin() + 2) == 2);
    verify(*(forward_list_2.begin() + 3) == 3);

    // Test `BackInsertIterator`.
    list_1.clear(allocator);
    cat::BackInsertIterator back_insert_iterator(list_1);
    cat::FrontInsertIterator front_insert_iterator(list_1);
    back_insert_iterator.insert(allocator, 10);
    verify(list_1.front() == 10);

    front_insert_iterator.insert(allocator, 2);
    verify(list_1.front() == 2);
    verify(list_1.back() == 10);
}
