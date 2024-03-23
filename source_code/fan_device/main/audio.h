#ifndef AUDIO_H
#define AUDIO_H

#include "driver/i2s_std.h"
#define AUDIO_BUFFER 2048

extern i2s_chan_handle_t tx_handle;

esp_err_t i2s_setup(void);

#endif /* AUDIO_H */
