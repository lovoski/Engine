#pragma once

#include "utils/Tref.hpp"
#include "utils/json.hpp"

#include "global.hpp"

#include "resource/MaterialData.hpp"

#define SerializableType TrefType
#define SerializableField TrefField

using Json = nlohmann::json;

// Register some serializable class
NLOHMANN_JSON_NAMESPACE_BEGIN
template <> struct adl_serializer<vec2> {
  static void to_json(json &j, const vec2 &v) {
    j["serializeType"] = "vec2";
    j["x"] = v.x;
    j["y"] = v.y;
  }
  static void from_json(const json &j, vec2 &v) {
    v.x = j["x"];
    v.y = j["y"];
  }
};
template <> struct adl_serializer<vec3> {
  static void to_json(json &j, const vec3 &v) {
    j["serializeType"] = "vec3";
    j["x"] = v.x;
    j["y"] = v.y;
    j["z"] = v.z;
  }
  static void from_json(const json &j, vec3 &v) {
    v.x = j["x"];
    v.y = j["y"];
    v.z = j["z"];
  }
};
template <> struct adl_serializer<quat> {
  static void to_json(json &j, const quat &q) {
    j["serializeType"] = "quat";
    j["w"] = q.w;
    j["x"] = q.x;
    j["y"] = q.y;
    j["z"] = q.z;
  }
  static void from_json(const json &j, quat &q) {
    q.w = j["w"];
    q.x = j["x"];
    q.y = j["y"];
    q.z = j["z"];
  }
};
NLOHMANN_JSON_NAMESPACE_END

namespace Reflection {

// deserialize from json file
template <typename T> bool Deserialize(Json &json, T &instance) {
  if (tref::is_reflected_v<T>) {
    cout << "deserializing type " << tref::class_info<T>().name << endl;
    if (json["serializeType"] == string(tref::class_info<T>().name)) {
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
      cout << "serialized type missmatch, source: " << json["serializeType"]
           << ", target: " << tref::class_info<T>().name << endl;
      return false;
    }
  } else {
    cout << "this type is not serializable" << endl;
    return false;
  }
}

// serialize to json file
template <typename T> bool Serialize(Json &json, T &instance) {
  if (tref::is_reflected_v<T>) {
    cout << "serializing type " << tref::class_info<T>().name << endl;
    json["serializeType"] = string(tref::class_info<T>().name);
    // serialize each field of the instance
    tref::class_info<T>().each_field([&](auto info, int) {
      auto ptr = info.value;
      using mem_t = decltype(ptr);
      if (std::is_same_v<tref::member_t<mem_t>, int> ||
          std::is_same_v<tref::member_t<mem_t>, float> ||
          std::is_same_v<tref::member_t<mem_t>, vec2> ||
          std::is_same_v<tref::member_t<mem_t>, vec3> ||
          std::is_same_v<tref::member_t<mem_t>, quat> ||
          std::is_same_v<tref::member_t<mem_t>, string> ||
          std::is_same_v<tref::member_t<mem_t>, ECS::EntityID> ||
          std::is_same_v<tref::member_t<mem_t>, MaterialData*>) {
        json[info.name] = instance.*ptr;
      } else {
        cout << "member " << info.name << " can't be serialized" << endl;
      }
      return true;
    });
    return true;
  } else {
    cout << "this type is not seriliazable" << endl;
    return false;
  }
}

}; // namespace Reflection