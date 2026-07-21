#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(soil, LOG_LEVEL_INF);

/* Board must define an "led0" alias in its devicetree. */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("LED device not ready");
		return 0;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	LOG_INF("soil-monitoring up, blinking led0");

	while (1) {
		gpio_pin_toggle_dt(&led);
		k_msleep(1000);
	}

	return 0;
}
