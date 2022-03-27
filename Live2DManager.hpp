/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at
 * https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#include <CubismFramework.hpp>
#include <Math/CubismMatrix44.hpp>
#include <Math/CubismViewMatrix.hpp>
#include <Type/csmVector.hpp>
#include <memory>
#include <vector>

#include "Model.hpp"

/**
 * @brief サンプルアプリケーションにおいてCubismModelを管理するクラス<br>
 *         モデル生成と破棄、タップイベントの処理、モデル切り替えを行う。
 *
 */
class Live2DManager {
  struct ModelData {
    ModelData() = default;
    ModelData(const ModelData &) = delete;
    ModelData &operator=(const ModelData &) = delete;
    ModelData(ModelData &&) = default;
    ModelData &operator=(ModelData &&) = default;

    std::vector<std::unique_ptr<Model>>
        _models;  ///< モデルインスタンスのコンテナ
    Csm::csmString _modelPath;
    int settings_id;
  };

 public:
  Live2DManager() = default;
  Live2DManager(const Live2DManager &) = delete;
  Live2DManager &operator=(const Live2DManager &) = delete;

  /**
   * @brief   クラスのインスタンス（シングルトン）を返す。<br>
   *           インスタンスが生成されていない場合は内部でインスタンを生成する。
   *
   * @return  クラスのインスタンス
   */
  static Live2DManager *GetInstance();

  /**
   * @brief   クラスのインスタンス（シングルトン）を解放する。
   *
   */
  static void ReleaseInstance();

  /**
   * @brief   現在のシーンで保持しているモデルを返す
   *
   * @param[in]   no  モデルリストのインデックス値
   * @return
   * モデルのインスタンスを返す。インデックス値が範囲外の場合はNULLを返す。
   */
  Model *GetModel(Csm::csmUint16 model_id) const;

  /**
   * @brief   画面を更新するときの処理
   *          モデルの更新処理および描画処理を行う
   */
  void OnUpdate(int id, Csm::csmUint16 model_id);

  /**
   * @brief   シーンを切り替える<br>
   *           サンプルアプリケーションではモデルセットの切り替えを行う。
   */
  Csm::csmInt32 ChangeScene(int id, const Csm::csmChar *_modelPath);

  void ChangeMouseMovement(int id, Csm::csmBool _mouse);

  void UpdateModelSetting(int id, Csm::csmBool randomMotion,
                          Csm::csmFloat32 delayTime, Csm::csmBool isBreath,
                          Csm::csmBool isEyeBlink, Csm::csmBool isTrack,
                          Csm::csmBool isMouseHorizontalFlip,
                          Csm::csmBool _sMouseVerticalFlip);

  void SetScreenOverride(int id, int screen_top_override,
                         int screen_bottom_override, int screen_left_override,
                         int screen_right_override);

 private:
  struct Settings {
    Csm::csmBool _isUseRelativeMouse;
    Csm::csmBool _randomMotion;
    Csm::csmFloat32 _delayTime;
    Csm::csmBool _isBreath;
    Csm::csmBool _isEyeBlink;
    Csm::csmBool _isTrack;
    Csm::csmBool _isMouseHorizontalFlip;
    Csm::csmBool _IsMouseVerticalFlip;

    int screen_top_override_;
    int screen_bottom_override_;
    int screen_left_override_;
    int screen_right_override_;
  };

  Settings &GetSettings(int id);

  std::vector<ModelData> model_data_ = std::vector<ModelData>(1);
  std::vector<Settings> settings_;
};
