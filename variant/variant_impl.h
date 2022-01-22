#pragma once

#include <memory>
#include <tuple>
#include <string_view>
#include <typeindex>
#include <vector>

// TODO: define VARIANT_TYPES macro
// Any UDT must be defined before this macro
// Example:
#define VARIANT_TYPES \
  char, short, int, long, long long, std::string,\
  std::vector<int>, int*, void(*)(int, int)

namespace ki
{
  namespace
  {
    using variant_t = std::tuple<VARIANT_TYPES>;
    inline const variant_t VariantTuple;
    inline const int16_t INVALID_TYPE = -1;

    template <typename SearchT, size_t N, size_t ... Ns>
    constexpr int16_t GetSearchedIndex(std::index_sequence<N, Ns...>) noexcept
    {
      auto isIndex = std::is_same_v<SearchT, std::tuple_element_t<N, variant_t>>;
      if constexpr (sizeof...(Ns))
      {
        return isIndex ? N : GetSearchedIndex<SearchT>(std::integer_sequence<size_t, Ns...>{});
      }
      else
      {
        return isIndex ? N : INVALID_TYPE;
      }
    }

    template <typename LegalT>
    constexpr bool IsVariantConstructible() noexcept
    {
      if constexpr (std::is_same_v<char*, LegalT> || std::is_same_v<const char*, LegalT>)
      {
        // TODO: Either std::string or desired string classes
        return GetSearchedIndex<std::string>(std::index_sequence_for<VARIANT_TYPES>{}) != INVALID_TYPE;
      }
      else
      {
        return GetSearchedIndex<LegalT>(std::index_sequence_for<VARIANT_TYPES>{}) != INVALID_TYPE;
      }
    }

    template <size_t N, size_t ... Ns>
    std::type_index GetTypeIndex(int16_t index, std::index_sequence<N, Ns...>) noexcept
    {
      if constexpr (sizeof...(Ns))
      {
        return index == N
          ? typeid(std::tuple_element_t<N, variant_t>)
          : GetTypeIndex(index, std::integer_sequence<size_t, Ns...>{});
      }
      else
      {
        return index == N
          ? typeid(std::tuple_element_t<N, variant_t>)
          : typeid(void);
      }
    }

    template <typename FunctorT, size_t N, size_t ... Ns>
    auto Visit(int16_t index, FunctorT functor, std::index_sequence<N, Ns...>)
    {
      if constexpr (sizeof...(Ns))
      {
        return index == N
          ? functor(std::get<N>(VariantTuple))
          : Visit(index, functor, std::integer_sequence<size_t, Ns...>{});
      }
      else
      {
        return functor(std::get<N>(VariantTuple));
      }
    }
  }

  struct VariantUnsupportedType : std::invalid_argument
  {
    VariantUnsupportedType() noexcept = default;

    template <typename StringT, typename = std::enable_if_t<!std::is_base_of_v<std::invalid_argument, StringT>>>
    VariantUnsupportedType(StringT&& string) noexcept
      : std::invalid_argument{std::forward<StringT>(string)}
    {
    }

    virtual const char* what() const noexcept override { return std::invalid_argument::what(); }
  };

  class VariantImpl final
  {
    struct AbstractHolder
    {
      virtual ~AbstractHolder() noexcept = default;
    };

    template <typename DataT>
    struct HolderImpl final : public AbstractHolder
    {
      DataT Data;

      template <typename ValueT, typename = std::enable_if_t<!std::is_base_of_v<AbstractHolder, ValueT>>>
      HolderImpl(ValueT&& value)
        : Data{std::forward<ValueT>(value)}
      {
      }
    };

    template <typename T>
    bool is_same_type(const T& other) const noexcept
    {
      return TypeId != INVALID_TYPE
        ? GetTypeIndex(TypeId, std::index_sequence_for<VARIANT_TYPES>{}) == typeid(other)
        : false;
    }

    std::unique_ptr<AbstractHolder> Holder;
    int16_t TypeId = INVALID_TYPE;

  public:
    VariantImpl() noexcept = default;

    VariantImpl(const VariantImpl& other)
      : Holder{}
      , TypeId{other.TypeId}
    {
      if (!other)
        return;

      auto functor =
        [this, &other](auto&& elem)
        {
          using decayed_t = std::decay_t<decltype(elem)>;
          auto& otherElem = other.get_value<decayed_t>();
          Holder.reset(new HolderImpl<decayed_t>{otherElem});
        };
      Visit(TypeId, functor, std::index_sequence_for<VARIANT_TYPES>{});
    }

    VariantImpl(VariantImpl&& other) noexcept
      : Holder{std::move(other.Holder)}
      , TypeId{std::move(other.TypeId)}
    {
      other.Holder = nullptr;
      other.TypeId = INVALID_TYPE;
    }

    template <
      typename ValueT,
      typename DecayedT = std::decay_t<ValueT>,
      typename = std::enable_if_t<!std::is_same_v<DecayedT, VariantImpl>>>
    VariantImpl(ValueT&& value)
      : Holder{}
      , TypeId{INVALID_TYPE}
    {
      if constexpr (IsVariantConstructible<DecayedT>())
      {
        if constexpr (std::is_same_v<char*, DecayedT> || std::is_same_v<const char*, DecayedT>)
        {
          TypeId = GetSearchedIndex<std::string>(std::index_sequence_for<VARIANT_TYPES>{});
          Holder.reset(new HolderImpl<std::string>(std::forward<ValueT>(value)));
        }
        else
        {
          TypeId = GetSearchedIndex<DecayedT>(std::index_sequence_for<VARIANT_TYPES>{});
          Holder.reset(new HolderImpl<DecayedT>(std::forward<ValueT>(value)));
        }
      }
      else
      {
        throw VariantUnsupportedType("Type is not supported");
      }
    }

    VariantImpl& operator=(const VariantImpl& other)
    {
      if (!other)
      {
        TypeId = INVALID_TYPE;
        Holder = nullptr;
        return *this;
      }

      TypeId = other.TypeId;
      auto functor =
        [this, &other](auto&& elem)
        {
          using decayed_t = std::decay_t<decltype(elem)>;
          auto& otherElem = other.get_value<decayed_t>();
          Holder.reset(new HolderImpl<decayed_t>(otherElem));
        };
      Visit(TypeId, functor, std::index_sequence_for<VARIANT_TYPES>{});

      return *this;
    }

    VariantImpl& operator=(VariantImpl&& other) noexcept
    {
      std::swap(Holder, other.Holder);
      std::swap(TypeId, other.TypeId);
      return *this;
    }

    template <
      typename ValueT,
      typename DecayT = std::decay_t<ValueT>,
      typename = std::enable_if_t<!std::is_same_v<DecayT, VariantImpl>>>
    VariantImpl& operator=(ValueT&& value)
    {
      if constexpr (IsVariantConstructible<DecayT>())
      {
        if constexpr (std::is_same_v<char*, DecayT> || std::is_same_v<const char*, DecayT>)
        {
          TypeId = GetSearchedIndex<std::string>(std::index_sequence_for<VARIANT_TYPES>{});
          Holder.reset(new HolderImpl<std::string>(std::forward<ValueT>(value)));
        }
        else
        {
          TypeId = GetSearchedIndex<DecayT>(std::index_sequence_for<VARIANT_TYPES>{});
          Holder.reset(new HolderImpl<DecayT>(std::forward<ValueT>(value)));
        }
      }
      else
      {
        throw VariantUnsupportedType("Type is not supported");
      }
      return *this;
    }

    template <typename ValueT, typename DecayedT = std::decay_t<ValueT>>
    const DecayedT& get_value() const
    {
      return static_cast<HolderImpl<DecayedT>*>(Holder.get())->Data;
    }

    template <typename ValueT, typename DecayedT = std::decay_t<ValueT>>
    DecayedT& get_value()
    {
      return static_cast<HolderImpl<DecayedT>*>(Holder.get())->Data;
    }

    operator bool() const noexcept
    {
      return TypeId != INVALID_TYPE;
    }

    template <typename ConvertT>
    operator ConvertT() const
    {
      using decayed_t = std::decay_t<ConvertT>;
      return get_value<decayed_t>();
    }

    template <
      typename ValueT,
      typename DecayT = std::decay_t<ValueT>,
      typename = std::enable_if_t<!std::is_same_v<DecayT, VariantImpl>>>
    bool operator==(const ValueT& value) const noexcept
    {
      return is_same_type(value)
        ? get_value<ValueT>() == value
        : false;
    }

    template <typename ValueT>
    bool operator!=(const ValueT& value) const noexcept
    {
      return !operator==(value);
    }

    bool operator==(const VariantImpl& other) const noexcept
    {
      return TypeId == other.TypeId;
    }

    bool operator!=(const VariantImpl& other) const noexcept
    {
      return !operator==(other);
    }

    int16_t get_type_id() const noexcept { return TypeId; }
  };

  using Variant = VariantImpl;
}
