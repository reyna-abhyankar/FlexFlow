/* Copyright 2023 CMU, Facebook, LANL, MIT, NVIDIA, and Stanford (alphabetical)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kernels/flat_kernels.h"
#include "kernels/hip_helper.h"
#include <hip/hip_runtime.h>

namespace FlexFlow {

namespace Kernels {
namespace Flat {

void forward_kernel(hipStream_t stream,
                    float const *input_ptr,
                    float *output_ptr,
                    size_t num_elements) {

  checkCUDA(hipMemcpyAsync(output_ptr,
                           input_ptr,
                           num_elements * sizeof(float),
                           hipMemcpyDeviceToDevice,
                           stream));
  // checkCUDA(hipDeviceSynchronize());
}

void backward_kernel(hipStream_t stream,
                     float *input_grad_ptr,
                     float const *output_grad_ptr,
                     size_t num_elements) {

  float alpha = 1.0f;
  hipLaunchKernelGGL(HIP_KERNEL_NAME(apply_add_with_scale<float>),
                     GET_BLOCKS(num_elements),
                     CUDA_NUM_THREADS,
                     0,
                     stream,
                     input_grad_ptr,
                     output_grad_ptr,
                     num_elements,
                     alpha);
  // checkCUDA(hipMemcpyAsync(acc_input_grad.ptr, acc_output_grad.ptr,
  //                           acc_input_grad.rect.volume() * sizeof(float),
  //                           hipMemcpyDeviceToDevice));
  // checkCUDA(hipDeviceSynchronize());
}

} // namespace Flat
} // namespace Kernels
} // namespace FlexFlow