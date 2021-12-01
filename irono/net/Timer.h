/*
 * @Author: your name
 * @Date: 2021-11-28 16:45:32
 * @LastEditTime: 2021-12-01 15:31:57
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /irono/irono/net/Timer.h
 */
#pragma once
#include "../base/noncopyable.h"

#include "../base/Timestamp.h"
#include "Callbacks.h"

namespace irono
{

///实际的定时器类对象
class Timer : noncopyable
{
 public:
  Timer(const TimerCallback& cb, Timestamp when, double interval)
    : callback_(cb),
      expiration_(when),
      interval_(interval),
      repeat_(interval > 0.0)
  { }

  void run() const
  {
    callback_();
  }

  Timestamp expiration() const  { return expiration_; }
  bool repeat() const { return repeat_; }

  void restart(Timestamp now);

 private:
  const TimerCallback callback_;
  Timestamp expiration_;
  const double interval_;
  const bool repeat_;
};

}
