#pragma once

#include <array>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace TypedView {
/**
 *  @brief  Basis for explicit traits specializations.
 */
template <class Type, class Enable = void> struct TypeTrait {};

/**
 * @brief Specialization for integlal and floating types
 */
template <class ValueType>
struct TypeTrait<
    ValueType,
    typename std::enable_if<std::is_integral<ValueType>::value ||
                            std::is_floating_point<ValueType>::value>::type> {

  static constexpr bool eq(const ValueType &t1, const ValueType &t2) {
    return t1 == t2;
  }

  static constexpr bool lt(const ValueType &t1, const ValueType &t2) {
    return t1 < t2;
  }

  static constexpr std::size_t length(const size_t sizeInBytes) {
    if constexpr (sizeof(ValueType) > 1) {
      if (size_t rem = sizeInBytes % sizeof(ValueType); rem > 0)
        return (sizeInBytes + (sizeof(ValueType) - rem)) / sizeof(ValueType);
    }
    return sizeInBytes / sizeof(ValueType);
  }
};

/**
 * @brief A non-owning reference a Type
 */
template <typename Type, typename Traits = TypeTrait<Type>>
class BasicTypeView {
public:
  using ValueType = Type;
  using TraitsType = Traits;
  using ConstIterator = const Type *;
  using ConstReverseIterator = std::reverse_iterator<ConstIterator>;
  using ReverseIterator = ConstReverseIterator;

  constexpr BasicTypeView() : m_Data{nullptr}, m_SizeBytes{0}, m_Len{0} {}

  __attribute__((__nonnull__)) constexpr BasicTypeView(/*const*/ Type *data,
                                                       const size_t bytes)
      : m_Data{data}, m_SizeBytes{bytes}, m_Len{TraitsType::length(bytes)} {}

  constexpr ConstIterator begin() const noexcept { return this->m_Data; }

  constexpr ConstIterator end() const noexcept {
    return this->m_Data + this->m_Len;
  }

  constexpr ConstIterator cbegin() const noexcept { return this->m_Data; }

  constexpr ConstIterator cend() const noexcept {
    return this->m_Data + this->m_Len;
  }

  constexpr ConstReverseIterator rbegin() const noexcept {
    return ConstReverseIterator(this->end());
  }

  constexpr ConstReverseIterator rend() const noexcept {
    return ConstReverseIterator(this->begin());
  }

  constexpr ConstReverseIterator crbegin() const noexcept {
    return ConstReverseIterator(this->end());
  }

  constexpr ConstReverseIterator crend() const noexcept {
    return ConstReverseIterator(this->begin());
  }

  const Type &operator[](size_t offset) const noexcept {
    return *(this->m_Data + offset);
  }

  const Type &operator+(size_t offset) const noexcept {
    return *(this->m_Data + offset);
  }

  Type &operator[](size_t offset) noexcept { return *(this->m_Data + offset); }

  Type &operator+(size_t offset) noexcept { return *(this->m_Data + offset); }

  /*!
   * \brief Returns the size of the view in bytes
   * \return Size in bytes
   */
  constexpr size_t Size() const noexcept { return this->m_SizeBytes; }

  /*!
   * \brief Returns the size of the view in viewed type
   * \return Size in viewed type
   */
  constexpr size_t Length() const noexcept { return this->m_Len; }

  /*!
   * \brief Returns type size in bytes
   * \return Size in bytes
   */
  constexpr size_t TypeSize() const noexcept { return sizeof(Type); }

  /**
   * @brief Returns a view of the subarray [pos, pos + n)
   * @param pos - position of the front edge of data (data begin)
   * @param n -	requested length
   * @return Returns a view of the subarray BasicTypeView<Type, Traits>
   */
  constexpr BasicTypeView<Type, Traits> SubView(const size_t pos,
                                                const size_t n) const
      noexcept(false) {

    if (pos > m_SizeBytes)
      throw std::range_error("pos (" + std::to_string(pos) + ") > m_Len (" +
                             std::to_string(m_Len) + ")");

    const size_t len = std::min(n, m_Len - (pos * TypeSize())) * TypeSize();
    return BasicTypeView<Type, Traits>(
        reinterpret_cast<Type *>(((uint8_t *)m_Data + pos)), len);
  }

  /**
   * @brief Return copy of data as array. This function unsafe. If Sz more than
   * m_Len you'll get undefined behavior.
   * @param Sz - array size
   * @param pos - position of the front edge of data (data begin)
   * @return Copy of data as std::array<Type,Sz>
   */
  template <size_t Sz>
  constexpr std::array<Type, Sz> SubArray(const size_t pos = 0) {
    auto l = std::min(pos, m_Len > Sz ? m_Len - Sz : m_Len);
    return SubArrayImpl(
        *reinterpret_cast<Type(*)[Sz]>((std::remove_cv_t<Type> *)m_Data + l),
        std::make_index_sequence<Sz>{});
  }

private:
  template <size_t Sz, size_t... Nums>
  constexpr std::array<std::remove_cv_t<Type>, Sz>
  SubArrayImpl(Type (&pointer)[Sz], std::index_sequence<Nums...>) {
    return {{pointer[Nums]...}};
  }

  Type *const m_Data{nullptr};
  const size_t m_SizeBytes{0};
  const size_t m_Len{0};
};

template <typename... Types> class ViewReader;

/**
 * @brief View is a class for iterating over user data with a type set by the
 * user
 */
template <typename... Types> class View {
public:
  using ReaderType = ViewReader<Types...>;

  __attribute__((__nonnull__)) constexpr View(const unsigned char *data,
                                              const size_t bytes)
      : m_Views{std::make_tuple(
            BasicTypeView<Types, TypeTrait<Types>>((Types *)data, bytes)...)} {}

  /**
   * @brief Views the data as iterable range of the selected type
   * @return Return a BasicTypeView<Type> object that contains non-owning
   * pointer to the data
   */
  template <typename Type>
  constexpr BasicTypeView<Type, TypeTrait<Type>> ViewAs() const noexcept {
    return std::get<BasicTypeView<Type, TypeTrait<Type>>>(m_Views);
  }

private:
  std::tuple<BasicTypeView<Types, TypeTrait<Types>>...> m_Views;
};

template <typename... Types> class ViewReader {
public:
  using ViewType = View<Types...>;

  __attribute__((__nonnull__))
  ViewReader(const unsigned char *data, const size_t bytes)
      : m_view(data, bytes) {}

  ViewReader(const View<Types...> &view) : m_view(view) {}

  template <typename T> auto ReadAs() noexcept(false) {
    size_t byteOffsetCache = m_byteOffset;
    auto v = m_view.template ViewAs<T>();

    if (v.Size() < byteOffsetCache + v.TypeSize())
      throw std::range_error("offset (" +
                             std::to_string(byteOffsetCache + sizeof(T)) +
                             ") is out of range!");

    m_byteOffset += v.TypeSize();
    return (v + (byteOffsetCache / v.TypeSize()));
  }

  /*!
   * \brief Returns View class instance
   * \return Link to View instance
   */
  auto &GetView() noexcept { return m_view; }

  /*!
   * \brief Returns BasicTypeView class instance based on type T
   * \return BasicTypeView instance
   */
  template <class T> auto GetViewAs() noexcept {
    return m_view.template ViewAs<T>();
  }

  /*!
   * \brief Returns current offset in bytes
   * \return Offset in bytes
   */
  size_t GetOffset() noexcept { return m_byteOffset; }

  /*!
   * \brief Return size of view depends of it type in bytes
   * \return Size in bytes
   */
  template <class T> size_t GetSizeByType() noexcept {
    return GetViewAs<T>().Size();
  }

  /*!
   * \brief Returns current offset in bytes
   * \return Offset in bytes
   */
  void SetOffset(const size_t offset) noexcept { m_byteOffset = offset; }

  template <typename T> auto MakeSubView(const size_t n) noexcept {
    return MakeSubView<T>(m_byteOffset, n);
  }

  template <typename T>
  auto MakeSubViewBySize(const size_t n, bool increaseOffset = true) noexcept {
    increaseOffset ? m_byteOffset += n * sizeof(T) : m_byteOffset += 0;
    auto tView = m_view.template ViewAs<T>();
    return tView.SubView(m_byteOffset, n);
  }

  template <typename T>
  auto MakeSubView(const size_t pos, const size_t n,
                   bool increaseOffset = true) noexcept {
    increaseOffset ? m_byteOffset += n * sizeof(T) : m_byteOffset += 0;
    return m_view.template ViewAs<T>().SubView(pos, n);
  }

private:
  size_t m_byteOffset{0};
  View<Types...> m_view{};
};

} // namespace TypedView
