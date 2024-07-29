# eProsima DDS Helper Yaml Module

This library implements the required functions to translate a DDS Helper configuration written in *yaml*
format into C++ source code.
It is powered by `yaml-cpp` library.

It provides methods:

- to create every type of object from a *yaml* node,
- to read a *yaml* file, and
- to interact with a *yaml* object using `YAML::Node` class from `yaml-cpp`.

---

## Example of usage

```cpp
// LOAD DDS HELPER CONFIGURATION FROM FILE
eprosima::ddshelper::yaml::HelperConfiguration configuration("configuration.yaml");

// LOAD DDS REPLAYER CONFIGURATION FROM FILE
eprosima::ddshelper::yaml::ReplayerConfiguration configuration("configuration.yaml");
```

---

## Dependencies

* `yaml-cpp`
* `cpp_utils`
* `ddspipe_core`
* `ddspipe_participants`
* `ddspipe_yaml`
* `ddshelper_participants`

---

## How to use it in your project

Just import library `ddshelper_yaml` into your CMake project.

```cmake
find_package(ddshelper_yaml)
target_link_libraries(${LIBRARY_TARGET_NAME} ddshelper_yaml)
```
