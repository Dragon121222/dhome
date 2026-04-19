#ifndef __DHomeMixinClass__
#define __DHomeMixinClass__

namespace dhome::util {

// --- tag_of ---
template <typename T>
using tag_of = typename T::typeTag;

// --- all_tagged: filter pack into tuple by tag ---
template <typename Tag, typename Accumulator, typename... Ts>
struct all_tagged_impl;

// Base: nothing left, return accumulator
template <typename Tag, typename... Accumulated>
struct all_tagged_impl<Tag, std::tuple<Accumulated...>> {
    using type = std::tuple<Accumulated...>;
};

// Match: tag matches, append T
template <typename Tag, typename... Accumulated, typename T, typename... Rest>
struct all_tagged_impl<Tag, std::tuple<Accumulated...>, T, Rest...>
    : std::conditional_t<
    std::is_same_v<tag_of<T>, Tag>,
    all_tagged_impl<Tag, std::tuple<Accumulated..., T>, Rest...>,
    all_tagged_impl<Tag, std::tuple<Accumulated...>,    Rest...>
    > {};

template <typename Tag, typename... Ts>
using all_tagged = typename all_tagged_impl<Tag, std::tuple<>, Ts...>::type;

template <typename... Ts>
struct mixin : Ts... {
public:
    template <typename Tag, std::size_t I=0>
    using as = std::tuple_element_t<I, all_tagged<Tag, Ts...>>::type;

    mixin() {}
    ~mixin() {}
};

}

#endif