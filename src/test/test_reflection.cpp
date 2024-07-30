#include "utils/Tref.hpp"
#include "utils/json.hpp"

#include "EngineConfig.h"

#include <iomanip>
#include <iostream>


using namespace std;
using namespace tref;
using Json = nlohmann::json;

#define SerializableType TrefType
#define SerializableField TrefField

const string basePath = REPO_SOURCE_DIR "/src/test/reflection";

struct SomeStruct {
  float a = -1.2f;
  float b = 1.53f;
};

struct Vec3 {
  double x, y, z;
};

ostream &operator<<(ostream &out, SomeStruct &ss) {
  out << std::fixed << std::setprecision(5) << ss.a << ", " << ss.b;
  return out;
}

NLOHMANN_JSON_NAMESPACE_BEGIN
template <> struct adl_serializer<SomeStruct> {
  static void to_json(json &j, const SomeStruct &ss) {
    j["serializeType"] = "SomeStruct";
    j["a"] = ss.a;
    j["b"] = ss.b;
  }

  static void from_json(const json &j, SomeStruct &ss) {
    ss.a = j["a"];
    ss.b = j["b"];
  }
};
NLOHMANN_JSON_NAMESPACE_END

class BaseComponent {
  SerializableType(BaseComponent);

public:
  BaseComponent() {}
  ~BaseComponent() {}

  int a;
  SerializableField(a);

  float b;
  SerializableField(b);

  SomeStruct ss;
  SerializableField(ss);

protected:
  unsigned int id;
};

// deserialize from json file
template <typename T> bool Deserialize(Json &json, T &instance) {
  if (is_reflected_v<T>) {
    cout << "deserializing type " << class_info<T>().name << endl;
    if (json["serializeType"] == string(class_info<T>().name)) {
      // iterate through this json file
      for (auto it : json.items()) {
        class_info<T>().each_field([&](auto info, int) {
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
           << ", target: " << class_info<T>().name << endl;
      return false;
    }
  } else {
    cout << "this type is not serializable" << endl;
    return false;
  }
}

// serialize to json file
template <typename T> bool Serialize(Json &json, T instance) {
  if (is_reflected_v<T>) {
    cout << "serializing type " << class_info<T>().name << endl;
    json["serializeType"] = string(class_info<T>().name);
    // serialize each field of the instance
    class_info<T>().each_field([&](auto info, int) {
      auto ptr = info.value;
      using mem_t = decltype(ptr);
      if (is_same_v<member_t<mem_t>, int> ||
          is_same_v<member_t<mem_t>, float> ||
          is_same_v<member_t<mem_t>, SomeStruct>) {
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

int main() {
  BaseComponent bc;
  bc.a = 10;
  bc.b = 1.0f;
  bc.ss.a = 1123.123f;
  bc.ss.b = 43212.64f;

  Json source;
  Serialize(source, bc);

  cout << source << endl;;

  BaseComponent nbc;
  Deserialize(source, nbc);

  int a = 0;

  return 0;
}