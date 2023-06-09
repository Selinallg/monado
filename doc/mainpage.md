# Monado

<!--
Copyright 2018-2021, Collabora, Ltd. and the Monado contributors
SPDX-License-Identifier: BSL-1.0
-->

This documentation is intended for developers wanting to dive into the code of
Monado. And assumes that you have read [README.md][], the file also holds
getting started information and general documentation.

## Useful pages

* @ref CHANGELOG - If this is the web version of the docs, the changelog also
  includes a section for changes that have not yet been in a tagged release.
* @ref conventions
* @ref understanding-targets - How all the pieces (`xrt_instance`, IPC, OpenXR)
  fit together.
* @ref vulkan-extensions
* @ref writing-driver
* @ref ipc-design
* @ref frame-pacing
* @ref tracing
* @ref implementing-extension
* @ref howto-remote-driver

## Source layout

* src/xrt/include - @ref xrt_iface defines the internal interfaces of Monado.
* src/xrt/drivers - Hardware @ref drv.
* src/xrt/compositor - @ref comp code for doing distortion and driving the
  display hardware of a device.
* src/xrt/state_trackers/oxr - @ref oxr, implements the OpenXR API.
* src/xrt/state_trackers/gui - @ref gui, various helper and debug GUI code.
* src/xrt/auxiliary - @ref aux and other larger components, like @ref
  aux_tracking and @ref aux_math.
* src/xrt/targets - glue code and build logic to produce final binaries.
* src/external - a small collection of external code and headers.

[README.md]: https://gitlab.freedesktop.org/monado/monado
