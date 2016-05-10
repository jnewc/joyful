/*
 *  hello-1.c - The simplest kernel module.
 */

#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>

#include <linux/gpio.h>
#include <linux/interrupt.h>

#include <asm/irq.h>
#include <asm/io.h>
#include <asm/bitops.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jack Newcombe");
MODULE_DESCRIPTION("Intercepts a button press on GPIO Pin 18");
MODULE_VERSION("0.1");

static unsigned int irqNumber;

static const int gpio_pin_map[] = {
	17,		// A Button
	18,		// B Button
	22,		// X Button
	23,		// Y Button
	24,		// C Button
	27		// Z Button
};

static const int gpio_pin_map_count = (sizeof(gpio_pin_map) / sizeof(int));

static int i2c_file;

struct pad_state {
	unsigned int gpio_state;
	signed int primary_x;
	signed int primary_y;
}

struct pad_data {
	struct input_dev *device;
	struct pad_state *pad_state;
}

static int read_bytes_i2c(u8 register)
{
	return i2c_smbus_read_byte_data(i2c_file, register);
}

static int setup_i2c()
{
	char filename[20] = "/dev/i2c-1";

	i2c_file = open(filename, O_RDWR);
	int addr = 0x48; /* The I2C address */

	if (ioctl(file, I2C_SLAVE, addr) < 0) {
    	/* ERROR HANDLING; you can check errno to see what went wrong */
    	return 1;
	}

	return 0;
}

static int setup_gpios()
{
	int result;

	for(int i = 0; i < gpio_pin_map_count; i++) {
		int gpio_pin = gpio_pin_map[i];
		result = gpio_request(gpio_pin, "sysfs");

		if (result < 0) {
			printk(KERN_ALERT "error %d: could not request gpio: %d\n", result, gpio_pin);
			return result;
		}

		gpio_direction_input(gpio_pin);
		gpio_export(gpio_pin, false);
	}

	return result;
}

static void setup_device_axis(struct input_dev* device, unsigned int axis)
{
	input_set_abs_params(device, axis, -127, 128, 0, 0);
}

static int setup_device(struct pad_data* pad_data)
{

	// Create the device
	pad_data->device = input_allocate_device();
	if (!pad_data->device) {
		printk(KERN_ERR "enough memory\n");
		error = -ENOMEM;
		return -1;
	}

	// Setup device description
	pad_data->device->name = "PadPadPad";
	pad_data->device->phys = "PadPadPad_phys";
	pad_data->device->uniq = "PadPadPad_uniq";

	pad_data->device->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

	// Setup available buttons
	__set_bit(BTN_A, pad_data->device->keybit);		// A Button (1, 1)
	__set_bit(BTN_B, pad_data->device->keybit);		// B Button (2, 1)
	__set_bit(BTN_X, pad_data->device->keybit);		// X Button (1, 2)
	__set_bit(BTN_Y, pad_data->device->keybit);		// Y Button (2, 2)
	__set_bit(BTN_C, pad_data->device->keybit);		// C Button (1, 3)
	__set_bit(BTN_Z, pad_data->device->keybit);		// Z Button (2, 3)

	// Setup analog sticks
	setup_device_axis(pad_data->device, ABS_X);			// Primary X
	setup_device_axis(pad_data->device, ABS_Y);			// Primary Y

	return 0;
}

static int __init button_init(void)
{
	struct pad_data *pad_data;

	printk(KERN_INFO "Loading gamepad module ...\n");

	// Allocate memory to store our pad data
	pad_data = kzalloc(sizeof (struct pad_data), GFP_KERNEL);

	setup_gpios();
	setup_i2c();

	setup_device(pad_data);



	error = input_register_device(pad_data->device);
	if (error) {
		printk(KERN_ERR "button.c: Failed to register device\n");
		goto err_free_dev;
	}

	return 0;

 err_free_dev:
	input_free_device(pad_data->device);

}

static void __exit button_exit(void)
{
    input_unregister_device(pad_data->device);

	free_irq(irqNumber, NULL);

	gpio_unexport(gpioPin);
	gpio_free(gpioPin);
}

module_init(button_init);
module_exit(button_exit);
