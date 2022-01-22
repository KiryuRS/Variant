#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

// TODO: define VARIANT_TYPES macro
// Any UDT must be defined before this macro
// Example:
#define VARIANT_TYPES \
  char, short, int, long, long long, const char*, std::string,\
  std::vector<int>, int*, void(*)(int, int)

namespace ki
{
  namespace
  {
    using variant_t = std::tuple<VARIANT_TYPES>;
    inline const variant_t VariantTuple;
    inline constexpr int16_t INVALID_TYPE = -1;

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
    struct IsVariantConstructible
    {
      static constexpr bool value = GetSearchedIndex<LegalT>(std::index_sequence_for<VARIANT_TYPES>{}) != INVALID_TYPE;
    };

    template <typename LegalT>
    inline constexpr bool is_variant_constructible_v = IsVariantConstructible<LegalT>::value;

    template <typename SearchT, size_t N, size_t ... Ns>
    bool IsTypeMatched(int16_t index, std::index_sequence<N, Ns...>) noexcept
    {
      if constexpr (sizeof...(Ns))
      {
        return index == N
          ? std::is_same_v<std::tuple_element_t<N, variant_t>, SearchT>
          : IsTypeMatched<SearchT>(index, std::integer_sequence<size_t, Ns...>{});
      }
      else
      {
        return index == N
          ? std::is_same_v<std::tuple_element_t<N, variant_t>, SearchT>
          : false;
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

    template <typename TypeT>
    bool is_same_type() const noexcept
    {
      return TypeId != INVALID_TYPE
        ? IsTypeMatched<TypeT>(TypeId, std::index_sequence_for<VARIANT_TYPES>{})
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
          const auto& otherElem = other.get_value<decayed_t>();
          Holder = std::make_unique<HolderImpl<decayed_t>>(otherElem);
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
      typename = std::enable_if_t<
        !std::is_same_v<DecayedT, VariantImpl> &&
        is_variant_constructible_v<DecayedT>>>
    VariantImpl(ValueT&& value)
      : Holder{std::make_unique<HolderImpl<DecayedT>>(std::forward<ValueT>(value))}
      , TypeId{GetSearchedIndex<DecayedT>(std::index_sequence_for<VARIANT_TYPES>{})}
    {
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
          const auto& otherElem = other.get_value<decayed_t>();
          Holder = std::make_unique<HolderImpl<decayed_t>>(otherElem);
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
      typename DecayedT = std::decay_t<ValueT>,
      typename = std::enable_if_t<
        !std::is_same_v<DecayedT, VariantImpl> &&
        is_variant_constructible_v<DecayedT>>>
    VariantImpl& operator=(ValueT&& value)
    {
      TypeId = GetSearchedIndex<DecayedT>(std::index_sequence_for<VARIANT_TYPES>{});
      Holder = std::make_unique<HolderImpl<DecayedT>>(std::forward<ValueT>(value));
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
      return get_value<ConvertT>();
    }

    template <typename ValueT>
    bool operator==(const ValueT& other) const noexcept
    {
      if constexpr (std::is_same_v<ValueT, VariantImpl>)
      {
        return TypeId == other.TypeId;
      }
      else
      {
        return is_same_type<ValueT>()
          ? get_value<ValueT>() == other
          : false;
      }
    }

    template <typename ValueT>
    bool operator!=(const ValueT& value) const noexcept
    {
      return !operator==(value);
    }

    int16_t get_type_id() const noexcept { return TypeId; }
  };

  using Variant = VariantImpl;
}
