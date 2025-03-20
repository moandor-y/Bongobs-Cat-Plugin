﻿/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at
 * https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#pragma once

#define KEYAMOUT 200

#include <mutex>
#include <vector>

struct KeyEvent {
  bool KeyBoardSignal;
};

class EventManager {
 public:
  EventManager();
  ~EventManager();

  float GetCenterX() const { return _lastX; }
  float GetCenterY() const { return _lastY; }
  float GetDeltaX() const { return _deltaX; }
  float GetDeltaY() const { return _deltaY; }
  float GetStartX() const { return _startX; }
  float GetStartY() const { return _startY; }
  float GetScale() const { return _scale; }
  float GetX() const { return _lastX; }
  float GetY() const { return _lastY; }
  float GetX1() const { return _lastX1; }
  float GetY1() const { return _lastY1; }
  float GetX2() const { return _lastX2; }
  float GetY2() const { return _lastY2; }
  bool GetKeySignal(int i) { return _keyEvent[i].KeyBoardSignal; }
  bool GetKeySignal2(int i) { return _keyEvent[i].KeyBoardSignal; }
  bool GetRightButton() { return _rightButton; }
  bool GetLeftButton() { return _leftButton; }

  /*
   * @brief タッチ開始時イベント
   *
   * @param[in] deviceY    タッチした画面のyの値
   * @param[in] deviceX    タッチした画面のxの値
   */
  void MouseEventBegan(float deviceX, float deviceY);

  /*
   * @brief ドラッグ時のイベント
   *
   * @param[in] deviceX    タッチした画面のyの値
   * @param[in] deviceY    タッチした画面のxの値
   */
  void MouseEventMoved(float deviceX, float deviceY);

  /*
   * @brief ドラッグ時のイベント
   *
   * @param[in] deviceX1   1つめのタッチした画面のxの値
   * @param[in] deviceY1   1つめのタッチした画面のyの値
   * @param[in] deviceX2   2つめのタッチした画面のxの値
   * @param[in] deviceY2   2つめのタッチした画面のyの値
   */
  void MouseEventMoved(float deviceX1, float deviceY1, float deviceX2,
                       float deviceY2);

  /*
   * @brief
   *
   * @param[in] width
   * @param[in] height
   * @param[in] deviceX
   * @param[in] deviceY
   */
  void MouseEventMoved(int width, int height, float deviceX, float deviceY);

  void KeyEventDown(int key);

  void KeyEventUp(int key);

  void LeftButtonDown();

  void LeftButtonUp();

  void RightButtonDown();

  void RightButtonUp();

  void SetRelativeMouse(int _rx, int _ry);

  void GetRelativeMouse(int &_rx, int &_ry);

  void GetCurrentMousePosition(int &_x, int &_y);

  /*
   * @brief フリックの距離測定
   *
   * @return フリック距離
   */
  float GetFlickDistance() const;

  std::vector<int> GetPressedKeys();

  void AllKeysUp();

 private:
  /*
   * @brief 点1から点2への距離を求める
   *
   * @param[in] x1 1つめのタッチした画面のxの値
   * @param[in] y1 1つめのタッチした画面のyの値
   * @param[in] x2 2つめのタッチした画面のxの値
   * @param[in] y2 2つめのタッチした画面のyの値
   * @return   2点の距離
   */
  float CalculateDistance(float x1, float y1, float x2, float y2) const;

  /*
   * 二つの値から、移動量を求める。
   * 違う方向の場合は移動量０。同じ方向の場合は、絶対値が小さい方の値を参照する
   *
   * @param[in] v1    1つめの移動量
   * @param[in] v2    2つめの移動量
   *
   * @return   小さい方の移動量
   */
  float CalculateMovingAmount(float v1, float v2);

  float _startY;  // タッチを開始した時のxの値
  float _startX;  // タッチを開始した時のyの値
  float _lastX;   // シングルタッチ時のxの値
  float _lastY;   // シングルタッチ時のyの値
  float _lastX1;  // ダブルタッチ時の一つ目のxの値
  float _lastY1;  // ダブルタッチ時の一つ目のyの値
  float _lastX2;  // ダブルタッチ時の二つ目のxの値
  float _lastY2;  // ダブルタッチ時の二つ目のyの値
  float _deltaX;  // 前回の値から今回の値へのxの移動距離。
  float _deltaY;  // 前回の値から今回の値へのyの移動距離。
  float _scale;  // このフレームで掛け合わせる拡大率。拡大操作中以外は1。

  KeyEvent _keyEvent[KEYAMOUT];

  bool _leftButton;
  bool _rightButton;

  int _relativemouseX;
  int _relativemouseY;

  std::vector<int> pressed_keys_;

  std::mutex mutex_;
};
