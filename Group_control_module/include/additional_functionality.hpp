template<typename T, unsigned int N>
constexpr unsigned int array_size(T(&)[N]) noexcept {
    return N;
};