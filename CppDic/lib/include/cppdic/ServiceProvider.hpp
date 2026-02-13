#pragma once

#include <cppdic/Service.hpp>
#include <cppdic/Utils.hpp>

#include <memory>
#include <tuple>
#include <typeindex>

namespace dic {

class ServiceProviderRef {
public:
  ServiceProviderRef() = default;

  template <class T> std::shared_ptr<T> get() const {
    if (getRawFn == nullptr) {
      return nullptr;
    }

    auto raw = getRawFn(impls, std::type_index(typeid(T)));
    return std::static_pointer_cast<T>(raw);
  }

private:
  using GetRawFn = std::shared_ptr<void> (*)(const std::shared_ptr<void> &,
                                             std::type_index);

  ServiceProviderRef(std::shared_ptr<void> _impls, GetRawFn _getRawFn)
      : impls(std::move(_impls)), getRawFn(_getRawFn) {}

  std::shared_ptr<void> impls;
  GetRawFn getRawFn = nullptr;

  template <class... Ts> friend class ServiceProvider;
};

template <class... Ts> class [[nodiscard]] ServiceProvider final {
public:
  explicit ServiceProvider(std::shared_ptr<Services<Ts...>> _impls)
      : impls(std::move(_impls)) {}

  ServiceProviderRef ref() const {
    return ServiceProviderRef(impls, &ServiceProvider::getRaw);
  }

public:
  template <class T> std::shared_ptr<T> get() const {
    constexpr auto index = utils::getIndex<T, Ts...>(0);
    return std::get<index>(*impls);
  }

private:
  static std::shared_ptr<void> getRaw(const std::shared_ptr<void> &impls,
                                      std::type_index type) {
    auto typedImpls = std::static_pointer_cast<Services<Ts...>>(impls);
    return getRawImpl<0, Ts...>(*typedImpls, type);
  }

  template <size_t Index, class Head, class... Tail>
  static std::shared_ptr<void> getRawImpl(const Services<Ts...> &impls,
                                          std::type_index type) {
    if (type == std::type_index(typeid(typename Head::BaseType))) {
      return std::static_pointer_cast<void>(std::get<Index>(impls));
    }

    if constexpr (sizeof...(Tail) > 0) {
      return getRawImpl<Index + 1, Tail...>(impls, type);
    }

    return nullptr;
  }

  std::shared_ptr<Services<Ts...>> impls;
};
} // namespace dic