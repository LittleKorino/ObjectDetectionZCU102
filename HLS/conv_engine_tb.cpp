/**
 * tb_conv.cpp
 * Testbench for Conv Engine with Golden Reference
 */
#include "conv_engine.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <cstring>

using namespace std;

// Helper to unpack elements from wide words for verification
data_t unpack_element(const wide_t* dram, int idx) {
    wide_t word = dram[idx/16];
    data_t val;
    val.range(15,0) = word.range((idx%16)*16+15, (idx%16)*16);
    return val;
}

// Golden Reference (Slow but accurate fixed-point emulation)
void conv_golden(
    const std::vector<data_t>& input_flat,
    std::vector<data_t>& output_flat,
    const std::vector<data_t>& weight_flat,
    const std::vector<data_t>& bn_params,
    int in_channels, int out_channels,
    int in_height, int in_width,
    int kernel_size, int stride, int padding,
    int out_height, int out_width,
    int use_leaky
) {
    for (int oc = 0; oc < out_channels; oc++) {
        data_t scale = bn_params[oc*2];
        data_t bias  = bn_params[oc*2+1];

        for (int oh = 0; oh < out_height; oh++) {
            for (int ow = 0; ow < out_width; ow++) {
                
                acc_t sum = 0;
                int h_start = oh * stride - padding;
                int w_start = ow * stride - padding;

                for (int ic = 0; ic < in_channels; ic++) {
                    for (int ky = 0; ky < kernel_size; ky++) {
                        for (int kx = 0; kx < kernel_size; kx++) {
                            int ih = h_start + ky;
                            int iw = w_start + kx;

                            if (ih >= 0 && ih < in_height && iw >= 0 && iw < in_width) {
                                int in_idx = (ic * in_height + ih) * in_width + iw;
                                int wt_idx = ((oc * in_channels + ic) * kernel_size + ky) * kernel_size + kx;
                                sum += input_flat[in_idx] * weight_flat[wt_idx];
                            }
                        }
                    }
                }
                
                // BN + Activation (match HW: truncate to data_t before activate)
                data_t bn_val = (data_t)(sum * scale + bias);
                data_t result;
                if (use_leaky < 0) {
                    result = bn_val;
                } else if (bn_val >= 0) {
                    result = bn_val;
                } else {
                    if (use_leaky > 0) {
                        acc_t tmp = bn_val;
                        result = (data_t)((tmp * 13) >> 7);
                    } else {
                        result = 0;
                    }
                }
                
                output_flat[(oc * out_height + oh) * out_width + ow] = result;
            }
        }
    }
}

// Golden reference for max-pool (2x2 stride 2, matches HW)
void pool_golden(
    const std::vector<data_t>& input_flat,
    std::vector<data_t>& output_flat,
    int channels, int in_h, int in_w
) {
    int out_h = in_h / 2;
    int out_w = in_w / 2;
    for (int c = 0; c < channels; c++) {
        for (int oh = 0; oh < out_h; oh++) {
            for (int ow = 0; ow < out_w; ow++) {
                data_t v0 = input_flat[(c * in_h + oh*2  ) * in_w + ow*2  ];
                data_t v1 = input_flat[(c * in_h + oh*2+1) * in_w + ow*2  ];
                data_t v2 = input_flat[(c * in_h + oh*2  ) * in_w + ow*2+1];
                data_t v3 = input_flat[(c * in_h + oh*2+1) * in_w + ow*2+1];
                data_t mx01 = (v0 > v1) ? v0 : v1;
                data_t mx23 = (v2 > v3) ? v2 : v3;
                output_flat[(c * out_h + oh) * out_w + ow] = (mx01 > mx23) ? mx01 : mx23;
            }
        }
    }
}

// Run a single test configuration. Returns 0 on pass, 1 on fail.
int run_test(const char* name,
             int IC, int OC, int H, int W, int K, int S, int P,
             int use_pool, int pool_stride, int use_leaky)
{
    int OH = (H + 2*P - K)/S + 1;
    int OW = (W + 2*P - K)/S + 1;
    int final_h = use_pool ? OH / 2 : OH;
    int final_w = use_pool ? OW / 2 : OW;

    printf("\n===== Test: %s =====\n", name);
    printf("  IC=%d OC=%d H=%d W=%d K=%d S=%d P=%d pool=%d leaky=%d\n",
           IC, OC, H, W, K, S, P, use_pool, use_leaky);
    printf("  Conv output: %dx%d  Final output: %dx%d\n", OH, OW, final_h, final_w);

    // Allocate DRAM arrays (wide words + generous padding)
    int in_size_words  = (IC * H * W) / 16 + 256;
    int wt_size_words  = (OC * IC * K * K) / 16 + 256;
    int out_size_words = (OC * final_h * final_w) / 16 + 256;

    std::vector<wide_t> input_dram(in_size_words, 0);
    std::vector<wide_t> weights_dram(wt_size_words, 0);
    std::vector<wide_t> output_dram(out_size_words, 0);
    std::vector<data_t> bn_dram(OC * 2 + 64, 0);

    // Flat buffers for golden reference
    std::vector<data_t> input_flat(IC * H * W);
    std::vector<data_t> weight_flat(OC * IC * K * K);
    std::vector<data_t> conv_out_flat(OC * OH * OW);
    std::vector<data_t> golden_flat(OC * final_h * final_w);

    // Initialize inputs (deterministic pattern)
    for (int i = 0; i < IC * H * W; i++) {
        data_t val = (float)(i % 100) / 100.0f;
        input_flat[i] = val;
        int word_idx = i / 16;
        int sub_idx  = i % 16;
        input_dram[word_idx].range(sub_idx*16+15, sub_idx*16) = val.range(15, 0);
    }

    // Initialize weights
    for (int i = 0; i < OC * IC * K * K; i++) {
        data_t val = (float)((i % 7) - 3) / 10.0f; // range -0.3 to +0.3
        weight_flat[i] = val;
        int word_idx = i / 16;
        int sub_idx  = i % 16;
        weights_dram[word_idx].range(sub_idx*16+15, sub_idx*16) = val.range(15, 0);
    }

    // Initialize BN params
    for (int i = 0; i < OC; i++) {
        bn_dram[i*2]     = 1.0; // Scale
        bn_dram[i*2 + 1] = 0.5; // Bias
    }

    // Run hardware
    conv_engine(input_dram.data(), output_dram.data(), weights_dram.data(), bn_dram.data(),
                IC, OC, H, W, K, S, P, use_pool, pool_stride, use_leaky);

    // Run golden reference
    conv_golden(input_flat, conv_out_flat, weight_flat, bn_dram,
                IC, OC, H, W, K, S, P, OH, OW, use_leaky);

    if (use_pool) {
        pool_golden(conv_out_flat, golden_flat, OC, OH, OW);
    } else {
        golden_flat = conv_out_flat;
    }

    // Compare
    int err_count = 0;
    float max_err = 0.0f;
    int total_elements = OC * final_h * final_w;
    for (int i = 0; i < total_elements; i++) {
        data_t hw_val = unpack_element(output_dram.data(), i);
        data_t sw_val = golden_flat[i];
        float diff = std::abs((float)hw_val - (float)sw_val);
        if (diff > 0.05f) {
            if (err_count < 10) {
                int oc = i / (final_h * final_w);
                int rem = i % (final_h * final_w);
                int oh = rem / final_w;
                int ow = rem % final_w;
                printf("  Error @ flat=%d (oc=%d h=%d w=%d): HW=%f SW=%f diff=%f\n",
                       i, oc, oh, ow, (float)hw_val, (float)sw_val, diff);
            }
            err_count++;
        }
        if (diff > max_err) max_err = diff;
    }

    if (err_count == 0) {
        printf("  PASSED! (%d elements, max_err=%.6f)\n", total_elements, max_err);
        return 0;
    } else {
        printf("  FAILED! %d/%d errors, max_err=%.6f\n", err_count, total_elements, max_err);
        return 1;
    }
}

int main() {
    std::cout << "Starting Conv Engine Testbench..." << std::endl;
    int failures = 0;

    // Test 1: Perfectly aligned (original test)
    //   OH=OW=16, 16%16=0, single tile in all dims
    failures += run_test("Aligned 16x16 IC=3 OC=16",
                         3, 16, 16, 16, 3, 1, 1, 0, 0, 0);

    // Test 2: Non-aligned width (single tile, partial fill)
    //   OH=OW=13, 13%16!=0, verifies RMW write logic
    failures += run_test("Non-aligned 13x13 IC=3 OC=16",
                         3, 16, 13, 13, 3, 1, 1, 0, 0, 0);

    // Test 3: Multi-tile with non-aligned dims
    //   H=W=26 → OH=OW=26, needs 2 tiles in H and W (16+10)
    //   OC=32 → 2 OC tiles. Cross-tile boundary + partial tiles.
    failures += run_test("Multi-tile 26x26 IC=3 OC=32",
                         3, 32, 26, 26, 3, 1, 1, 0, 0, 0);

    // Test 4: Pooled output (aligned conv, aligned pool)
    //   OH=OW=16 → pooled 8×8. 8 < 16 so single word per row.
    failures += run_test("Pooled 16x16 IC=3 OC=16",
                         3, 16, 16, 16, 3, 1, 1, 1, 2, 0);

    // Test 5: Pooled with non-aligned pooled output
    //   H=W=26 → OH=OW=26 → pooled 13×13, 13%16!=0
    failures += run_test("Pooled non-aligned 26x26 IC=3 OC=16",
                         3, 16, 26, 26, 3, 1, 1, 1, 2, 0);

    // Test 6: LeakyReLU activation
    failures += run_test("LeakyReLU 16x16 IC=3 OC=16",
                         3, 16, 16, 16, 3, 1, 1, 0, 0, 1);

    printf("\n===== SUMMARY =====\n");
    if (failures == 0) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("%d test(s) FAILED!\n", failures);
        return 1;
    }
}