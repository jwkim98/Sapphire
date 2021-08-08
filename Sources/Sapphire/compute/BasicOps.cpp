// Copyright (c) 2021, Justin Kim

// We are making my contributions/submissions to this project solely in our
// personal capacity and are not conveying any rights to any intellectual
// property of any third parties.

#include <Sapphire/compute/Broadcast.hpp>
#include <Sapphire/compute/BasicOps.hpp>
#include <Sapphire/compute/dense/cuda/Basic.cuh>
#include <Sapphire/compute/dense/cuda/Gemm.cuh>
#include <Sapphire/compute/dense/naive/NaiveBasic.hpp>
#include <Sapphire/compute/dense/naive/NaiveGemm.hpp>
#include <Sapphire/compute/dense/cuda/BasicBackward.cuh>
#include <algorithm>

namespace Sapphire::Compute
{
void Add(TensorData& y, const TensorData& a, const TensorData& b)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;

    auto shapeOut = y.TensorShape;
    auto shapeA = a.TensorShape;
    auto shapeB = b.TensorShape;

    const auto maxDim = std::max(
        { y.TensorShape.Dim(), a.TensorShape.Dim(), b.TensorShape.Dim() });

    shapeOut.Expand(maxDim);
    shapeA.Expand(maxDim);
    shapeB.Expand(maxDim);

    const auto sizeOut = shapeOut.Size();
    const auto sizeA = shapeA.Size();
    const auto sizeB = shapeB.Size();

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, sizeOut, sizeA, sizeB,
                             y.GetMutableDenseCuda(), a.GetDenseCuda(),
                             b.GetDenseCuda(), 0,
                             1, Dense::Cuda::Add, 0, false, false);
    }
    else
    {
        const auto paddedSizeOut = (sizeOut / N) * paddedN;
        const auto paddedSizeA = (sizeA / N) * paddedN;
        const auto paddedSizeB = (sizeB / N) * paddedN;
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, paddedSizeOut,
                             paddedSizeA, paddedSizeB, y.GetMutableDenseHost(),
                             a.GetDenseHost(), b.GetDenseHost(), 0, 1,
                             Dense::Naive::Add, 0, false, false);
    }
}

void Sub(TensorData& y, const TensorData& a, const TensorData& b)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;

    auto shapeOut = y.TensorShape;
    auto shapeA = a.TensorShape;
    auto shapeB = b.TensorShape;

    const auto maxDim = std::max(
        { y.TensorShape.Dim(), a.TensorShape.Dim(), b.TensorShape.Dim() });

    shapeOut.Expand(maxDim);
    shapeA.Expand(maxDim);
    shapeB.Expand(maxDim);

    const auto sizeOut = shapeOut.Size();
    const auto sizeA = shapeA.Size();
    const auto sizeB = shapeB.Size();

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, sizeOut, sizeA, sizeB,
                             y.GetMutableDenseCuda(), a.GetDenseCuda(),
                             b.GetDenseCuda(), 0, 1, Dense::Cuda::Sub, 0, false,
                             false);
    }
    else
    {
        const auto paddedSizeOut = (sizeOut / N) * paddedN;
        const auto paddedSizeA = (sizeA / N) * paddedN;
        const auto paddedSizeB = (sizeB / N) * paddedN;
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, paddedSizeOut,
                             paddedSizeA, paddedSizeB, y.GetMutableDenseHost(),
                             a.GetDenseHost(), b.GetDenseHost(), 0, 1,
                             Dense::Naive::Sub, 0, false, false);
    }
}

void Dot(TensorData& y, const TensorData& a, const TensorData& b)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;

    auto shapeOut = y.TensorShape;
    auto shapeA = a.TensorShape;
    auto shapeB = b.TensorShape;

    const auto maxDim = std::max(
        { y.TensorShape.Dim(), a.TensorShape.Dim(), b.TensorShape.Dim() });

    shapeOut.Expand(maxDim);
    shapeA.Expand(maxDim);
    shapeB.Expand(maxDim);

    const auto sizeOut = shapeOut.Size();
    const auto sizeA = shapeA.Size();
    const auto sizeB = shapeB.Size();

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, sizeOut, sizeA, sizeB,
                             y.GetMutableDenseCuda(), a.GetDenseCuda(),
                             b.GetDenseCuda(), 0, 1, Dense::Cuda::Dot, 0, false,
                             false);
    }
    else
    {
        const auto paddedSizeOut = (sizeOut / N) * paddedN;
        const auto paddedSizeA = (sizeA / N) * paddedN;
        const auto paddedSizeB = (sizeB / N) * paddedN;
        BroadcastWith2Inputs(shapeOut, shapeA, shapeB, paddedSizeOut,
                             paddedSizeA, paddedSizeB, y.GetMutableDenseHost(),
                             a.GetDenseHost(), b.GetDenseHost(), 0, 1,
                             Dense::Naive::Dot, 0, false, false);
    }
}

void Gemm(TensorData& y, const TensorData& a, const TensorData& b,
          const TensorData& c)
{
    auto shapeOut = y.TensorShape;
    auto shapeA = a.TensorShape;
    auto shapeB = b.TensorShape;
    auto shapeC = c.TensorShape;

    //! treat Make inputs, outputs to have at least 2 dimension
    shapeOut.Expand(2);
    shapeA.Expand(2);
    shapeB.Expand(2);
    shapeC.Expand(2);

    const auto device = y.GetCudaDevice();
    const auto M = shapeOut.Rows();
    const auto N = shapeOut.Cols();
    const auto K = shapeA.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto paddedK = a.PaddedHostColSize;

    //! Faster broadcast multiply for Cuda if all tensor dimensions are fixed to
    //! 2
    if (y.TensorShape.Dim() == 2 && a.TensorShape.Dim() == 2 &&
        b.TensorShape.Dim() == 2 && c.TensorShape.Dim() == 2 && y.
        GetBatchSize(2) > 1)
    {
        const auto batchSize = y.GetBatchSize(2);

        if (y.Mode() == DeviceType::Cuda)
        {
            cudaSetDevice(device.GetID());
            Dense::Cuda::GemmMatrixWiseBroadcast(
                y.GetMutableDenseCuda(), a.GetDenseCuda(), b.GetDenseCuda(),
                c.GetDenseCuda(),
                M, N, K, batchSize, a.GetBatchSize(2) == 1,
                b.GetBatchSize(2) == 1,
                c.GetBatchSize(2) == 1, 0);
            return;
        }
    }

    const auto maxDim = std::max({ y.TensorShape.Dim(), a.TensorShape.Dim(),
                                   b.TensorShape.Dim(), c.TensorShape.Dim() });

    //! Treat batch size as part of tensor shape
    shapeOut.Expand(maxDim);
    shapeA.Expand(maxDim);
    shapeB.Expand(maxDim);
    shapeC.Expand(maxDim);

    const auto sizeOut = shapeOut.Size();
    const auto sizeA = shapeA.Size();
    const auto sizeB = shapeB.Size();
    const auto sizeC = shapeC.Size();

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        BroadcastWith3Inputs(shapeOut, shapeA, shapeB, shapeC, sizeOut, sizeA,
                             sizeB, sizeC, y.GetMutableDenseCuda(),
                             a.GetDenseCuda(),
                             b.GetDenseCuda(), c.GetDenseCuda(), 0, 2,
                             Dense::Cuda::Gemm, M, N, K, 0);
    }
    else
    {
        const auto paddedSizeOut = (sizeOut / N) * paddedN;
        const auto paddedSizeA = (sizeA / K) * paddedK;
        const auto paddedSizeB = (sizeB / N) * paddedN;
        const auto paddedSizeC = (sizeC / N) * paddedN;

        BroadcastWith3Inputs(shapeOut, shapeA, shapeB, shapeC, paddedSizeOut,
                             paddedSizeA, paddedSizeB, paddedSizeC,
                             y.GetMutableDenseHost(), a.GetDenseHost(),
                             b.GetDenseHost(),
                             c.GetDenseHost(), 0, 2, Dense::Naive::NaiveGemm, M,
                             N, paddedN, K, paddedK);
    }
}

void Scale(TensorData& y, const TensorData& x, const float factor)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto totalSize = y.TensorShape.Size();
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::Scale(y.GetMutableDenseCuda(), x.GetDenseCuda(), factor,
                           totalSize);
    }
    else
    {
        Dense::Naive::Scale(y.GetMutableDenseHost(), x.GetDenseHost(), factor,
                            totalSizeWithPadding);
    }
}

void Transpose(TensorData& y, const TensorData& x)
{
    const auto device = y.GetCudaDevice();
    const auto inputM = x.Rows();
    const auto inputN = x.Cols();
    const auto paddedM = y.PaddedHostColSize;
    const auto paddedN = x.PaddedHostColSize;
    const auto broadcast = x.GetBatchSize(2) == 1;
    const auto chunkSize = y.TensorShape.Size() / (inputM * inputN);

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::Transpose(y.GetMutableDenseCuda(), x.GetDenseCuda(),
                               inputM, inputN,
                               chunkSize, broadcast);
    }
    else
    {
        Dense::Naive::Transpose(y.GetMutableDenseHost(), x.GetDenseHost(),
                                inputM, paddedM,
                                inputN, paddedN, chunkSize, broadcast);
    }
}

//! Performs y = x^factor for each element
void Pow(TensorData& y, const TensorData& x, const float factor)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto totalSize = y.TensorShape.Size();
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::Pow(y.GetMutableDenseCuda(), x.GetDenseCuda(), factor,
                         totalSize);
    }
    else
    {
        Dense::Naive::Pow(y.GetMutableDenseHost(), x.GetDenseHost(), factor,
                          totalSizeWithPadding);
    }
}

void log(TensorData& y, const TensorData& x)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto totalSize = y.TensorShape.Size();
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::log(y.GetMutableDenseCuda(), x.GetDenseCuda(), totalSize);
    }
    else
    {
        Dense::Naive::log(y.GetMutableDenseHost(), x.GetDenseHost(),
                          totalSizeWithPadding);
    }
}

void log10(TensorData& y, const TensorData& x)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto totalSize = y.TensorShape.Size();
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::log10(y.GetMutableDenseCuda(), x.GetDenseCuda(),
                           totalSize);
    }
    else
    {
        Dense::Naive::log10(y.GetMutableDenseHost(), x.GetDenseHost(),
                            totalSizeWithPadding);
    }
}

void Inverse(TensorData& y, const TensorData& x)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto totalSize = y.TensorShape.Size();
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::Inverse(y.GetMutableDenseCuda(), x.GetDenseCuda(),
                             totalSize);
    }
    else
    {
        Dense::Naive::Inverse(y.GetMutableDenseHost(), x.GetDenseHost(),
                              totalSizeWithPadding);
    }
}

void Mean(TensorData& y, const TensorData& x)
{
    const auto device = y.GetCudaDevice();
    const auto N = y.Cols();
    const auto paddedN = y.PaddedHostColSize;
    const auto unitSize = y.TensorShape.Size();
    const auto totalSize = unitSize;
    const auto totalSizeWithPadding = (totalSize / N) * paddedN;

    if (y.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        Dense::Cuda::Mean(y.GetMutableDenseCuda(), x.GetDenseCuda(), totalSize,
                          unitSize);
    }
    else
    {
        Dense::Naive::Mean(y.GetMutableDenseHost(), x.GetDenseHost(),
                           totalSizeWithPadding,
                           unitSize);
    }
}

void DotBackward(TensorData& da, TensorData& db, const TensorData& dy,
                 const TensorData& a, const TensorData& b)
{
    const auto device = dy.GetCudaDevice();

    const auto maxDim = std::max(
        { dy.TensorShape.Dim(), da.TensorShape.Dim(), db.TensorShape.Dim() });

    auto shapeOut = dy.TensorShape;
    auto shapeA = a.TensorShape;
    auto shapeB = b.TensorShape;

    shapeOut.Expand(maxDim);
    shapeA.Expand(maxDim);
    shapeB.Expand(maxDim);

    const auto sizeOut = shapeOut.Size();
    const auto sizeA = shapeA.Size();
    const auto sizeB = shapeB.Size();

    if (dy.Mode() == DeviceType::Cuda)
    {
        cudaSetDevice(device.GetID());
        BroadcastBackwardWith2Inputs(
            shapeOut, shapeA, shapeB, sizeOut, sizeA, sizeB, dy.GetDenseCuda(),
            da.GetMutableDenseCuda(), db.GetMutableDenseCuda(),
            a.GetDenseCuda(),
            b.GetDenseCuda(), 0,
            0, Dense::Cuda::DotBackward, 0, false, false);
    }
    else
    {
        throw std::runtime_error("Compute::DotBackward - Host not implemented");
    }
}
} // namespace Sapphire::Compute
