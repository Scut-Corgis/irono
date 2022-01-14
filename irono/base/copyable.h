#pragma once

namespace irono
{

/// 变量是可拷贝的值语义，则继承它
class copyable
{
 protected:
  copyable() = default;
  ~copyable() = default;
};

}  // namespace irono


