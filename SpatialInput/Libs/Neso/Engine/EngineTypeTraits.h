////////////////////////////////////////////////////////////////////////////////
// Copyright (C) Microsoft Corporation.  All Rights Reserved
// Licensed under the MIT License. See License.txt in the project root for license information.
#pragma once

////////////////////////////////////////////////////////////////////////////////
// TypeTraits used throughout Neso for compile-time validation
namespace Neso {

    class Engine;
    class SystemBase;
    struct ComponentBase;
    class Entity;

    namespace detail
    {
        ////////////////////////////////////////
        // https://codereview.stackexchange.com/a/67394
        template <typename Tuple, typename Func, std::size_t ...Indices>
        void tuple_for_each_impl(Tuple&& tuple, Func&& func, std::index_sequence<Indices...>)
        {
            using swallow = int[];
            (void)swallow {
                1, (func(std::get<Indices>(std::forward<Tuple>(tuple))), void(), int{})...
            };
        }

        template <typename Tuple, typename Func>
        void tuple_for_each(Tuple&& tuple, Func&& f)
        {
            tuple_for_each_impl(
                std::forward<Tuple>(tuple),
                std::forward<Func>(f),
                std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
        }
        ////////////////////////////////////////

        ////////////////////////////////////////
        // https://stackoverflow.com/a/30346803
        template<bool...> 
        struct bool_pack;

        template<bool... Bs>
        using all_true = std::is_same<bool_pack<Bs..., true>, bool_pack<true, Bs...>>;
        ////////////////////////////////////////

        template <typename Base, typename... Ts>
        using all_base_of = all_true<std::is_base_of<Base, Ts>::value...>;

        template <typename To, typename... Ts>
        using all_convertible_to = all_true<std::is_convertible<Ts, To>::value...>;

        template <typename T, typename... Ts>
        using all_same = all_true<std::is_same<T, Ts>::value...>;

        template<typename T>
        using is_component = std::is_base_of<ComponentBase, T>;

        template<typename T>
        using is_system = std::is_base_of<SystemBase, T>;

        template<typename... Ts>
        using all_components = all_base_of<ComponentBase, Ts...>;

        template<typename... Ts>
        using all_systems = all_base_of<SystemBase, Ts...>;

        template <typename Tuple>
        struct tuple_all_components : std::false_type {};

        template <typename... Ts>
        struct tuple_all_components<std::tuple<Ts...>> : all_base_of<ComponentBase, Ts...> {};
    }
}

