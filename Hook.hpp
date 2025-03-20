#pragma once
#include <Windows.h>

#include <atomic>
#include <thread>

class Hook {
 public:
  Hook();

  ~Hook();

  void Strat();

  void Stop();

 private:
  void Run();
  void OnWmInput(HWND window, HRAWINPUT input);

  static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam,
                                           LPARAM lParam);

  static LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam,
                                        LPARAM lParam);

  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam,
                                     LPARAM lParam);

  HWND m_hWnd;

  std::thread *th;
  std::atomic_bool enabled_ = false;
  std::atomic_bool running_ = false;
};
