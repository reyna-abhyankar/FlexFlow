#include "doctest/doctest.h"
#include "kernels/reshape_kernels.h"
#include "test_utils.h"

using namespace ::FlexFlow;
TEST_SUITE(FF_TEST_SUITE) {
  TEST_CASE("Test Reshape Forward and Backward") {
    const std::size_t num_elements = 100;
    // ArrayShape shape = ArrayShape{
    //     std::vector<size_t>{num_elements},
    // };

    TensorShape shape = get_float_tensor_shape({num_elements});

    cudaStream_t stream;
    checkCUDA(cudaStreamCreate(&stream));

    Allocator allocator = get_local_memory_allocator();

    SUBCASE("Test Reshape Forward") {
      GenericTensorAccessorR input_accessor =
          makeReadOnlyAccessor(getFilledAccessorW(shape, allocator, 1.0f));
      GenericTensorAccessorW output_accessor = allocator.allocate_tensor(shape);

      ReshapePerDeviceState state =
          Kernels::Reshape::init_kernel(DataType::FLOAT);

      Kernels::Reshape::forward_kernel(
          stream, state, input_accessor, output_accessor);

      std::vector<float> check_output_data =
          fill_host_data<float>(output_accessor.ptr, num_elements);

      for (std::size_t i = 0; i < num_elements; ++i) {
        REQUIRE(1.0f == check_output_data[i]);
      }

      SUBCASE("Test Reshape Kernel Backward") {
        GenericTensorAccessorR grad_accessor =
            makeReadOnlyAccessor(getFilledAccessorW(shape, allocator, 1.0f));

        ReshapePerDeviceState state =
            Kernels::Reshape::init_kernel(DataType::FLOAT);

        Kernels::Reshape::backward_kernel(
            stream, state, output_accessor, grad_accessor);

        std::vector<float> host_grad_input_data =
            fill_host_data<float>(output_accessor.ptr, num_elements);

        for (std::size_t i = 0; i < num_elements; ++i) {
          CHECK(host_grad_input_data[i] == 2.0f);
        }
      }
    }

    checkCUDA(cudaStreamDestroy(stream));
  }
}
