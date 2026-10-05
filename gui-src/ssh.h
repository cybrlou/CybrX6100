/*
 * SSH_START
 * CybrX6100 SSH preference + service control (APP page toggle).
 * SSH_END
 */
#pragma once

#include <stdbool.h>
#include "buttons.h"

#ifdef __cplusplus
extern "C" {
#endif

void ssh_setup(void);
void ssh_toggle_cb(button_data_t *btn_data);
const char *ssh_label_getter(void);
bool ssh_is_enabled(void);

#ifdef __cplusplus
}
#endif
