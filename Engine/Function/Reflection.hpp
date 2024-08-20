#pragma once

#include <NlohmannJson.hpp>
#include <Tref.hpp>

#include "Global.hpp"

namespace aEngine {

// deserialize from json file
template <typename T> bool Deserialize(nlohmann::json &json, T &instance) {
  std::cout << "deserializing type " << tref::class_info<T>().name << std::endl;
  if (json["serializeType"] == string(class_info<T>().name)) {
    // iterate through this json file
    for (auto it : json.items()) {
      tref::class_info<T>().each_field([&](auto info, int) {
        auto ptr = info.value;
        if (info.name == it.key()) {
          instance.*ptr = it.value();
        }
        return true;
      });
    }
    return true;
  } else {
    std::cout << "serialized type missmatch, source: " << json["serializeType"]
              << ", target: " << tref::class_info<T>().name << std::endl;
    return false;
  }
}

// serialize to json file
template <typename T> bool Serialize(nlohmann::json &json, T instance) {
  std::cout << "serializing type " << tref::class_info<T>().name << std::endl;
  json["serializeType"] = std::string(tref::class_info<T>().name);
  // serialize each field of the instance
  tref::class_info<T>().each_field([&](auto info, int) {
    auto ptr = info.value;
    using mem_t = decltype(ptr);
    if constexpr (std::is_same_v<tref::member_t<mem_t>, int> ||
                  std::is_same_v<tref::member_t<mem_t>, float>) {
      json[info.name] = instance.*ptr;
    } else if constexpr (std::is_same_v<tref::member_t<mem_t>, float>) {
      json[info.name] = instance.*ptr;
    } else if constexpr (std::is_same_v<tref::member_t<mem_t>, glm::vec2>) {
      json[info.name] = instance.*ptr;
    } else if constexpr (std::is_same_v<tref::member_t<mem_t>, glm::vec3>) {
      json[info.name] = instance.*ptr;
    } else if constexpr (std::is_same_v<tref::member_t<mem_t>, glm::vec4>) {
      json[info.name] = instance.*ptr;
    } else if constexpr (std::is_same_v<tref::member_t<mem_t>, glm::quat>) {
      json[info.name] = instance.*ptr;
    } else {
      std::cout << "member " << info.name << " is not a serializable type"
                << std::endl;
      return false;
    }
  });
  return true;
}

}; // namespace aEngine
