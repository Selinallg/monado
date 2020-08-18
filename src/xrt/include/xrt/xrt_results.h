// Copyright 2020, Collabora, Ltd.
// SPDX-License-Identifier: BSL-1.0
/*!
 * @file
 * @brief  Internal result type for XRT.
 * @author Christoph Haag <christoph.haag@collabora.com>
 * @ingroup xrt_iface
 */

#pragma once

typedef enum xrt_result
{
	XRT_SUCCESS = 0,
	XRT_ERROR_IPC_FAILURE = -1,
	XRT_ERROR_NO_IMAGE_AVAILABLE = -2,
	XRT_ERROR_VULKAN = -3,
	XRT_ERROR_OPENGL = -4,
	XRT_ERROR_FAILED_TO_SUBMIT_VULKAN_COMMANDS = -5,
	/*!
	 *
	 * Returned when a swapchain create flag is passed that is valid, but
	 * not supported by the main compositor (and lack of support is also
	 * valid).
	 *
	 * For use when e.g. the protected content image flag is requested but
	 * isn't supported.
	 */
	XRT_ERROR_SWAPCHAIN_FLAG_VALID_BUT_UNSUPPORTED = -6,
	/*!
	 * Could not allocate native image buffer(s).
	 */
	XRT_ERROR_ALLOCATION = -7,
	/*
	 * The pose is no longer active, this happens when the application
	 * tries to access a pose that is no longer active.
	 */
	XRT_ERROR_POSE_NOT_ACTIVE = -8,
} xrt_result_t;
