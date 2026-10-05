/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * CybrX6100 V.01 — tiny REST control server for the X6100 GUI.
 * Started next to the LAN CAT service. Does not touch CAT frequency,
 * mode, filter, power, PTT, or meters.
 */
#pragma once

int cybr_http_init(void);
void cybr_http_destruct(void);
