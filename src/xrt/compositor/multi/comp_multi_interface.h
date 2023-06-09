// Copyright 2021, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Interface for the multi-client layer code.
 * @author Jakob Bornecrantz <jakob@collabora.com>
 * @ingroup comp_main
 */

#pragma once

#include "xrt/xrt_compositor.h"


#ifdef __cplusplus
extern "C" {
#endif

struct u_pacing_app_factory;


/*!
 * Create a system compositor that can handle multiple clients and that drives
 * a single native compositor. Both the native compositor and the pacing factory
 * is owned by the multi compositor and destroyed by it.
 */
xrt_result_t
comp_multi_create_system_compositor(struct xrt_compositor_native *xcn,
                                    struct u_pacing_app_factory *upaf,
                                    const struct xrt_system_compositor_info *xsci,
                                    struct xrt_system_compositor **out_xsysc);


#ifdef __cplusplus
}
#endif
