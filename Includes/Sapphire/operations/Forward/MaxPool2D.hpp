// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef SAPPHIRE_NN_FUNCTIONAL_MAX_POOL_2D
#define SAPPHIRE_NN_FUNCTIONAL_MAX_POOL_2D

#include <Sapphire/operations/Unit.hpp>
#include <Sapphire/tensor/Tensor.hpp>
#include <utility>

namespace Sapphire::NN
{
class MaxPool2D : public Unit
{
public:
    MaxPool2D(std::pair<int, int> windowSize,
              std::pair<int, int> stride,
              std::pair<int, int> padSize = std::pair(0, 0));

    Tensor operator()(const Tensor& tensor);


private:
    [[nodiscard]] int m_registerOutputTensor(
        const TensorUtil::TensorDescriptor& xDesc) const;

    void m_checkArguments(
        std::vector<TensorUtil::TensorDescriptor*> arguments) const;

    static int m_unitIdCount;
    std::pair<int, int> m_windowSize, m_stride, m_padSize;

    int m_channels = -1;
    bool m_isSparse = false;
    int m_yRows = -1;
    int m_yCols = -1;
};
}

#endif
