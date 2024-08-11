#pragma once

#include <Tref.hpp>
#include <NlohmannJson.hpp>

namespace aEngine {

// deserialize from json file
template <typename T> bool Deserialize(nlohmann::json &json, T &instance) {
  if (tref::is_reflected_v<T>) {
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
  } else {
    std::cout << "this type is not serializable" << std::endl;
    return false;
  }
}

// serialize to json file
template <typename T> bool Serialize(Json &json, T instance) {
  if (tref::is_reflected_v<T>) {
    std::cout << "serializing type " << tref::class_info<T>().name << std::endl;
    json["serializeType"] = std::string(tref::class_info<T>().name);
    // serialize each field of the instance
    tref::class_info<T>().each_field([&](auto info, int) {
      auto ptr = info.value;
      using mem_t = decltype(ptr);
      if (std::is_same_v<member_t<mem_t>, int> ||
          std::is_same_v<member_t<mem_t>, float>) {
        json[info.name] = instance.*ptr;
      } else {
        std::cout << "member " << info.name << " can't be serialized" << std::endl;
      }
      return true;
    });
    return true;
  } else {
    std::cout << "this type is not seriliazable" << std::endl;
    return false;
  }
}

};
