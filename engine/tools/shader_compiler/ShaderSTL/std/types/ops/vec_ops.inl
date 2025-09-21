// clang-format off
using ElementType = T;

[[nodiscard, access]] constexpr T access_(uint32 idx) const noexcept {
    return _v[idx];
}

[[nodiscard, access]] constexpr T operator[](uint32 idx) const noexcept {
    return _v[idx];
}
[[nodiscard, access]] constexpr T &access_(uint32 idx) noexcept {
    return _v[idx];
}

[[nodiscard, access]] constexpr T &operator[](uint32 idx) noexcept {
    return _v[idx];
}

template<uint32 i>
constexpr void set() {}

template<uint32 i, concepts::arithmetic_scalar U, typename...Args>
constexpr void set(U v, Args...args) { _v[i] = v; set<i + 1>(args...); }

template<uint32 i, concepts::arithmetic_vec U, typename...Args>
constexpr void set(U v, Args...args) {
    constexpr auto dim = vec_dim_v<U>;
    if constexpr (dim == 2)
    {
        _v[i] = v[0]; _v[i + 1] = v[1];
    }
    else if constexpr (dim == 3)
    {
        _v[i] = v[0]; _v[i + 1] = v[1]; _v[i + 2] = v[2];
    }
    else if constexpr (dim == 4)
    {
        _v[i] = v[0]; _v[i + 1] = v[1]; _v[i + 2] = v[2]; _v[i + 3] = v[3];
    }
    set<i + dim>(args...);
}

template<typename X>
static constexpr bool is_same_dim_vec_or_scalar_v = (is_scalar_v<X> || same_dim_v<ThisType, X>);

template<typename X>
static constexpr bool same_family = is_same_dim_vec_or_scalar_v<T> && 
(
    cppsl::is_same_v<X, ThisType> || 
    cppsl::is_same_v<X, ElementType> ||
    (is_int_family_v<X> && is_int_family_v<ThisType>) // mix sint & uint
);

template<typename X>
static constexpr bool arithmetical = is_same_dim_vec_or_scalar_v<X> && is_arithmetic_v<X>;

template<class E, uint32_t N> using NoDoubleVec = vec<cppsl::conditional_t<cppsl::is_same_v<E, double>, float, E>, N>;
template<class U> using MulT = NoDoubleVec<decltype(cppsl::declval<ElementType>() * cppsl::declval<scalar_type<U>>()), dim>;
template<class U> using AddT = NoDoubleVec<decltype(cppsl::declval<ElementType>() + cppsl::declval<scalar_type<U>>()), dim>;
template<class U> using SubT = NoDoubleVec<decltype(cppsl::declval<ElementType>() - cppsl::declval<scalar_type<U>>()), dim>;
template<class U> using DivT = NoDoubleVec<decltype(cppsl::declval<ElementType>() / cppsl::declval<scalar_type<U>>()), dim>;
template<class U> using ModT = NoDoubleVec<decltype(cppsl::declval<ElementType>() % cppsl::declval<scalar_type<U>>()), dim>;

[[unaop("PLUS")]] ThisType operator+() const;
[[unaop("MINUS")]] ThisType operator-() const;

template <typename U> requires(cppsl::is_same_v<U, matrix<dim>>)
[[binop("MUL")]] MulT<U> operator*(const U&) const;
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("ADD")]] AddT<U> operator+(const U&) const;
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("SUB")]] SubT<U> operator-(const U&) const;
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("MUL")]] MulT<U> operator*(const U&) const;
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("DIV")]] DivT<U> operator/(const U&) const;
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("MOD")]] ModT<U> operator%(const U&) const requires(is_int_family_v<ThisType>);

template <typename U> requires(same_family<U>)
[[binop("BIT_AND")]] ThisType operator&(const U&) const requires(is_int_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("BIT_OR")]] ThisType operator|(const U&) const requires(is_int_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("BIT_XOR")]] ThisType operator^(const U&) const requires(is_int_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("SHL")]] ThisType operator<<(const U&) const requires(is_int_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("SHR")]] ThisType operator>>(const U&) const requires(is_int_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("AND")]] ThisType operator&&(const U&) const requires(is_bool_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("OR")]] ThisType operator||(const U&) const requires(is_bool_family_v<ThisType>);

template <typename U> requires(same_family<U>)
[[binop("LESS")]] vec<bool, dim> operator<(const U&) const requires(!is_bool_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("GREATER")]] vec<bool, dim> operator>(const U&) const requires(!is_bool_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("LESS_EQUAL")]] vec<bool, dim> operator<=(const U&) const requires(!is_bool_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("GREATER_EQUAL")]] vec<bool, dim> operator>=(const U&) const requires(!is_bool_family_v<ThisType>);
template <typename U> requires(same_family<U>)
[[binop("EQUAL")]] vec<bool, dim> operator==(const U&) const;
template <typename U> requires(same_family<U>)
[[binop("NOT_EQUAL")]] vec<bool, dim> operator!=(const U&) const;

template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("ADD_ASSIGN")]] ThisType operator+=(const U& rhs) { return *this = *this + rhs; }
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("SUB_ASSIGN")]] ThisType operator-=(const U& rhs) { return *this = *this - rhs; }
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("MUL_ASSIGN")]] ThisType operator*=(const U& rhs) { return *this = *this * rhs; }
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("DIV_ASSIGN")]] ThisType operator/=(const U& rhs) { return *this = *this / rhs; }
template <typename U> requires(same_family<U> || arithmetical<U>)
[[binop("MOD_ASSIGN")]] ThisType operator%=(const U& rhs) requires(is_int_family_v<ThisType>) { return *this = (*this % rhs); }

template <typename U> requires(same_family<U>)
[[binop("BIT_AND_ASSIGN")]] ThisType operator&=(const U& rhs) requires(is_int_family_v<ThisType>)  { return *this = (*this & rhs); }
template <typename U> requires(same_family<U>)
[[binop("BIT_OR_ASSIGN")]] ThisType operator|=(const U& rhs) requires(is_int_family_v<ThisType>) { return *this = (*this | rhs); }
template <typename U> requires(same_family<U>)
[[binop("BIT_XOR_ASSIGN")]] ThisType operator^=(const U& rhs) requires(is_int_family_v<ThisType>) { return *this = (*this ^ rhs); }
template <typename U> requires(same_family<U>)
[[binop("SHL_ASSIGN")]] ThisType operator<<=(const U& rhs) requires(is_int_family_v<ThisType>) { return *this = (*this << rhs); }
template <typename U> requires(same_family<U>)
[[binop("SHR_ASSIGN")]] ThisType operator>>=(const U& rhs) requires(is_int_family_v<ThisType>) { return *this = (*this >> rhs); }
// clang-format on