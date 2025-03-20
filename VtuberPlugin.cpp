﻿/**
  Created by Weng Y on 2020/05/25.
  Copyright © 2020 Weng Y. Under GNU General Public License v2.0.
*/
#include "VtuberPlugin.hpp"

#include <obs-module.h>
#include <windows.h>

#include <codecvt>
#include <string>

#include "Live2DManager.hpp"
#include "VtuberDelegate.hpp"
#include "VtuberFrameWork.hpp"

namespace {
constexpr char kPropertyNameCaptureSpecificWindow[] = "capture_specific_window";
constexpr char kPropertyNameCaptureWindow[] = "capture_window";

struct Vtuber_data {
  obs_source_t *source;
  uint16_t modelId;
};

bool CaptureSpecificWindowCallback(obs_properties_t *ppts, obs_property_t *p,
                                   obs_data_t *settings) {
  bool value = obs_data_get_bool(settings, kPropertyNameCaptureSpecificWindow);
  obs_property_set_visible(obs_properties_get(ppts, kPropertyNameCaptureWindow),
                           value);
  return true;
}

bool IsWindowValid(HWND window) {
  if (!IsWindowVisible(window)) {
    return false;
  }

  DWORD styles = GetWindowLongPtrW(window, GWL_STYLE);
  if (styles & WS_CHILD) {
    return false;
  }

  DWORD ex_styles = GetWindowLongPtrW(window, GWL_EXSTYLE);
  if (ex_styles & WS_EX_TOOLWINDOW) {
    return false;
  }

  return true;
}

HWND NextWindow(HWND window, bool use_findwindowex) {
  do {
    if (use_findwindowex) {
      window = FindWindowExW(GetDesktopWindow(), window, nullptr, nullptr);
    } else {
      window = GetWindow(window, GW_HWNDNEXT);
    }
  } while (window != nullptr && !IsWindowValid(window));
  return window;
}

void FillWindowList(obs_property_t *p) {
  bool use_findwindowex = true;

  HWND window = FindWindowExW(GetDesktopWindow(), nullptr, nullptr, nullptr);
  if (window == nullptr) {
    use_findwindowex = false;
    window = GetWindow(GetDesktopWindow(), GW_CHILD);
  }

  if (!IsWindowValid(window)) {
    window = NextWindow(window, use_findwindowex);

    if (window == nullptr && use_findwindowex) {
      use_findwindowex = false;

      window = GetWindow(GetDesktopWindow(), GW_CHILD);
      if (!IsWindowValid(window)) {
        window = NextWindow(window, use_findwindowex);
      }
    }
  }

  while (window != nullptr) {
    int length = GetWindowTextLengthW(window);
    if (length != 0) {
      std::vector<wchar_t> buffer(length + 1);
      if (GetWindowTextW(window, buffer.data(), buffer.size())) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        std::string title_utf8 = convert.to_bytes(buffer.data());
        obs_property_list_add_string(p, title_utf8.c_str(), title_utf8.c_str());
      }
    }

    window = NextWindow(window, use_findwindowex);
  }
}

bool InsertSelectedWindowIfNotPresent(obs_property_t *p, obs_data_t *settings) {
  const char *current_value =
      obs_data_get_string(settings, kPropertyNameCaptureWindow);
  if (current_value == nullptr) {
    return false;
  }

  const char *value = nullptr;
  bool found = false;
  for (int i = 0; (value = obs_property_list_item_string(p, i)) != nullptr;
       ++i) {
    if (std::strcmp(value, current_value) == 0) {
      found = true;
      break;
    }
  }

  if (*current_value != '\0' && !found) {
    obs_property_list_insert_string(p, /*idx=*/1, current_value, current_value);
    obs_property_list_item_disable(p, /*idx=*/1, /*disabled=*/true);
    return true;
  }

  return false;
}

bool OnCaptureWindowChanged(obs_properties_t *ppts, obs_property_t *p,
                            obs_data_t *settings) {
  return InsertSelectedWindowIfNotPresent(p, settings);
}

void UpdateSettings(obs_data_t *settings) {
  std::string capture_window_utf8(
      obs_data_get_string(settings, kPropertyNameCaptureWindow));
  std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
  VtuberDelegate::GetInstance()->UpdateSettings({
      /*capture_specific_window=*/obs_data_get_bool(
          settings, kPropertyNameCaptureSpecificWindow),
      /*capture_window=*/convert.from_bytes(capture_window_utf8),
  });
}

}  // namespace

const char *VtuberPlugin::VtuberPlugin::VtuberGetName(void *unused) {
  UNUSED_PARAMETER(unused);
  return "bongobs cat";
}

void *VtuberPlugin::VtuberPlugin::VtuberCreate(obs_data_t *settings,
                                               obs_source_t *source) {
  static int id = 0;
  Vtuber_data *vtb = new Vtuber_data();
  vtb->source = source;
  vtb->modelId = id++;

  double x = obs_data_get_double(settings, "x");
  double y = obs_data_get_double(settings, "y");
  int width = obs_data_get_int(settings, "width");
  int height = obs_data_get_int(settings, "height");
  double scale = obs_data_get_double(settings, "scale");
  double delayTime = obs_data_get_double(settings, "delaytime");
  bool random_motion = obs_data_get_bool(settings, "random_motion");
  bool breath = obs_data_get_bool(settings, "breath");
  bool eyeblink = obs_data_get_bool(settings, "eyeblink");
  bool track = obs_data_get_bool(settings, "track");
  const char *mode = obs_data_get_string(settings, "mode");
  bool live2D = obs_data_get_bool(settings, "live2d");
  bool relative_mouse = obs_data_get_bool(settings, "relative_mouse");
  bool mouse_horizontal_flip =
      obs_data_get_bool(settings, "mouse_horizontal_flip");
  bool mouse_vertical_flip = obs_data_get_bool(settings, "mouse_vertical_flip");
  bool mask = obs_data_get_bool(settings, "mask");
  int screen_top_override = obs_data_get_int(settings, "screen_top_override");
  int screen_bottom_override =
      obs_data_get_int(settings, "screen_bottom_override");
  int screen_left_override = obs_data_get_int(settings, "screen_left_override");
  int screen_right_override =
      obs_data_get_int(settings, "screen_right_override");

  VtuberFrameWork::InitVtuber(vtb->modelId);

  VtuberFrameWork::UpData(vtb->modelId, x, y, width, height, scale, delayTime,
                          random_motion, breath, eyeblink, NULL, track, mode,
                          live2D, relative_mouse, mouse_horizontal_flip,
                          mouse_vertical_flip, mask);

  Live2DManager::GetInstance()->SetScreenOverride(
      vtb->modelId, screen_top_override, screen_bottom_override,
      screen_left_override, screen_right_override);

  UpdateSettings(settings);

  return vtb;
}

void VtuberPlugin::VtuberPlugin::VtuberDestroy(void *data) {
  Vtuber_data *spv = (Vtuber_data *)data;

  VtuberFrameWork::UinitVtuber(spv->modelId);

  delete spv;
}

void VtuberPlugin::VtuberPlugin::VtuberRender(void *data, gs_effect_t *effect) {
  Vtuber_data *spv = (Vtuber_data *)data;

  int width, height;
  width = VtuberFrameWork::GetWidth(spv->modelId);
  height = VtuberFrameWork::GetHeight(spv->modelId);

  obs_enter_graphics();

  gs_texture_t *tex =
      gs_texture_create(width, height, GS_RGBA, 1, NULL, GS_DYNAMIC);

  uint8_t *ptr;
  uint32_t linesize;
  if (gs_texture_map(tex, &ptr, &linesize))
    VtuberFrameWork::ReanderVtuber(spv->modelId, (char *)ptr, width, height);
  gs_texture_unmap(tex);

  obs_source_draw(tex, 0, 0, 0, 0, true);

  gs_texture_destroy(tex);

  obs_leave_graphics();
}

uint32_t VtuberPlugin::VtuberPlugin::VtuberWidth(void *data) {
  Vtuber_data *vtb = (Vtuber_data *)data;
  return VtuberFrameWork::GetWidth(vtb->modelId);
}

uint32_t VtuberPlugin::VtuberPlugin::VtuberHeight(void *data) {
  Vtuber_data *vtb = (Vtuber_data *)data;
  return VtuberFrameWork::GetHeight(vtb->modelId);
}

static void fill_vtuber_model_list(obs_property_t *p, void *data) {
  Vtuber_data *vtb = (Vtuber_data *)data;
  int _size;
  const char **_modeNames = VtuberFrameWork::GetModeDefine(_size);

  for (int index = 0; index < _size; index++) {
    obs_property_list_add_string(p, _modeNames[index], _modeNames[index]);
  }
}

obs_properties_t *VtuberPlugin::VtuberPlugin::VtuberGetProperties(void *data) {
  Vtuber_data *vtb = (Vtuber_data *)data;

  obs_properties_t *ppts = obs_properties_create();
  obs_properties_set_param(ppts, vtb, NULL);

  obs_property_t *p;

  // obs_properties_add_path(ppts, "models_path",obs_module_text("ModelsPath"),
  // OBS_PATH_FILE,"*.model3.json", "Resources/");

  p = obs_properties_add_list(ppts, "mode", obs_module_text("Mode"),
                              OBS_COMBO_TYPE_LIST, OBS_COMBO_FORMAT_STRING);
  fill_vtuber_model_list(p, data);

  // obs_property_set_modified_callback(p, vtuber_model_callback);
  // obs_properties_add_int(ppts, "width", obs_module_text("Width"), 32, 1900,
  // 32); obs_properties_add_int(ppts, "height", obs_module_text("Height"), 32,
  // 1050, 32);
  // obs_properties_add_float_slider(ppts,"scale",obs_module_text("Scale"),0.1,10.0,0.1);
  // obs_properties_add_float_slider(ppts, "x", obs_module_text("X"), -3.0, 3.0,
  // 0.1); obs_properties_add_float_slider(ppts, "y", obs_module_text("Y"),
  // -3.0, 3.0, 0.1);
  obs_properties_add_bool(ppts, "relative_mouse",
                          obs_module_text("Relative Mouse Movement"));
  obs_properties_add_bool(ppts, "mouse_horizontal_flip",
                          obs_module_text("Mouse Horizontal Flip"));
  obs_properties_add_bool(ppts, "mouse_vertical_flip",
                          obs_module_text("Mouse Vertical Flip"));
  obs_properties_add_bool(ppts, "mask", obs_module_text("Use Mask"));
  obs_properties_add_bool(ppts, "live2d", obs_module_text("Live2D"));
  obs_properties_add_float_slider(ppts, "delay", obs_module_text("Speed"), 0.0,
                                  10.0, 0.1);
  obs_properties_add_bool(ppts, "random_motion",
                          obs_module_text("RandomMotion"));
  obs_properties_add_bool(ppts, "breath", obs_module_text("Breath"));
  obs_properties_add_bool(ppts, "eyeblink", obs_module_text("EyeBlink"));
  obs_properties_add_bool(ppts, "track", obs_module_text("Mouse Tracktion"));
  obs_properties_add_int(ppts, "screen_top_override",
                         obs_module_text("Screen Top Override"), -1,
                         1'000'000'000, 1);
  obs_properties_add_int(ppts, "screen_bottom_override",
                         obs_module_text("Screen Bottom Override"), -1,
                         1'000'000'000, 1);
  obs_properties_add_int(ppts, "screen_left_override",
                         obs_module_text("Screen Left Override"), -1,
                         1'000'000'000, 1);
  obs_properties_add_int(ppts, "screen_right_override",
                         obs_module_text("Screen Right Override"), -1,
                         1'000'000'000, 1);

  p = obs_properties_add_bool(ppts, kPropertyNameCaptureSpecificWindow,
                              obs_module_text("Capture specific window"));
  obs_property_set_modified_callback(p, CaptureSpecificWindowCallback);

  p = obs_properties_add_list(ppts, kPropertyNameCaptureWindow,
                              obs_module_text("Window"), OBS_COMBO_TYPE_LIST,
                              OBS_COMBO_FORMAT_STRING);
  obs_property_list_add_string(p, "", "");
  FillWindowList(p);
  obs_property_set_modified_callback(p, OnCaptureWindowChanged);

  return ppts;
}

void VtuberPlugin::VtuberPlugin::Vtuber_update(void *data,
                                               obs_data_t *settings) {
  Vtuber_data *vtb = (Vtuber_data *)data;

  double x = obs_data_get_double(settings, "x");
  double y = obs_data_get_double(settings, "y");
  int width = obs_data_get_int(settings, "width");
  int height = obs_data_get_int(settings, "height");
  double vscale = obs_data_get_double(settings, "scale");
  double delayTime = obs_data_get_double(settings, "delay");
  bool random_motion = obs_data_get_bool(settings, "random_motion");
  bool breath = obs_data_get_bool(settings, "breath");
  bool eyeblink = obs_data_get_bool(settings, "eyeblink");
  bool track = obs_data_get_bool(settings, "track");
  const char *mode = obs_data_get_string(settings, "mode");
  bool live2D = obs_data_get_bool(settings, "live2d");
  bool relative_mouse = obs_data_get_bool(settings, "relative_mouse");
  bool mouse_horizontal_flip =
      obs_data_get_bool(settings, "mouse_horizontal_flip");
  bool mouse_vertical_flip = obs_data_get_bool(settings, "mouse_vertical_flip");
  bool mask = obs_data_get_bool(settings, "mask");
  int screen_top_override = obs_data_get_int(settings, "screen_top_override");
  int screen_bottom_override =
      obs_data_get_int(settings, "screen_bottom_override");
  int screen_left_override = obs_data_get_int(settings, "screen_left_override");
  int screen_right_override =
      obs_data_get_int(settings, "screen_right_override");

  const char *vtb_str = NULL;  // obs_data_get_string(settings, "models_path");

  VtuberFrameWork::UpData(vtb->modelId, x, y, width, height, vscale, delayTime,
                          random_motion, breath, eyeblink, vtb_str, track, mode,
                          live2D, relative_mouse, mouse_horizontal_flip,
                          mouse_vertical_flip, mask);

  Live2DManager::GetInstance()->SetScreenOverride(
      vtb->modelId, screen_top_override, screen_bottom_override,
      screen_left_override, screen_right_override);

  UpdateSettings(settings);
}

void VtuberPlugin::VtuberPlugin::Vtuber_defaults(obs_data_t *settings) {
  obs_data_set_default_int(settings, "width", 1280);
  obs_data_set_default_int(settings, "height", 768);
  obs_data_set_default_double(settings, "x", 0);
  obs_data_set_default_double(settings, "y", 0.02);
  obs_data_set_default_double(settings, "scale", 1.83);
  obs_data_set_default_double(settings, "delaytime", 1.0);
  obs_data_set_default_bool(settings, "random_motion", true);
  obs_data_set_default_bool(settings, "breath", true);
  obs_data_set_default_bool(settings, "eyeblink", true);
  obs_data_set_default_bool(settings, "track", true);
  obs_data_set_default_string(settings, "Mode", "standard");
  obs_data_set_default_bool(settings, "live2d", true);
  obs_data_set_default_bool(settings, "relative_mouse", false);
  obs_data_set_default_bool(settings, "mouse_horizontal_flip", true);
  obs_data_set_default_bool(settings, "mouse_vertical_flip", true);
  obs_data_set_default_bool(settings, "mask", false);
  obs_data_set_default_int(settings, "screen_top_override", -1);
  obs_data_set_default_int(settings, "screen_bottom_override", -1);
  obs_data_set_default_int(settings, "screen_left_override", -1);
  obs_data_set_default_int(settings, "screen_right_override", -1);
}
