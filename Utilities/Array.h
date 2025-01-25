#pragma once

#include <array>

// std::array is an object which holds only a static contiguous C Array, but which has a lot of array-helper functions. So it's more convenient. Aka a wrapper for C-array.
// The performance is exactly the same as C Array.
namespace Array
{
  void Demonstrate();

  template <typename T>
  concept NumericType = std::integral<T> || std::floating_point<T>;

  template<NumericType num, size_t size>
  std::string ToString(const std::array<num, size>& arr)
  {
    std::string result;

    for (size_t i = 0; i < size; i++)
    {
      result += std::to_string(arr[i]) + ", ";
    }

    if (size > 0)
    {
      result.resize(result.size() - 2);
    }

    return result;
  }

  template<size_t size>
  std::string ToString(const std::array<std::string, size>& arr)
  {
    std::string result;

    for (size_t i = 0; i < size; i++)
    {
      result += arr[i] + ", ";
    }

    if (size > 0)
    {
      result.resize(result.size() - 2);
    }

    return result;
  }
};