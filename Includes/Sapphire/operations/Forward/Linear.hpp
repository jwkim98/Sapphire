// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#ifndef SAPPHIRE_LINEAR_HPP
#define SAPPHIRE_LINEAR_HPP

#include <Sapphire/tensor/Tensor.hpp>
#include <Sapphire/operations/optimizers/Optimizer.hpp>
#include <Sapphire/util/SharedPtr.hpp>
#include <Sapphire/operations/Unit.hpp>

namespace Sapphire::NN
{
class Linear : public Unit
{
public:
    Linear(unsigned int inputFeatureSize, unsigned int outputFeatureSize,
           Util::SharedPtr<Optimizer::Optimizer> optimizer,
           Device device, bool isSparse = false);
    ~Linear() override = default;

    Linear(const Linear& linear) = default;
    Linear(Linear&& linear) noexcept = default;
    Linear& operator=(const Linear& linear) = default;
    Linear& operator=(Linear&& linear) noexcept = default;

    Tensor operator()(const Tensor& input);

private:
    [[nodiscard]] int m_registerOutputTensor(
        const TensorUtil::TensorDescriptor& xDesc) const;

    bool m_checkArguments(
        std::vector<TensorUtil::TensorDescriptor> arguments) override;

    unsigned int m_inputs;
    unsigned int m_outputs;
    Util::SharedPtr<Optimizer::Optimizer> m_optimizer;
    Device m_device;
    bool m_isSparse;
};
} // namespace Sapphire::NN

#endif  // Sapphire_LINEAR_HPP
