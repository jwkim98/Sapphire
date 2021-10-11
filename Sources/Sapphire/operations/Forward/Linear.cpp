// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Sapphire/Model.hpp>
#include <Sapphire/compute/BasicOps.hpp>
#include <Sapphire/operations/Backward/LinearBackward.hpp>
#include <Sapphire/operations/Forward/Linear.hpp>
#include <Sapphire/compute/Initialize.hpp>
#include <Sapphire/operations/Unit.hpp>
#include <Sapphire/util/UnitUtils.hpp>
#include <Sapphire/tensor/TensorData.hpp>

namespace Sapphire::NN
{
Linear::Linear(int inputFeatureSize, int outputFeatureSize,
               Optimizer::Optimizer* optimizer,
               CudaDevice device, bool isSparse)
    : Unit(optimizer), m_inputs(inputFeatureSize),
      m_outputs(outputFeatureSize),
      m_device(std::move(device)),
      m_isSparse(isSparse)
{
    if (m_isSparse)
        throw std::invalid_argument(
            "NN::Linear - Sparse version not implemented");
}

Tensor Linear::operator()(Tensor& x, Tensor weight, Tensor bias)
{
    auto mode = x.Mode();
    if (!Util::CheckModeEquality(mode, weight, bias))
        throw std::invalid_argument("NN::Linear - Device mode inequality");
    auto& model = ModelManager::CurModel();

    auto& xDesc =
        model.GetDescriptor(x.TensorDescriptorKey());
    m_checkArguments({ &xDesc });
    auto& weightDesc = model.GetDescriptor(weight.TensorDescriptorKey());
    auto& biasDesc = model.GetDescriptor(bias.TensorDescriptorKey());
    const auto yKey = m_registerOutputTensor(xDesc);
    auto& yDesc = model.GetDescriptor(yKey);
    yDesc.SetMode(mode);

    auto weightData = weightDesc.GetForwardData();
    auto biasData = biasDesc.GetForwardData();
    auto xData = xDesc.GetForwardData();
    auto dxData = xDesc.GetBackwardData();
    auto yData = yDesc.GetForwardData();
    auto dyData = yDesc.GetBackwardData();

    if (!m_exists("transposedWeight"))
    {
        auto transposedWeight =
            TensorUtil::TensorData(Shape({ m_outputs, m_inputs }), Type::Dense,
                                   weight.GetDevice(), true);
        transposedWeight.SetMode(weight.Mode());
        m_addTensorData("transposedWeight", transposedWeight);
    }

    if (!m_exists("ones"))
    {
        auto ones = TensorUtil::TensorData(bias.GetShape().GetTranspose(),
                                           Type::Dense,
                                           bias.GetDevice(), true);
        ones.SetMode(bias.Mode());
        Compute::Initialize::Ones(ones);
        m_addTensorData("ones", ones);
    }

    //! Change the dimension of the data to match the requirements
    Util::ChangeTensorDataDimension(2, xData, dxData, yData, dyData);

    if (!m_exists("expandedBias"))
    {
        auto expandedBias = TensorUtil::TensorData(
            yData.GetShape(), Type::Dense, bias.GetDevice(), true);
        expandedBias.SetMode(bias.Mode());
        m_addTensorData("expandedBias", expandedBias);
    }

    Compute::Initialize::Zeros(yData);
    Compute::Initialize::Zeros(m_getTensorData("expandedBias"));
    Compute::Transpose(m_getTensorData("transposedWeight"), weightData);
    Compute::Gemm(m_getTensorData("expandedBias"), m_getTensorData("ones"),
                  biasData, m_getTensorData("expandedBias"));
    Compute::Gemm(yData, xData, m_getTensorData("transposedWeight"),
                  m_getTensorData("expandedBias"));

    auto* backPropWrapper =
        new BackProp::LinearBackProp(
            dxData, dyData, weightData, biasData, xData,
            m_optimizer,
            xData.GetBatchSize(2));
    Util::SaveHistory(backPropWrapper, std::make_tuple(&xDesc),
                      std::make_tuple(&yDesc));
    return Tensor(yKey);
}

int Linear::m_registerOutputTensor(
    const TensorUtil::TensorDescriptor& xDesc) const
{
    auto& model = ModelManager::CurModel();
    const auto x = xDesc.GetForwardData();
    const Shape xShape = xDesc.GetShape();
    Shape yShape = xShape;
    yShape[yShape.Dim() - 1] = m_outputs;
    const auto yKey = model.RegisterTensorDescriptor(
        yShape, xDesc.GetType(), xDesc.GetDevice());
    return yKey;
}

void Linear::m_checkArguments(
    std::vector<TensorUtil::TensorDescriptor*> arguments) const
{
    const auto input = arguments.at(0);
    if (input->GetShape().Cols() != m_inputs)
        throw std::invalid_argument("NN::Linear - Shape mismatch");
}
} // namespace Sapphire::NN
