// c string style terminating the symbols

#pragma once

#include <array>
#include <cstddef>
#include <string_view>

constexpr std::size_t MAX_SYMBOL_LENGTH = 8;
using Symbol = std::array<char, MAX_SYMBOL_LENGTH>;

Symbol make_symbol(std::string_view value);
std::string_view symbol_view(const Symbol& symbol);