#pragma once

#include <utility>
#include <type_traits>

template <typename Fn>
struct on_scope_exit {
    static_assert(std::is_nothrow_move_constructible<Fn>::value,
                    "Fn must be nothrow move constructible");

    explicit on_scope_exit(Fn &&fn) noexcept
        : _fn(std::move(fn)) {};
    on_scope_exit(const on_scope_exit&) = delete;
    on_scope_exit& operator=(const on_scope_exit&) = delete;
    ~on_scope_exit() noexcept { this->_fn(); }

private:
    Fn _fn;
};
