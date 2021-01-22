// SPDX-License-Identifier: GPL-2.0

/*
 * Input driver for potentiometer connected on ADC
 *
 * Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 */

#include <linux/err.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/input.h>
#include <linux/input-polldev.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>

struct adc_pot_host {
	struct iio_channel *channel;
	int range;
	int resolution;
	u32 last_raw;
	u32 keycode;
	int scaled;
};

static void adc_pot_poll(struct input_polled_dev *dev)
{
	struct adc_pot_host *host = dev->private;
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
		input_report_key(dev->input, host->keycode, 1);
		input_sync(dev->input);
		input_report_key(dev->input, host->keycode, 0);
		input_sync(dev->input);
		host->last_raw = raw;
	}
}


static ssize_t scaled_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct adc_pot_host *host = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", host->scaled);
	return 0;
}

static DEVICE_ATTR_RO(scaled);

static struct attribute *adc_pot_attrs[] = {
	&dev_attr_scaled.attr,
	NULL,
};

static struct attribute_group adc_pot_attr_group = {
	.attrs = adc_pot_attrs,
};

static int adc_pot_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct adc_pot_host *host;
	struct input_polled_dev *poll_dev;
	struct input_dev *input;
	enum iio_chan_type type;
	int value;
	int error;

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
	if (device_property_read_u32(dev, "linux,code", &host->keycode)) {
		dev_warn(dev, "Key with invalid or missing linux,code\n");
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

	poll_dev->poll = adc_pot_poll;
	poll_dev->private = host;

	input = poll_dev->input;

	input->name = pdev->name;
	input->phys = "adc-pot/input0";

	input->id.bustype = BUS_HOST;
	input->id.vendor = 0x0001;
	input->id.product = 0x0001;
	input->id.version = 0x0100;

	__set_bit(EV_KEY, input->evbit);
	__set_bit(host->keycode, input->keybit);

	error = sysfs_create_group(&pdev->dev.kobj, &adc_pot_attr_group);
	if (error) {
		dev_err(dev, "Unable to export raw/scaled, error: %d\n", error);
		return error;
	}

	error = input_register_polled_device(poll_dev);
	if (error) {
		dev_err(dev, "Unable to register input device: %d\n", error);
		return error;
	}

	return 0;
}

static int adc_pot_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &adc_pot_attr_group);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id adc_pot_of_match[] = {
	{ .compatible = "adc-pot", },
	{ }
};
MODULE_DEVICE_TABLE(of, adc_pot_of_match);
#endif

static struct platform_driver __refdata adc_pot_driver = {
	.driver = {
		.name = "adc_pot",
		.of_match_table = of_match_ptr(adc_pot_of_match),
	},
	.probe = adc_pot_probe,
	.remove = adc_pot_remove,
};
module_platform_driver(adc_pot_driver);

MODULE_AUTHOR("Ziyuan Xu <xzy.xu@rock-chips.com>");
MODULE_DESCRIPTION("Input driver for potentiometer connected on ADC");
MODULE_LICENSE("GPL v2");
