/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at
 * https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include "Live2DManager.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <Rendering/CubismRenderer.hpp>
#include <string>

#include "Define.hpp"
#include "EventManager.hpp"
#include "Model.hpp"
#include "Pal.hpp"
#include "View.hpp"
#include "VtuberDelegate.hpp"

using namespace Csm;
using namespace Define;
using namespace std;

namespace {
Live2DManager* s_instance = NULL;
}

Live2DManager* Live2DManager::GetInstance() {
  if (s_instance == NULL) {
    s_instance = new Live2DManager();
  }

  return s_instance;
}

void Live2DManager::ReleaseInstance() {
  if (s_instance != NULL) {
    delete s_instance;
  }

  s_instance = NULL;
}

Model* Live2DManager::GetModel(Csm::csmUint16 model_id) const {
  if (0 < model_data_[model_id]._models.size()) {
    return model_data_[model_id]._models[0].get();
  }
  return NULL;
}

void Live2DManager::OnUpdate(int id, Csm::csmUint16 model_id) {
  CubismMatrix44 projection;

  // update mouse position
  EventManager* _eventmanager;
  _eventmanager = VtuberDelegate::GetInstance()->GetView()->GetEventManager();

  int width, height;
  Pal::GetDesktopResolution(width, height);
  int px, py;
  const auto& settings = GetSettings(id);
  if (settings._isUseRelativeMouse) {
    VtuberDelegate::GetInstance()
        ->GetView()
        ->GetEventManager()
        ->GetRelativeMouse(px, py);

    _eventmanager->MouseEventMoved(width, height, px, py);
    px = _eventmanager->GetCenterX() - _eventmanager->GetStartX() + width / 2;
    py = _eventmanager->GetCenterY() - _eventmanager->GetStartY() + height / 2;

  } else {
    VtuberDelegate::GetInstance()
        ->GetView()
        ->GetEventManager()
        ->GetCurrentMousePosition(px, py);
  }

  if (settings.screen_left_override_ >= 0 &&
      settings.screen_right_override_ >= 0 &&
      settings.screen_right_override_ > settings.screen_left_override_) {
    width = settings.screen_right_override_ - settings.screen_left_override_;
  }
  if (settings.screen_top_override_ >= 0 &&
      settings.screen_bottom_override_ >= 0 &&
      settings.screen_bottom_override_ > settings.screen_top_override_) {
    height = settings.screen_bottom_override_ - settings.screen_top_override_;
  }
  if (settings.screen_left_override_ >= 0) {
    px -= settings.screen_left_override_;
  }
  if (settings.screen_top_override_ >= 0) {
    py -= settings.screen_top_override_;
  }

  float mousex = static_cast<float>(px) / static_cast<float>(width);
  float mousey = static_cast<float>(py) / static_cast<float>(height);

  // update model view matrix
  CubismMatrix44* _viewMatrix;
  _viewMatrix = VtuberDelegate::GetInstance()->GetView()->GetViewMatrix(id);

  if (_viewMatrix != NULL) {
    projection.MultiplyByMatrix(_viewMatrix);
  }

  Model* model = GetModel(model_id);

  if (model) {
    VtuberDelegate::GetInstance()->GetView()->PreModelDraw(*model);

    model->UpdateTime();
    model->UpdataSetting(settings._randomMotion, settings._delayTime,
                         settings._isBreath, settings._isEyeBlink,
                         settings._isTrack, settings._isMouseHorizontalFlip,
                         settings._IsMouseVerticalFlip);
    model->UpdateMouseState(mousex, mousey, _eventmanager->GetLeftButton(),
                            _eventmanager->GetRightButton());
    model->Update(model_id);
    model->Draw(projection);

    VtuberDelegate::GetInstance()->GetView()->PostModelDraw(*model);
  }
}

Csm::csmInt32 Live2DManager::ChangeScene(int id,
                                         const Csm::csmChar* _modelPath) {
  if (strcmp(_modelPath, "") == 0) return -1;

  int model_id = model_data_.size() - 1;
  if (strcmp(model_data_[model_id]._modelPath.GetRawString(), _modelPath) == 0)
    return model_id;

  // E:/obspl/build/rundir/Debug/bin/64bit/Resources/l2d03.u/l2d03.u.model3.json
  string modelFilePath = std::string(_modelPath);
  size_t pos = modelFilePath.rfind("/");
  string modelPath = modelFilePath.substr(0, pos + 1);
  string modelJsonName =
      modelFilePath.substr(pos + 1, modelFilePath.size() - pos - 1);

  if (strcmp(modelJsonName.c_str(), "") == 0) return -1;

  if (VtuberDelegate::GetInstance()->isLoadResource(model_id)) {
    model_data_[model_id] = ModelData();
  }

  model_data_[model_id]._models.push_back(std::make_unique<Model>());

  if (model_data_[model_id]._models[0]->LoadAssets(modelPath.c_str(),
                                                   modelJsonName.c_str())) {
    model_data_.emplace_back();
    model_data_[model_id]._modelPath = _modelPath;
  } else {
    model_data_[model_id]._models.clear();
    return -1;
  }

  /*
   * モデル半透明表示を行うサンプルを提示する。
   * ここでUSE_RENDER_TARGET、USE_MODEL_RENDER_TARGETが定義されている場合
   * 別のレンダリングターゲットにモデルを描画し、描画結果をテクスチャとして別のスプライトに張り付ける。
   */
  {
#if defined(USE_RENDER_TARGET)
    // Viewの持つターゲットに描画を行う場合、こちらを選択
    SelectTarget useRenderTarget = SelectTarget_ViewFrameBuffer;
#elif defined(USE_MODEL_RENDER_TARGET)
    // 各Modelの持つターゲットに描画を行う場合、こちらを選択
    SelectTarget useRenderTarget = SelectTarget_ModelFrameBuffer;
#else
    // デフォルトのメインフレームバッファへレンダリングする(通常)
    SelectTarget useRenderTarget = SelectTarget_None;
#endif

#if defined(USE_RENDER_TARGET) || defined(USE_MODEL_RENDER_TARGET)
    // モデル個別にαを付けるサンプルとして、もう1体モデルを作成し、少し位置をずらす
    _models.PushBack(new Model());
    _models[1]->LoadAssets(modelPath.c_str(), modelJsonName.c_str());
    _models[1]->GetModelMatrix()->TranslateX(0.2f);
#endif

    VtuberDelegate::GetInstance()->GetView()->SwitchRenderingTarget(
        useRenderTarget, model_id);

    // 別レンダリング先を選択した際の背景クリア色
    float clearColor[3] = {1.0f, 1.0f, 1.0f};
    VtuberDelegate::GetInstance()->GetView()->SetRenderTargetClearColor(
        clearColor[0], clearColor[1], clearColor[2]);

    return model_id;
  }
}

void Live2DManager::ChangeMouseMovement(int id, Csm::csmBool _mouse) {
  auto& settings = GetSettings(id);
  if (settings._isUseRelativeMouse != _mouse) {
    settings._isUseRelativeMouse = _mouse;
    if (_mouse) {
      EventManager* _eventmanager;
      _eventmanager =
          VtuberDelegate::GetInstance()->GetView()->GetEventManager();

      int px, py;
      VtuberDelegate::GetInstance()
          ->GetView()
          ->GetEventManager()
          ->GetRelativeMouse(px, py);
      _eventmanager->MouseEventBegan(px, py);
    }
  }
}

void Live2DManager::UpdateModelSetting(
    int id, Csm::csmBool randomMotion, Csm::csmFloat32 delayTime,
    Csm::csmBool isBreath, Csm::csmBool isEyeBlink, Csm::csmBool isTrack,
    Csm::csmBool isMouseHorizontalFlip, Csm::csmBool IsMouseVerticalFlip) {
  auto& settings = GetSettings(id);
  settings._randomMotion = randomMotion;
  settings._delayTime = delayTime;
  settings._isBreath = isBreath;
  settings._isEyeBlink = isEyeBlink;
  settings._isTrack = isTrack;
  settings._isMouseHorizontalFlip = isMouseHorizontalFlip;
  settings._IsMouseVerticalFlip = IsMouseVerticalFlip;
}

void Live2DManager::SetScreenOverride(int id, int screen_top_override,
                                      int screen_bottom_override,
                                      int screen_left_override,
                                      int screen_right_override) {
  auto& settings = GetSettings(id);
  settings.screen_top_override_ = screen_top_override;
  settings.screen_bottom_override_ = screen_bottom_override;
  settings.screen_left_override_ = screen_left_override;
  settings.screen_right_override_ = screen_right_override;
}

Live2DManager::Settings& Live2DManager::GetSettings(int id) {
  if (id >= settings_.size()) {
    settings_.resize(id + 1);
  }
  return settings_[id];
}
