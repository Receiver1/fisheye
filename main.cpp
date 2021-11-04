
#include <windows.h>
#include <fstream>
#include <string>
#include <minhook/MinHook.h>

#include "samp.hpp"

using void_t = void(__cdecl *)();
void_t original__timer_update;

bool is_timer_initializated = false;
bool is_samp = false;
float fov = 70.0f;

void custom_fov_command(const char *args) {
  try {
    float value = std::stof(args);
    if (!value) {
      fov = 70.0f;
      return;
    }
    if (value >= 10.0f || value <= 140.0f) {
      fov = value;
    }
  } catch (std::exception err) {
    return;
  }
}

void hook__timer_update() {
  if (!is_timer_initializated && g_samp.is_initializated()) {
      g_samp.register_chat_command("customfov", custom_fov_command);
      is_timer_initializated = true;
  }

  float &default_fov_first = *reinterpret_cast<float *>(0x522F7AU);
  float &default_fov_second = *reinterpret_cast<float *>(0x858CE0U);

  if (default_fov_first != fov) {
    default_fov_first = fov;
  }

  if (default_fov_second != fov) {
    default_fov_second = fov;
  }

  original__timer_update();
}

class c_entry {
  void *timer_ptr{reinterpret_cast<void *>(0x561B10U)};

 public:
  c_entry() {
    std::ifstream file(PROJECT_NAME ".ini");
    if (file.is_open()) {
      std::string key, value;
      while (file >> key >> value) {
        if (key == "fov") {
          fov = std::stof(value);
        }
      }
      file.close();
    }

    MH_Initialize();

    MH_CreateHook(timer_ptr, &hook__timer_update,
                  reinterpret_cast<void **>(&original__timer_update));
    MH_EnableHook(timer_ptr);
  }
  ~c_entry() {
    std::ofstream file(PROJECT_NAME ".ini");
    if (file.is_open()) {
      file << "fov " << fov;
      file.close();
    }

    MH_RemoveHook(MH_ALL_HOOKS);
    MH_Uninitialize();
  }
} entry;