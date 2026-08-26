#include "symbol.hpp"

#include <stdexcept>

Symbol make_symbol(std::string_view value){
    if(value.empty()){  throw std::invalid_argument("Symbol cannot be empty");  }
    if(value.size() >= MAX_SYMBOL_LENGTH){  throw std::invalid_argument("Symbol is too long");  }

    Symbol symbol{};
    for(std::size_t i=0; i<value.size(); i++){
        symbol[i] = value[i];
    }
    return symbol;
}

std::string_view symbol_view(const Symbol& symbol){
    std::size_t length = 0;
    while(length<symbol.size() && symbol[length]!='\0'){    ++length;    }
    return std::string_view(symbol.data(), length);
    // symbol is type alias of array and it provides .data() method
}