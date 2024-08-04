#include "utils/Tref.hpp"
#include "utils/json.hpp"

#include <iomanip>
#include <tuple>
#include <iostream>


using namespace std;
using namespace tref;
using Json = nlohmann::json;

#define SerializableType TrefType
#define SerializableField TrefField

const string basePath = "/src/test/reflection";

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

using EntityID = size_t;
using ComponentTypeID = size_t;

inline ComponentTypeID GetRuntimeComponentTypeID() {
  static ComponentTypeID typeID = 0u;
  return typeID++;
}

// attach type id to component class and return it
template <typename T> inline std::tuple<ComponentTypeID, T> ComponentType() noexcept {
  // the class should be inferented from component, but not the class itself
  static const ComponentTypeID typeID = GetRuntimeComponentTypeID();
  static T instance;
  return std::make_tuple(typeID, instance);
}

class ComponentA {
public:
  ComponentA() {}
  ~ComponentA() {}

  int prop1;
};

class BC {
public:
  BC() {}
  virtual ~BC() = default;
};

typedef std::map<std::string, BC*(*)()> mapType;
static mapType registerMap;

template<typename T>
inline BC *createInstance() { return new T; }

template<typename T>
void RegisterType() {
  if (tref::is_reflected_v<T>) {
    registerMap[string(tref::class_info<T>().name)] = &createInstance<T>;
  } else {
    std::cout << "can't register type without reflection" << std::endl;
  }
}

class CompA : public BC {
  SerializableType(CompA);

  int a = 10;
  int b = 20;
};
class CompB : public BC {
  SerializableType(CompB);

  int c = 10;
};
class CompC : public BC {
  // SerializableType(CompC);
};

int main() {
  // BaseComponent bc;
  // bc.a = 10;
  // bc.b = 1.0f;
  // bc.ss.a = 1123.123f;
  // bc.ss.b = 43212.64f;

  // Json source;
  // Serialize(source, bc);

  // cout << source << endl;;

  // BaseComponent nbc;
  // Deserialize(source, nbc);

  // int a = 0;

  // auto [id, instance] = ComponentType<ComponentA>();

  // if (std::is_same_v<decltype(instance), ComponentA>) {
  //   cout << "instance is componentA" << endl;
  // } else cout << "instance is not componentA" << endl;

  // RegisterType<CompA>();
  // RegisterType<CompB>();
  // // RegisterType<CompC>();

  // auto ptr1 = registerMap["CompA"]();

  // cout << ((CompA*)ptr1)->a << endl;

  // if (std::is_same_v<decltype(*ptr1), CompA>) cout << "111" << endl;
  // else cout << "222" << endl;

  // auto ptr2 = registerMap["CompB"]();

  // int a = 10;
  // while (a-- > 0) {
  //   cout << "!" << endl;
  // }

  return 0;
}