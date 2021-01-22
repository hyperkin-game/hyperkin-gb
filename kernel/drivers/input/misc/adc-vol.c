/*
 * Input driver for potentiometer connected on ADC
 *
 * Copyright (c) 2020 BTE Co. Ltd.
 */

#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>

struct adc_vol_host {
	struct iio_channel *channel;
	int range;
	int resolution;
	u32 last_raw;
	u32 volup;
	u32 voldown;
	int scaled;

	int ratio_gpio;
	u32 ratio_state;
	u32 ratio_code;
};

static void adc_vol_poll(struct input_polled_dev *dev)
{
	struct adc_vol_host *host = dev->private;
	u32 raw, val;

	if (unlikely(iio_read_channel_raw(host->channel, &raw) < 0))
		return ;
	/*
	 * Usually, the raw data of the adc channel is not a fixed value even
	 * though the potentiometer doesn't change. It makes sense that we
	 * check the difference with the previous raw data.
	 */
	if (abs(host->last_raw - raw) > (host->resolution / host->range)) {
		val = raw * host->range / host->resolution;
		host->scaled = val;
		if(raw > 1000){
			input_report_key(dev->input, host->volup, 0);
			input_report_key(dev->input, host->voldown, 0);
		}else if(raw > 100){
			input_report_key(dev->input, host->voldown, 1);
		}else{
			input_report_key(dev->input, host->volup, 1);
		}
		input_sync(dev->input);
		host->last_raw = raw;
	}
}


static ssize_t scaled_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adc_vol_host *host = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", host->scaled);
	return 0;
}

static ssize_t ratio_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adc_vol_host *host = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", gpio_get_value(host->ratio_gpio));
	return 0;
}

#include <linux/mm.h>
static ssize_t mem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//2MB is TEE size
	return sprintf(buf, "%ld\n", (get_num_physpages() >> (20 - PAGE_SHIFT)) + 2);
}
static DEVICE_ATTR_RO(scaled);
static DEVICE_ATTR_RO(ratio);
static DEVICE_ATTR_RO(mem);

static struct attribute *adc_vol_attrs[] = {
	&dev_attr_scaled.attr,
	&dev_attr_ratio.attr,
	&dev_attr_mem.attr,
	NULL,
};

static struct attribute_group adc_vol_attr_group = {
	.attrs = adc_vol_attrs,
};

static int adc_vol_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct adc_vol_host *host;
	struct input_polled_dev *poll_dev;
	struct input_dev *input;
	enum iio_chan_type type;
	int value;
	int error, err, ret, host_drv_gpio;
	struct device_node *np = pdev->dev.of_node;

	host = devm_kzalloc(dev, sizeof(*host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;

	host->channel = devm_iio_channel_get(dev, "variant");
	if (IS_ERR(host->channel))
		return PTR_ERR(host->channel);

	if (!host->channel->indio_dev)
		return -ENXIO;

	error = iio_get_channel_type(host->channel, &type);
	if (error < 0)
		return error;

	if (type != IIO_VOLTAGE) {
		dev_err(dev, "Incompatible channel type %d\n", type);
		return -EINVAL;
	}

	if (device_property_read_u32(dev, "resolution", &value)) {
		dev_warn(dev, "Missing resolution, default 10\n");
		value = 10;
	}
	host->resolution = GENMASK(value - 1, 0);
	if (device_property_read_u32(dev, "range", &host->range)) {
		dev_warn(dev, "Missing range, default 100\n");
		host->range = 100;
	}
	if (device_property_read_u32(dev, "linux,volup", &host->volup)) {
		dev_warn(dev, "Key with invalid or missing linux,volup\n");
		return -EINVAL;
	}
	if (device_property_read_u32(dev, "linux,voldown", &host->voldown)) {
		dev_warn(dev, "Key with invalid or missing linux,voldown\n");
		return -EINVAL;
	}

	platform_set_drvdata(pdev, host);

	poll_dev = devm_input_allocate_polled_device(dev);
	if (!poll_dev) {
		dev_err(dev, "failed to allocate input device\n");
		return -ENOMEM;
	}

	if (!device_property_read_u32(dev, "poll-interval", &value))
		poll_dev->poll_interval = value;

	poll_dev->poll = adc_vol_poll;
	poll_dev->private = host;

	input = poll_dev->input;

	input->name = pdev->name;
	input->phys = "adc-vol/input0";

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	__set_bit(EV_KEY, input->evbit);
	input_set_capability(input, EV_KEY, host->volup);
	input_set_capability(input, EV_KEY, host->voldown);

	error = sysfs_create_group(&pdev->dev.kobj, &adc_vol_attr_group);
	if (error) {
		dev_err(dev, "Unable to export raw/scaled, error: %d\n", error);
		return error;
	}

	error = input_register_polled_device(poll_dev);
	if (error) {
		dev_err(dev, "Unable to register input device: %d\n", error);
		return error;
	}

	//Enable USB Host 2.0
	host_drv_gpio = of_get_named_gpio(np, "host_drv_gpio", 0);
	if(!gpio_is_valid(host_drv_gpio)){
		dev_err(&pdev->dev, "invalid host gpio%d\n", host_drv_gpio);
	} else {
		err = devm_gpio_request(&pdev->dev, host_drv_gpio, "host_drv_gpio");
		if (err) {
			dev_err(&pdev->dev,
				"failed to request GPIO%d for host_drv\n",
				host_drv_gpio);
			ret = err;
			return 0;
		}
		gpio_direction_output(host_drv_gpio, 1);
	}

	//Enable GPIO9 PB5
	host->ratio_gpio = of_get_named_gpio(np, "ratio_gpio", 0);
	if(!gpio_is_valid(host->ratio_gpio)){
		dev_err(&pdev->dev, "invalid ratio gpiod%d\n", host->ratio_gpio);
	}else{
		err = devm_gpio_request(&pdev->dev, host->ratio_gpio, "ratio_gpio");
		if (err) {
			dev_err(&pdev->dev, "failed to request GPIO%d for ratio_gpio\n",  host->ratio_gpio);
			ret = err;
			return 0;
		}
		err = gpio_direction_input(host->ratio_gpio);
		if (err) {
			dev_err(&pdev->dev, "failed to configure input direction for GPIO%d, error %d\n", host->ratio_gpio, err);
			ret = err;
			return 0;
		}
		//input_set_capability(input, EV_KEY, KEY_F);
		host->ratio_state = -1;
	} 

	return 0;
}

static int adc_vol_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &adc_vol_attr_group);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id adc_vol_of_match[] = {
	{ .compatible = "adc-vol", },
	{ }
};
MODULE_DEVICE_TABLE(of, adc_vol_of_match);
#endif

static struct platform_driver __refdata adc_vol_driver = {
	.driver = {
		.name = "adc_vol",
		.of_match_table = of_match_ptr(adc_vol_of_match),
	},
	.probe = adc_vol_probe,
	.remove = adc_vol_remove,
};
module_platform_driver(adc_vol_driver);

MODULE_AUTHOR("Stanley Ho <stanley@bte.com.tw>");
MODULE_DESCRIPTION("Input driver for potentiometer connected on ADC");
MODULE_LICENSE("GPL v2");
