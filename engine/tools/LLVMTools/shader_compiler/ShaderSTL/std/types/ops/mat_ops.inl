// clang-format off

[[nodiscard, access]] constexpr auto access_(uint32 idx) const noexcept { return _v[idx]; }
[[nodiscard, access]] constexpr auto operator[](uint32 idx) const noexcept { return _v[idx]; }
[[nodiscard, access]] constexpr auto& operator[](uint32 idx) noexcept { return _v[idx]; }
[[nodiscard, noignore]] constexpr T get(uint32 row, uint32 col) const noexcept { return access_(row).access_(col); }

[[unaop("PLUS")]] ThisType operator+() const;
[[unaop("MINUS")]] ThisType operator-() const;

template <typename U> requires(cppsl::is_same_v<U, ThisType>)
[[binop("MUL")]] ThisType operator*(const U&) const;

template <typename U> requires(cppsl::is_same_v<U, vec<T, X>>)
[[binop("MUL")]] vec<T, X> operator*(const U&) const;

template <uint64_t A, uint64_t B>
matrix(const matrix<A, B>& rhs);

// clang-format on