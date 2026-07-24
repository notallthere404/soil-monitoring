#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "cap.h"

LOG_MODULE_REGISTER(soil, LOG_LEVEL_INF);

int main(void) {
    int err = cap_init();
    if (err != 0) {
        return 1;
    }

    cap_read();

    return 0;
}
