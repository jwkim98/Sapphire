// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Motutapu/compute/Compute.hpp>
#include <Motutapu/compute/cuda/dense/Gemm.cuh>
#include <Motutapu/compute/naive/NaiveGemm.hpp>

namespace Motutapu::Compute
{
void Gemm(Util::TensorData& out, const Util::TensorData& a,
          const Util::TensorData& b, const Util::TensorData& c)
{
    const auto device = out.GetDevice();
    const auto paddedM = out.PaddedRowSize;
    const auto paddedN = out.PaddedColumnSize;
    const auto paddedK = a.PaddedRowSize;
    const auto batchSize = out.BatchSize;
    const auto broadCastA = a.BatchSize == 1;
    const auto broadCastB = b.BatchSize == 1;
    const auto broadCastC = c.BatchSize == 1;

    if (device.Type() == DeviceType::CUDA)
        Cuda::Dense::GemmNormalFloat(out.DenseMatCuda, a.DenseMatCuda,
                                     b.DenseMatCuda, c.DenseMatCuda, paddedM,
                                     paddedN, paddedK, batchSize, broadCastA,
                                     broadCastB, broadCastC);
    else
        Naive::Dense::NaiveGemm(out.DenseMatCuda, a.DenseMatCuda,
                                b.DenseMatCuda, c.DenseMatCuda, paddedM,
                                paddedN, paddedK, batchSize, broadCastA,
                                broadCastB, broadCastC);
}

void Gemm(Util::TensorData& out, const Util::TensorData& a,
          const Util::TensorData& b)
{
    const auto device = out.GetDevice();
    const auto paddedM = out.PaddedRowSize;
    const auto paddedN = out.PaddedColumnSize;
    const auto paddedK = a.PaddedRowSize;
    const auto batchSize = out.BatchSize;
    const auto broadCastA = a.BatchSize == 1;
    const auto broadCastB = b.BatchSize == 1;

    if (device.Type() == DeviceType::CUDA)
        Cuda::Dense::GemmNormalFloat(out.DenseMatCuda, a.DenseMatCuda,
                                     b.DenseMatCuda, out.DenseMatCuda, paddedM,
                                     paddedN, paddedK, batchSize, broadCastA,
                                     broadCastB, false);
    else
        Naive::Dense::NaiveGemm(out.DenseMatCuda, a.DenseMatCuda,
                                b.DenseMatCuda, out.DenseMatCuda, paddedM,
                                paddedN, paddedK, batchSize, broadCastA,
                                broadCastB, false);
}

}  // namespace Motutapu::Compute