// MIT License

// Copyright (c) 2018 crazybie

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Tref: A *T*iny compile time *ref*lection system.

/***********************************************************************
Copyright 2019-2020 crazybie<soniced@sina.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef TREF_H
#define TREF_H
#pragma once

#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>

#define ZTrefHasTref
#define ZTrefVersion 0x010000

#ifdef _MSC_VER
#define ZTrefCxxVersion _MSVC_LANG
#else
#define ZTrefCxxVersion __cplusplus
#endif

#if ZTrefCxxVersion < 201700L
#error "Need a c++17 compiler"
#endif

#ifndef TrefMaxElems
#define TrefMaxElems 255
#endif

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////
///
/// Common facility
///
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct Type {
  using type = T;
};

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
}

template <typename T, typename Fn, size_t... Idx>
constexpr bool tuple_for_each_idx(T&& tp, Fn&& fn, index_sequence<Idx...>) {
  return (std::forward<Fn>(fn)(get<Idx>(std::forward<T>(tp))) && ...);
}

template <typename T, typename Fn>
constexpr bool tuple_for_each(T&& tp, Fn&& fn) {
  using Tp = remove_reference_t<T>;
  return tuple_for_each_idx(std::forward<T>(tp), std::forward<Fn>(fn), make_index_sequence<tuple_size_v<Tp>>());
}

template <typename D, typename S, size_t... I>
constexpr auto tuple_convert(S&& s, index_sequence<I...>) {
  return make_tuple(tuple_element_t<I, D>{get<I>(std::forward<S>(s))}...);
}

// member pointer trait

template <class C>
struct member_pointer_trait {
  using enclosing_class_t = void;
  using member_t = void;
};

template <class T, class C>
struct member_pointer_trait<T C::*> {
  using member_t = T;
  using enclosing_class_t = C;
};

template <class T>
using member_t = typename member_pointer_trait<T>::member_t;

template <class T>
using enclosing_class_t = typename member_pointer_trait<T>::enclosing_class_t;

// function trait

template <typename T>
struct func_trait : func_trait<decltype(&T::operator())> {};

template <typename R, typename C, typename... A>
struct func_trait<R (C::*)(A...) const> {
  using args_t = tuple<A...>;
  static constexpr auto args_count = sizeof...(A);
  using ret_t = R;
};

template <typename R, typename C, typename... A>
struct func_trait<R (C::*)(A...)> {
  using args_t = tuple<A...>;
  static constexpr auto args_count = sizeof...(A);
  using ret_t = R;
};

// function overloading helper

template <typename... Args>
struct Overload {
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...)) const {
    return ptr;
  }
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...) const) const {
    return ptr;
  }
  template <typename R>
  constexpr auto operator()(R (*ptr)(Args...)) const {
    return ptr;
  }
};

template <typename... Args>
constexpr Overload<Args...> overload_v{};

template <typename... Meta>
struct Metas : Meta... {
  using Meta::Meta...;
  constexpr explicit Metas(Meta... m)
      : Meta(m)... {}
};

//  common macros

#define ZTrefReturn(...) \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

#define ZTrefMsvcExpand(...) __VA_ARGS__
#define ZTrefDelay(X, ...) ZTrefMsvcExpand(X(__VA_ARGS__))
#define ZTrefDelay2(X, ...) ZTrefMsvcExpand(X(__VA_ARGS__))
#define ZTrefFirst(...) ZTrefMsvcExpand(ZTrefFirst2(__VA_ARGS__))
#define ZTrefFirst2(A, ...) A
#define ZTrefSecond(...) ZTrefMsvcExpand(ZTrefSecond2(__VA_ARGS__))
#define ZTrefSecond2(A, B, ...) B
#define ZTrefTail(A, ...) __VA_ARGS__
#define ZTrefStringify(...) ZTrefStringify2(__VA_ARGS__)
#define ZTrefStringify2(...) #__VA_ARGS__

#define ZTrefRemoveParen(A) \
  ZTrefDelay(ZTrefRemoveParen2, ZTrefRemoveParenHelper A)
#define ZTrefRemoveParen2(...) \
  ZTrefDelay2(ZTrefTail, ZTrefRemoveParenHelper##__VA_ARGS__)
#define ZTrefRemoveParenHelper(...) _, __VA_ARGS__
#define ZTrefRemoveParenHelperZTrefRemoveParenHelper _,

#define ZTrefFirstRemoveParen(X) ZTrefFirst(ZTrefRemoveParen(X))
#define ZTrefSecondRemoveParen(X) ZTrefSecond(ZTrefRemoveParen(X))

// macro version of map

#define ZTrefMap(f, arg1, ...)               \
  ZTrefMsvcExpand(ZTrefDelay(ZTrefChooseMap, \
                             ZTrefCount(__VA_ARGS__))(f, arg1, __VA_ARGS__))

#define ZTrefChooseMap(N) ZTrefMap##N

#define ZTrefMap1(m, a, x) m(a, x)
#define ZTrefMap2(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap1(m, a, __VA_ARGS__))
#define ZTrefMap3(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap2(m, a, __VA_ARGS__))
#define ZTrefMap4(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap3(m, a, __VA_ARGS__))
#define ZTrefMap5(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap4(m, a, __VA_ARGS__))
#define ZTrefMap6(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap5(m, a, __VA_ARGS__))
#define ZTrefMap7(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap6(m, a, __VA_ARGS__))
#define ZTrefMap8(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap7(m, a, __VA_ARGS__))
#define ZTrefMap9(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap8(m, a, __VA_ARGS__))
#define ZTrefMap10(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap9(m, a, __VA_ARGS__))
#define ZTrefMap11(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap10(m, a, __VA_ARGS__))
#define ZTrefMap12(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap11(m, a, __VA_ARGS__))
#define ZTrefMap13(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap12(m, a, __VA_ARGS__))
#define ZTrefMap14(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap13(m, a, __VA_ARGS__))
#define ZTrefMap15(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap14(m, a, __VA_ARGS__))
#define ZTrefMap16(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap15(m, a, __VA_ARGS__))
#define ZTrefMap17(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap16(m, a, __VA_ARGS__))
#define ZTrefMap18(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap17(m, a, __VA_ARGS__))
#define ZTrefMap19(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap18(m, a, __VA_ARGS__))
#define ZTrefMap20(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap19(m, a, __VA_ARGS__))
#define ZTrefMap21(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap20(m, a, __VA_ARGS__))
#define ZTrefMap22(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap21(m, a, __VA_ARGS__))
#define ZTrefMap23(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap22(m, a, __VA_ARGS__))
#define ZTrefMap24(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap23(m, a, __VA_ARGS__))
#define ZTrefMap25(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap24(m, a, __VA_ARGS__))
#define ZTrefMap26(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap25(m, a, __VA_ARGS__))
#define ZTrefMap27(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap26(m, a, __VA_ARGS__))
#define ZTrefMap28(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap27(m, a, __VA_ARGS__))
#define ZTrefMap29(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap28(m, a, __VA_ARGS__))
#define ZTrefMap30(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap29(m, a, __VA_ARGS__))

#define ZTrefEvaluateCount(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                           _13, _14, _15, _16, _17, _18, _19, _20, _21, _22,  \
                           _23, _24, _25, _26, _27, _28, _29, _30, N, ...)    \
  N

#define ZTrefCount(...)                                                        \
  ZTrefMsvcExpand(ZTrefEvaluateCount(                                          \
      __VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

//////////////////////////////////////////////////////////////////////////
///
/// Core facility
///
//////////////////////////////////////////////////////////////////////////

template <int N = TrefMaxElems>
struct Slot : Slot<N - 1> {
  enum { value = N };
};

template <>
struct Slot<0> {
  enum { value = 0 };
};

constexpr auto invalid_index = 0;

template <typename C, typename Tag>
tuple<Slot<invalid_index>> _tref_slot(C**, Tag, Slot<0> slot);

template <typename T>
constexpr auto slot_cnt = std::tuple_element_t<0, T>::value;

#define ZTrefSlotCnt(C, Tag) \
  tref::imp::slot_cnt<decltype(_tref_slot((ZTrefRemoveParen(C)**)0, Tag{}, tref::imp::Slot<>{}))>

#define ZTrefSlotPush(C, Tag, ...)                                                                      \
  constexpr auto _tref_slot(ZTrefRemoveParen(C)**, Tag, tref::imp::Slot<ZTrefSlotCnt(C, Tag) + 1> slot) \
      ZTrefReturn(std::tuple(slot, __VA_ARGS__))

template <class C, class Tag, int Idx>
constexpr auto slot_data() {
  return get<1>(_tref_slot((C**)0, Tag{}, Slot<Idx>{}));
}

template <class C, class Tag, class F, size_t... Is>
constexpr bool slots_data_fold(index_sequence<Is...>, F&& f) {
  return (f(slot_data<C, Tag, Is>()) && ...);
}

template <typename C, typename Tag, typename F>
constexpr bool each_slots_data(F f) {
  constexpr auto cnt = ZTrefSlotCnt(C, Tag);
  if constexpr (cnt > 0) {
    return slots_data_fold<C, Tag>(tail(make_index_sequence<cnt + 1>{}), f);
  } else {
    return true;
  }
}

template <class C, class Tag, size_t... Is>
constexpr auto all_slots_data_fold(index_sequence<Is...>) {
  return make_tuple(slot_data<C, Tag, Is>()...);
}

template <typename C, typename Tag>
constexpr auto all_slots_data() {
  constexpr auto cnt = ZTrefSlotCnt(C, Tag);
  return all_slots_data_fold<C, Tag>(tail(make_index_sequence<cnt + 1>{}));
}

//////////////////////////////////////////////////////////////////////////
///
/// class reflection
///
//////////////////////////////////////////////////////////////////////////

struct DummyBase;

struct FieldTag {};
struct MemberTypeTag {};
struct SubclassTag {};

void _tref_class_info(void*);

template <typename T>
constexpr auto is_reflected_v = !std::is_same_v<decltype(_tref_class_info((T**)0)), void>;

// Use function to delay the evaluation. (for non-conformance mode of MSVC)
template <typename T>
constexpr auto class_info() {
  return _tref_class_info((T**)0);
}

template <typename T>
constexpr auto class_info_v = class_info<T>();

template <typename T>
using class_info_t = decltype(class_info_v<T>);

// Use macro to delay the evaluation. (for non-conformance mode of MSVC)
#define ZTrefBaseOf(T) \
  typename decltype(tref::imp::class_info<ZTrefRemoveParen(T)>())::base_t

template <typename T>
constexpr auto has_base_class_v = is_reflected_v<T> && !std::is_same_v<ZTrefBaseOf(T), DummyBase>;

template <typename TP, size_t... I>
constexpr auto unpack_subclass(TP, index_sequence<I...>) {
  return tuple_cat(class_info<typename tuple_element_t<I, TP>::type>().get_subclasses()...);
}

// Meta for Member

template <typename T, typename Meta>
struct FieldInfo {
  using enclosing_class_t = imp::enclosing_class_t<T>;
  using member_t = imp::member_t<T>;

  static constexpr auto is_member_v = !is_same_v<enclosing_class_t, void>;

  int         index;
  string_view name;

  // Possible values:
  // 1. Address of member variables & functions
  // 2. Type<T> for member type T: use decltype(value)::type to retrieve it.
  T    value;
  Meta meta;

  constexpr FieldInfo(int idx, string_view n, T a, Meta m)
      : index{idx}, name{n}, value{a}, meta{m} {}
};

// Meta for class

template <typename T, typename Base, typename Meta>
struct ClassInfo {
  using class_t = T;
  using base_t = Base;

  string_view name;
  size_t      size;
  Type<Base>  base;
  Meta        meta;

  constexpr ClassInfo(T*, string_view n, size_t sz, Type<Base> b, Meta m)
      : name{n}, size{sz}, base{b}, meta{m} {
    if constexpr (!is_same_v<Base, DummyBase>) {
      static_assert(is_base_of_v<Base, class_t>, "invalid base class");
    }
  }

  constexpr auto get_fields() const {
    auto f = all_slots_data<T, FieldTag>();
    if constexpr (!is_same_v<Base, DummyBase>) {
      return tuple_cat(class_info<Base>().get_fields(), f);
    } else {
      return f;
    }
  }

  template <typename Tag, typename F>
  constexpr bool each(F&& f, int level = 0) const {
    auto next = each_slots_data<T, Tag>([&](const auto& info) { return f(info, level); });
    if (next)
      if constexpr (!is_same_v<Base, DummyBase>)
        return class_info<Base>().template each<Tag>(f, level + 1);
    return next;
  }

  // Iterate through the members recursively.
  // @param f: [](FieldInfo info, int level) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_field(F&& f) const {
    return each<FieldTag>(f);
  }

  template <typename FilterMeta, typename F>
  constexpr bool each_field_with_meta(F&& f) const {
    return each<FieldTag>([&](auto info, int level) {
      if constexpr (std::is_convertible_v<decltype(info.meta), FilterMeta>) {
        return f(info, level);
      }
      return true;
    });
  }

  constexpr auto get_field_index(string_view name) const {
    int  idx = 0;
    bool found = false;
    tuple_for_each(get_fields(), [&](auto f) {
      if (f.name == name) {
        found = true;
        return false;
      }
      idx++;
      return true;
    });
    return found ? idx : -1;
  }

  template <size_t index>
  constexpr auto get_field() const {
    return get<index>(get_fields());
  }

  // Iterate through the subclasses recursively.
  // @param F: [](ClassInfo info, int level) -> bool, return false to stop the
  // iterating.
  //
  // NOTE: must call this function in a template function.
  template <typename F>
  constexpr bool each_subclass(F&& f, int level = 0) const {
    return each_slots_data<T, SubclassTag>([&](auto info) {
      using S = typename decltype(info)::type;
      return f(class_info<S>(), level) &&
             class_info<S>().each_subclass(f, level + 1);
    });
  }

  // NOTE: must call this function in a template function.
  constexpr auto get_subclass_index(string_view name) const {
    int  idx = 0;
    bool found = false;
    tuple_for_each(get_subclasses(), [&](auto t) {
      if (class_info<typename decltype(t)::type>().name == name) {
        found = true;
        return false;
      }
      idx++;
      return true;
    });
    return found ? idx : -1;
  }

  // NOTE: must call this function in a template function.
  template <typename Sub>
  constexpr auto get_subclass_index() const {
    int  idx = 0;
    bool found = false;
    tuple_for_each(get_subclasses(), [&](auto t) {
      using S = typename decltype(t)::type;
      if (is_same_v<Sub, S>) {
        found = true;
        return false;
      }
      idx++;
      return true;
    });
    return found ? idx : -1;
  }

  // NOTE: must call this function in a template function.
  template <size_t index>
  constexpr auto get_subclass() const {
    return get<index>(get_subclasses());
  }

  // NOTE: must call this function in a template function.
  template <typename F>
  constexpr auto get_subclass(int index, F&& f) const {
    int  idx = 0;
    bool found = false;
    tuple_for_each(get_subclasses(), [&](auto t) {
      if (idx == index) {
        f(t);
        return false;
      }
      idx++;
      return true;
    });
    return found;
  }

  // NOTE: must call this function in a template function.
  constexpr auto get_subclasses() const {
    constexpr auto f = all_slots_data<T, SubclassTag>();
    constexpr int  cnt = tuple_size_v<decltype(f)>;
    constexpr auto idx = make_index_sequence<cnt>();
    if constexpr (cnt) {
      auto sub = unpack_subclass(f, idx);
      if constexpr (tuple_size_v<decltype(sub)>) {
        return tuple_cat(f, sub);
      } else {
        return f;
      }
    } else {
      return f;
    }
  }

  // NOTE: must call this function in a template function.
  constexpr auto get_direct_subclasses() const {
    return all_slots_data<T, SubclassTag>();
  }

  // NOTE: must call this function in a template function.
  template <typename FilterMeta, typename F>
  constexpr bool each_subclass_with_meta(F&& f) const {
    return each_subclass([&](auto info, int level) {
      if constexpr (std::is_convertible_v<decltype(info.meta), FilterMeta>) {
        return f(info, level);
      }
      return true;
    });
  }

  // Iterate through the member types.
  // @param F: [](Type info) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_member_type(F&& f) const {
    return each<MemberTypeTag>(f);
  }

  template <typename FilterMeta, typename F>
  constexpr bool each_member_type_with_meta(F&& f) const {
    return each_member_type<FieldTag>([&](auto info, int level) {
      if constexpr (std::is_convertible_v<decltype(info.meta), FilterMeta>) {
        return f(info, level);
      }
      return true;
    });
  }
};

template <typename T>
constexpr auto class_fields_v = class_info_v<T>.get_fields();

template <typename T, typename F>
constexpr auto each_field(F&& f) {
  return tuple_for_each(class_fields_v<T>, f);
}

// NOTE: must call this function in a template function.
template <typename T, typename F>
constexpr auto each_subclass(F&& f) {
  return class_info_v<T>.each_subclass(f);
}

template <typename T, typename S, typename = std::enable_if_t<std::is_base_of_v<T, S>>>
constexpr auto subclass_id = class_info_v<T>.template get_subclass_index<S>();

template <typename T, typename... Args>
T* create_subclass(int subclassId, Args&&... args) {
  T* ret{};
  class_info_v<T>.template get_subclass(subclassId, [&](auto c) {
    using C = typename decltype(c)::type;
    ret = new C(std::forward<Args>(args)...);
  });
  return ret;
};

#define ZTrefClassMetaImp(T, Base, meta)                              \
  constexpr auto _tref_class_info(ZTrefRemoveParen(T)**) {            \
    return tref::imp::ClassInfo{                                      \
        (ZTrefRemoveParen(T)*)0, ZTrefStringify(ZTrefRemoveParen(T)), \
        sizeof(ZTrefRemoveParen(T)),                                  \
        tref::imp::Type<ZTrefRemoveParen(Base)>{}, meta};             \
  }

#define ZTrefClassMeta(T, Base, meta) friend ZTrefClassMetaImp(T, Base, meta)

#define ZTrefPushFieldImp(T, Tag, name, val, meta) \
  ZTrefSlotPush(T, Tag, tref::imp::FieldInfo{slot.value, name, val, meta})

//////////////////////////////////////////////////////////////////////////
///
/// macros for class reflection
///
//////////////////////////////////////////////////////////////////////////

// sub type

template <typename T, class = void_t<>>
struct get_parent {
  using type = DummyBase;
};

template <typename T>
struct get_parent<T, void_t<typename T::__base_of_child_t>> {
  using type = typename T::__base_of_child_t;
};

// NOTE: needed only when you want to iterate all children from base class.
#define ZTrefSubType(T) \
  ZTrefSubTypeImp(T, (typename ZTrefRemoveParen(T)::base_t))

// NOTE: can't put into class:
// 1. template overloads to increase the id will not work on clang.
// 1. friend function unrelated to the current class may be removed by clang.
#define ZTrefSubTypeImp(T, Base)                                                      \
  ZTrefSlotPush(Base, tref::imp::SubclassTag, tref::imp::Type<ZTrefRemoveParen(T)>{}) \
      ZTrefAllowSemicolon(ZTrefRemoveParen(T))

// fix lint issue: `TrefSubType(T);` : empty statement.
#define ZTrefAllowSemicolon(...) using zUnused = std::void_t<__VA_ARGS__>

//

#define ZTrefTypeCommon(T, Base, meta) \
 private:                              \
  using this_t = ZTrefRemoveParen(T);  \
                                       \
 public:                               \
  using __base_of_child_t = this_t;    \
  ZTrefClassMeta(T, Base, meta);

// Just reflect the type.

#define ZTrefType(T) ZTrefTypeWithMeta(T, nullptr)
#define ZTrefTypeWithMeta(T, meta)                                          \
 public:                                                                    \
  using base_t = typename tref::imp::get_parent<ZTrefRemoveParen(T)>::type; \
  ZTrefTypeCommon(T, base_t, meta);

// reflect member variable & function

#define ZTrefField1(t) ZTrefFieldWithMeta2(t, nullptr)
#define ZTrefFieldWithMeta2(t, meta) ZTrefFieldWithMeta2Imp(this_t, t, meta)
#define ZTrefFieldWithMeta2Imp(T, t, meta)               \
  ZTrefPushFieldImp(T, tref::imp::FieldTag,              \
                    ZTrefStringify(ZTrefRemoveParen(t)), \
                    &T::ZTrefRemoveParen(t), meta)

// provide arguments for overloaded function
#define ZTrefField2(t, sig) ZTrefFieldWithMeta3(t, sig, nullptr)
#define ZTrefFieldWithMeta3(t, sig, meta) \
  ZTrefFieldWithMeta3Imp(this_t, t, sig, meta)

#define ZTrefFieldWithMeta3Imp(T, t, sig, meta)                              \
  ZTrefPushFieldImp(                                                         \
      T, tref::imp::FieldTag, ZTrefStringify(ZTrefRemoveParen(t)),           \
      tref::imp::overload_v<ZTrefRemoveParen(sig)>(&T::ZTrefRemoveParen(t)), \
      meta)

// auto select from ZTrefField1 or ZTrefField2 by argument count
#define ZTrefFieldImp(...) \
  ZTrefMsvcExpand(         \
      ZTrefDelay(ZTrefChooseField, ZTrefCount(__VA_ARGS__))(__VA_ARGS__))
#define ZTrefChooseField(N) ZTrefField##N

#define ZTrefField(...) friend ZTrefFieldImp(__VA_ARGS__)

// auto select from ZTrefFieldWithMeta2 or ZTrefFieldWithMeta3 by argument
// count
#define ZTrefFieldWithMetaImp(...)                     \
  ZTrefMsvcExpand(ZTrefDelay(ZTrefChooseFieldWithMeta, \
                             ZTrefCount(__VA_ARGS__))(__VA_ARGS__))
#define ZTrefChooseFieldWithMeta(N) ZTrefFieldWithMeta##N

#define ZTrefFieldWithMeta(...) friend ZTrefFieldWithMetaImp(__VA_ARGS__)

// reflect member type
#define ZTrefMemberTypeImp(T) ZTrefMemberTypeWithMetaImp(T, nullptr)
#define ZTrefMemberTypeWithMetaImp(T, meta)              \
  ZTrefPushFieldImp(this_t, tref::imp::MemberTypeTag,    \
                    ZTrefStringify(ZTrefRemoveParen(T)), \
                    tref::imp::Type<ZTrefRemoveParen(T)>{}, meta)

#define ZTrefMemberType(T) friend ZTrefMemberTypeImp(T)
#define ZTrefMemberTypeWithMeta(T, meta) \
  friend ZTrefMemberTypeWithMetaImp(T, meta)

//////////////////////////////////////////////////////////////////////////
///
/// enum reflection
///
//////////////////////////////////////////////////////////////////////////

struct EnumValueConvertor {
  template <typename T>
  constexpr explicit EnumValueConvertor(T v)
      : value((size_t)v) {
    static_assert(sizeof(T) <= sizeof(value));
  }

  template <typename U>
  constexpr EnumValueConvertor& operator=(U) {
    return *this;
  }

  template <typename U>
  constexpr operator U() {
    static_assert(is_enum_v<U>);
    return (U)value;
  }

  size_t value = 0;
};

template <typename T, typename Meta>
struct EnumItem {
  char name[255]{};
  T    value;
  Meta meta;

  constexpr EnumItem(string_view name, T value, Meta meta)
      : value{value}, meta{meta} {
    std::copy(name.data(), name.data() + name.size(), this->name);
  }

  constexpr string_view name_view() {
    return name;
  }
};

template <typename T, size_t N, typename ItemMeta>
using EnumItems = array<EnumItem<T, ItemMeta>, N>;

template <typename T, typename BASE, size_t N, typename Meta, typename ItemMeta>
struct EnumInfo {
  using enum_t = T;
  using base_t = BASE;

  string_view               name;
  EnumItems<T, N, ItemMeta> items;
  size_t                    size;
  Meta                      meta;

  // @param f: [](auto item)-> bool, return false to stop
  // the iterating.
  template <typename F>
  constexpr auto each_item(F&& f) const {
    for (auto& e : items) {
      if (!f(e))
        return false;
    }
    return true;
  }

  static constexpr auto npos = -1;

  constexpr int index_of_value(T v) {
    auto i = 0;
    for (auto& e : items) {
      if (e.value == v) {
        return i;
      }
      i++;
    }
    return npos;
  }

  constexpr int index_of_name(string_view n) {
    auto i = 0;
    for (auto& e : items) {
      if (e.name == n) {
        return i;
      }
      i++;
    }
    return npos;
  }
};

template <typename T, typename BASE, int N, typename Meta, typename ItemMeta>
constexpr auto makeEnumInfo(string_view                      name,
                            const EnumItems<T, N, ItemMeta>& items,
                            Meta                             meta) {
  return EnumInfo<T, BASE, N, Meta, ItemMeta>{name, items, N, meta};
}

void _tref_enum_info(void*);

template <typename T, typename = enable_if_t<is_enum_v<T>>>
constexpr auto enum_info() {
  return _tref_enum_info((T**)0);
}

template <typename T>
constexpr auto is_reflected_enum_v = !std::is_same_v<decltype(_tref_enum_info((T**)0)), void>;

template <typename T>
constexpr auto enum_info_v = enum_info<T>();

template <typename T>
using enum_info_t = decltype(enum_info_v<T>);

struct Enums {};
#define ZTrefEnumRegister(T, Tag) ZTrefSlotPush(tref::imp::Enums, Tag, tref::imp::Type<T>{})

template <typename Tag, typename F>
constexpr bool each_enum(F f) {
  return each_slots_data<Enums, Tag>([&](const auto& info) { return f(info); });
}

// Use it out of class.
#define ZTrefEnum(T, BASE, ...) ZTrefEnumWithMeta(T, BASE, nullptr, __VA_ARGS__)
#define ZTrefEnumWithMeta(T, BASE, meta, ...) \
  enum class T : BASE { __VA_ARGS__ };        \
  ZTrefEnumImpWithMeta(T, BASE, meta, __VA_ARGS__)

// Use it inside of class.
#define ZTrefMemberEnum(T, BASE, ...) ZTrefMemberEnumWithMeta(T, BASE, 0, __VA_ARGS__)
#define ZTrefMemberEnumWithMeta(T, BASE, meta, ...) \
  enum class T : BASE { __VA_ARGS__ };              \
  friend ZTrefEnumImpWithMeta(T, BASE, meta, __VA_ARGS__)

// Reflect enum items of already defined enum.
#define ZTrefEnumImp(T, BASE, ...) ZTrefEnumImpWithMeta(T, BASE, nullptr, __VA_ARGS__)
#define ZTrefEnumImpWithMeta(T, BASE, meta, ...)                                        \
  constexpr auto _tref_enum_info(ZTrefRemoveParen(T)**) {                               \
    return tref::imp::makeEnumInfo<ZTrefRemoveParen(T), BASE, ZTrefCount(__VA_ARGS__)>( \
        ZTrefStringify(ZTrefRemoveParen(T)),                                            \
        std::array{ZTrefEnumStringize(T, __VA_ARGS__)},                                 \
        std::move(meta));                                                               \
  }

#define ZTrefEnumStringize(P, ...) \
  ZTrefMsvcExpand(ZTrefMap(ZTrefEnumStringizeSingle, P, __VA_ARGS__))

// (EnumValueConvertor)EnumType::EnumItem = EnumItemValue,
#define ZTrefEnumStringizeSingle(P, E)                                         \
  tref::imp::EnumItem<ZTrefRemoveParen(P), std::nullptr_t>{                    \
      tref::imp::enum_trim_name(#E),                                           \
      (tref::imp::EnumValueConvertor)ZTrefRemoveParen(P)::ZTrefRemoveParen(E), \
      nullptr},

//////////////////////////////////////////////////////////////////////////
// ex version support meta for enum items.

#define ZTrefMemberEnumEx(T, BASE, ...) ZTrefMemberEnumWithMetaEx(T, BASE, 0, __VA_ARGS__)
#define ZTrefMemberEnumWithMetaEx(T, BASE, meta, ...) \
  ZTrefEnumDefineEnum2(T, BASE, __VA_ARGS__);         \
  friend ZTrefEnumImpWithMetaEx(T, BASE, meta, __VA_ARGS__)

#define ZTrefEnumEx(T, BASE, ...) ZTrefEnumWithMetaEx(T, BASE, nullptr, __VA_ARGS__)

#define ZTrefEnumWithMetaEx(T, BASE, meta, ...) \
  ZTrefEnumDefineEnum2(T, BASE, __VA_ARGS__);   \
  ZTrefEnumImpWithMetaEx(T, BASE, meta, __VA_ARGS__)

#define ZTrefFirstArgAsEnumItemDef(P, E) ZTrefFirstRemoveParen(E),
#define ZTrefEnumDefineEnum2(T, BASE, ...)                                \
  enum class T : BASE {                                                   \
    ZTrefMsvcExpand(ZTrefMap(ZTrefFirstArgAsEnumItemDef, T, __VA_ARGS__)) \
  }

#define ZTrefEnumImpEx(T, BASE, ...) ZTrefEnumImpWithMetaEx(T, BASE, 0, __VA_ARGS__)
#define ZTrefEnumImpWithMetaEx(T, BASE, meta, ...)                         \
  constexpr auto _tref_enum_info(ZTrefRemoveParen(T)**) {                  \
    return tref::imp::makeEnumInfo<ZTrefRemoveParen(T), BASE,              \
                                   ZTrefCount(__VA_ARGS__)>(               \
        ZTrefStringify(ZTrefRemoveParen(T)),                               \
        std::array{ZTrefEnumStringize2(T, __VA_ARGS__)}, std::move(meta)); \
  }

#define ZTrefEnumStringize2(P, ...) \
  ZTrefMsvcExpand(ZTrefMap(ZTrefEnumStringizeSingle2, P, __VA_ARGS__))

#define ZTrefEnumStringizeSingle2(P, E)                                       \
  tref::imp::EnumItem<ZTrefRemoveParen(P),                                    \
                      decltype(ZTrefRemoveParen(ZTrefSecondRemoveParen(E)))>{ \
      tref::imp::enum_trim_name(ZTrefStringify(ZTrefFirstRemoveParen(E))),    \
      (tref::imp::EnumValueConvertor)ZTrefRemoveParen(P)::ZTrefRemoveParen(   \
          ZTrefFirstRemoveParen(E)),                                          \
      ZTrefRemoveParen(ZTrefSecondRemoveParen(E))},

/////////////////////////////////////

constexpr string_view enum_trim_name(string_view s) {
  auto p = s.find_first_of('=');
  if (p != string_view::npos)
    p = s.rfind(' ', p);
  return s.substr(0, p);
}

template <typename T>
constexpr string_view enum_to_string(T v) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info_v<T>.items) {
    if (e.value == v) {
      return e.name;
    }
  }
  return {};
}

template <typename T>
constexpr T string_to_enum(string_view s, T default_) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info_v<T>.items) {
    if (s == e.name) {
      return e.value;
    }
  }
  return default_;
}

}  // namespace imp

//////////////////////////////////////////////////////////////////////////
///
/// public APIs
///
//////////////////////////////////////////////////////////////////////////

#define TrefHasTref ZTrefHasTref
#define TrefVersion ZTrefVersion

using imp::class_fields_v;
using imp::class_info;
using imp::class_info_t;
using imp::class_info_v;
using imp::ClassInfo;
using imp::create_subclass;
using imp::each_field;
using imp::each_subclass;
using imp::enclosing_class_t;
using imp::enum_info_v;
using imp::FieldInfo;
using imp::func_trait;
using imp::has_base_class_v;
using imp::is_reflected_v;
using imp::member_t;
using imp::Metas;
using imp::overload_v;
using imp::subclass_id;
using imp::tuple_convert;
using imp::tuple_for_each;

#define TrefType ZTrefType
#define TrefTypeWithMeta ZTrefTypeWithMeta
#define TrefSubType ZTrefSubType
#define TrefBaseOf ZTrefBaseOf

#define TrefField ZTrefField
#define TrefFieldWithMeta ZTrefFieldWithMeta
#define TrefMemberType ZTrefMemberType
#define TrefMemberTypeWithMeta ZTrefMemberTypeWithMeta

//////////////////////////
// Reflect external types
// NOTE: do not support template.
//////////////////////////

#define TrefNoBase tref::imp::DummyBase
#define TrefExternalTypeWithMeta ZTrefClassMetaImp
#define TrefExternalFieldWithMeta ZTrefFieldWithMeta2Imp
#define TrefExternalOverloadedFieldWithMeta ZTrefFieldWithMeta3Imp
#define TrefExternalSubType ZTrefSubTypeImp

/// enum

using imp::each_enum;
using imp::enum_info;
using imp::enum_info_t;
using imp::enum_to_string;
using imp::EnumInfo;
using imp::EnumItem;
using imp::is_reflected_enum_v;
using imp::string_to_enum;

// ex version support meta for enum items.

#define TrefEnum ZTrefEnum
#define TrefEnumEx ZTrefEnumEx
#define TrefEnumWithMeta ZTrefEnumWithMeta
#define TrefEnumWithMetaEx ZTrefEnumWithMetaEx
#define TrefMemberEnum ZTrefMemberEnum
#define TrefMemberEnumEx ZTrefMemberEnumEx
#define TrefMemberEnumWithMeta ZTrefMemberEnumWithMeta
#define TrefMemberEnumWithMetaEx ZTrefMemberEnumWithMetaEx
#define TrefExternalEnum ZTrefEnumImp
#define TrefExternalEnumEx ZTrefEnumImpEx
#define TrefExternalEnumWithMeta ZTrefEnumImpWithMeta
#define TrefExternalEnumWithMetaEx ZTrefEnumImpWithMetaEx
#define TrefEnumRegister ZTrefEnumRegister

}  // namespace tref

#endif