// SPDX-License-Identifier: GPL-2.0-only
/*
 * vcnl4000.c - Support for Vishay VCNL4000/4010/4020/4040/4200 combined ambient
 * light and proximity sensor
 *
 * Copyright 2012 Peter Meerwald <pmeerw@pmeerw.net>
 * Copyright 2019 Pursim SPC
 *
 * IIO driver for:
 *   VCNL4000/10/20 (7-bit I2C slave address 0x13)
 *   VCNL4040 (7-bit I2C slave address 0x60)
 *   VCNL4200 (7-bit I2C slave address 0x51)
 *
 * TODO:
 *   allow to adjust IR current
 *   proximity threshold and event handling
 *   periodic ALS/proximity measurement (VCNL4010/20)
 *   interrupts (VCNL4010/20/40, VCNL4200)
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>

#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>

#define VCNL4000_DRV_NAME "vcnl4000"
#define VCNL4000_PROD_ID	0x01
#define VCNL4010_PROD_ID	0x02 /* for VCNL4020, VCNL4010 */
#define VCNL4040_PROD_ID	0x86
#define VCNL4200_PROD_ID	0x58

#define VCNL4000_COMMAND	0x80 /* Command register */
#define VCNL4000_PROD_REV	0x81 /* Product ID and Revision ID */
#define VCNL4000_LED_CURRENT	0x83 /* IR LED current for proximity mode */
#define VCNL4000_AL_PARAM	0x84 /* Ambient light parameter register */
#define VCNL4000_AL_RESULT_HI	0x85 /* Ambient light result register, MSB */
#define VCNL4000_AL_RESULT_LO	0x86 /* Ambient light result register, LSB */
#define VCNL4000_PS_RESULT_HI	0x87 /* Proximity result register, MSB */
#define VCNL4000_PS_RESULT_LO	0x88 /* Proximity result register, LSB */
#define VCNL4000_PS_MEAS_FREQ	0x89 /* Proximity test signal frequency */
#define VCNL4000_PS_MOD_ADJ	0x8a /* Proximity modulator timing adjustment */

#define VCNL4200_AL_CONF	0x00 /* Ambient light configuration */
#define VCNL4200_PS_CONF1	0x03 /* Proximity configuration */
#define VCNL4200_PS_DATA	0x08 /* Proximity data */
#define VCNL4200_AL_DATA	0x09 /* Ambient light data */
#define VCNL4200_DEV_ID		0x0e /* Device ID, slave address and version */

#define VCNL4040_DEV_ID		0x0c /* Device ID and version */

/* Bit masks for COMMAND register */
#define VCNL4000_AL_RDY		BIT(6) /* ALS data ready? */
#define VCNL4000_PS_RDY		BIT(5) /* proximity data ready? */
#define VCNL4000_AL_OD		BIT(4) /* start on-demand ALS measurement */
#define VCNL4000_PS_OD		BIT(3) /* start on-demand proximity measurement */

#define VCNL4000_SLEEP_DELAY_MS	2000 /* before we enter pm_runtime_suspend */

enum vcnl4000_device_ids {
	VCNL4000,
	VCNL4010,
	VCNL4040,
	VCNL4200,
};

struct vcnl4200_channel {
	u8 reg;
	ktime_t last_measurement;
	ktime_t sampling_rate;
	struct mutex lock;
};

struct vcnl4000_data {
	struct i2c_client *client;
	enum vcnl4000_device_ids id;
	int rev;
	int al_scale;
	const struct vcnl4000_chip_spec *chip_spec;
	struct mutex vcnl4000_lock;
	struct vcnl4200_channel vcnl4200_al;
	struct vcnl4200_channel vcnl4200_ps;
	uint32_t near_level;
};

struct vcnl4000_chip_spec {
	const char *prod;
	int (*init)(struct vcnl4000_data *data);
	int (*measure_light)(struct vcnl4000_data *data, int *val);
	int (*measure_proximity)(struct vcnl4000_data *data, int *val);
	int (*set_power_state)(struct vcnl4000_data *data, bool on);
};

static const struct i2c_device_id vcnl4000_id[] = {
	{ "vcnl4000", VCNL4000 },
	{ "vcnl4010", VCNL4010 },
	{ "vcnl4020", VCNL4010 },
	{ "vcnl4040", VCNL4040 },
	{ "vcnl4200", VCNL4200 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, vcnl4000_id);

static int vcnl4000_set_power_state(struct vcnl4000_data *data, bool on)
{
	/* no suspend op */
	return 0;
}

static int vcnl4000_init(struct vcnl4000_data *data)
{
	int ret, prod_id;

	ret = i2c_smbus_read_byte_data(data->client, VCNL4000_PROD_REV);
	if (ret < 0)
		return ret;

	prod_id = ret >> 4;
	switch (prod_id) {
	case VCNL4000_PROD_ID:
		if (data->id != VCNL4000)
			dev_warn(&data->client->dev,
					"wrong device id, use vcnl4000");
		break;
	case VCNL4010_PROD_ID:
		if (data->id != VCNL4010)
			dev_warn(&data->client->dev,
					"wrong device id, use vcnl4010/4020");
		break;
	default:
		return -ENODEV;
	}

	data->rev = ret & 0xf;
	data->al_scale = 250000;
	mutex_init(&data->vcnl4000_lock);

	return data->chip_spec->set_power_state(data, true);
};

static int vcnl4200_set_power_state(struct vcnl4000_data *data, bool on)
{
	u16 val = on ? 0 /* power on */ : 1 /* shut down */;
	int ret;

	ret = i2c_smbus_write_word_data(data->client, VCNL4200_AL_CONF, val);
	if (ret < 0)
		return ret;

	ret = i2c_smbus_write_word_data(data->client, VCNL4200_PS_CONF1, val);
	if (ret < 0)
		return ret;

	if (on) {
		/* Wait at least one integration cycle before fetching data */
		data->vcnl4200_al.last_measurement = ktime_get();
		data->vcnl4200_ps.last_measurement = ktime_get();
	}

	return 0;
}

static int vcnl4200_init(struct vcnl4000_data *data)
{
	int ret, id;

	ret = i2c_smbus_read_word_data(data->client, VCNL4200_DEV_ID);
	if (ret < 0)
		return ret;

	id = ret & 0xff;

	if (id != VCNL4200_PROD_ID) {
		ret = i2c_smbus_read_word_data(data->client, VCNL4040_DEV_ID);
		if (ret < 0)
			return ret;

		id = ret & 0xff;

		if (id != VCNL4040_PROD_ID)
			return -ENODEV;
	}

	dev_dbg(&data->client->dev, "device id 0x%x", id);

	data->rev = (ret >> 8) & 0xf;

	data->vcnl4200_al.reg = VCNL4200_AL_DATA;
	data->vcnl4200_ps.reg = VCNL4200_PS_DATA;
	switch (id) {
	case VCNL4200_PROD_ID:
		/* Default wait time is 50ms, add 20% tolerance. */
		data->vcnl4200_al.sampling_rate = ktime_set(0, 60000 * 1000);
		/* Default wait time is 4.8ms, add 20% tolerance. */
		data->vcnl4200_ps.sampling_rate = ktime_set(0, 5760 * 1000);
		data->al_scale = 24000;
		break;
	case VCNL4040_PROD_ID:
		/* Default wait time is 80ms, add 20% tolerance. */
		data->vcnl4200_al.sampling_rate = ktime_set(0, 96000 * 1000);
		/* Default wait time is 5ms, add 20% tolerance. */
		data->vcnl4200_ps.sampling_rate = ktime_set(0, 6000 * 1000);
		data->al_scale = 120000;
		break;
	}
	mutex_init(&data->vcnl4200_al.lock);
	mutex_init(&data->vcnl4200_ps.lock);

	ret = data->chip_spec->set_power_state(data, true);
	if (ret < 0)
		return ret;

	return 0;
};

static int vcnl4000_measure(struct vcnl4000_data *data, u8 req_mask,
				u8 rdy_mask, u8 data_reg, int *val)
{
	int tries = 20;
	__be16 buf;
	int ret;

	mutex_lock(&data->vcnl4000_lock);

	ret = i2c_smbus_write_byte_data(data->client, VCNL4000_COMMAND,
					req_mask);
	if (ret < 0)
		goto fail;

	/* wait for data to become ready */
	while (tries--) {
		ret = i2c_smbus_read_byte_data(data->client, VCNL4000_COMMAND);
		if (ret < 0)
			goto fail;
		if (ret & rdy_mask)
			break;
		msleep(20); /* measurement takes up to 100 ms */
	}

	if (tries < 0) {
		dev_err(&data->client->dev,
			"vcnl4000_measure() failed, data not ready\n");
		ret = -EIO;
		goto fail;
	}

	ret = i2c_smbus_read_i2c_block_data(data->client,
		data_reg, sizeof(buf), (u8 *) &buf);
	if (ret < 0)
		goto fail;

	mutex_unlock(&data->vcnl4000_lock);
	*val = be16_to_cpu(buf);

	return 0;

fail:
	mutex_unlock(&data->vcnl4000_lock);
	return ret;
}

static int vcnl4200_measure(struct vcnl4000_data *data,
		struct vcnl4200_channel *chan, int *val)
{
	int ret;
	s64 delta;
	ktime_t next_measurement;

	mutex_lock(&chan->lock);

	next_measurement = ktime_add(chan->last_measurement,
			chan->sampling_rate);
	delta = ktime_us_delta(next_measurement, ktime_get());
	if (delta > 0)
		usleep_range(delta, delta + 500);
	chan->last_measurement = ktime_get();

	mutex_unlock(&chan->lock);

	ret = i2c_smbus_read_word_data(data->client, chan->reg);
	if (ret < 0)
		return ret;

	*val = ret;

	return 0;
}

static int vcnl4000_measure_light(struct vcnl4000_data *data, int *val)
{
	return vcnl4000_measure(data,
			VCNL4000_AL_OD, VCNL4000_AL_RDY,
			VCNL4000_AL_RESULT_HI, val);
}

static int vcnl4200_measure_light(struct vcnl4000_data *data, int *val)
{
	return vcnl4200_measure(data, &data->vcnl4200_al, val);
}

static int vcnl4000_measure_proximity(struct vcnl4000_data *data, int *val)
{
	return vcnl4000_measure(data,
			VCNL4000_PS_OD, VCNL4000_PS_RDY,
			VCNL4000_PS_RESULT_HI, val);
}

static int vcnl4200_measure_proximity(struct vcnl4000_data *data, int *val)
{
	return vcnl4200_measure(data, &data->vcnl4200_ps, val);
}

static const struct vcnl4000_chip_spec vcnl4000_chip_spec_cfg[] = {
	[VCNL4000] = {
		.prod = "VCNL4000",
		.init = vcnl4000_init,
		.measure_light = vcnl4000_measure_light,
		.measure_proximity = vcnl4000_measure_proximity,
		.set_power_state = vcnl4000_set_power_state,
	},
	[VCNL4010] = {
		.prod = "VCNL4010/4020",
		.init = vcnl4000_init,
		.measure_light = vcnl4000_measure_light,
		.measure_proximity = vcnl4000_measure_proximity,
		.set_power_state = vcnl4000_set_power_state,
	},
	[VCNL4040] = {
		.prod = "VCNL4040",
		.init = vcnl4200_init,
		.measure_light = vcnl4200_measure_light,
		.measure_proximity = vcnl4200_measure_proximity,
		.set_power_state = vcnl4200_set_power_state,
	},
	[VCNL4200] = {
		.prod = "VCNL4200",
		.init = vcnl4200_init,
		.measure_light = vcnl4200_measure_light,
		.measure_proximity = vcnl4200_measure_proximity,
		.set_power_state = vcnl4200_set_power_state,
	},
};

static ssize_t vcnl4000_read_near_level(struct iio_dev *indio_dev,
					uintptr_t priv,
					const struct iio_chan_spec *chan,
					char *buf)
{
	struct vcnl4000_data *data = iio_priv(indio_dev);

	return sprintf(buf, "%u\n", data->near_level);
}

static const struct iio_chan_spec_ext_info vcnl4000_ext_info[] = {
	{
		.name = "nearlevel",
		.shared = IIO_SEPARATE,
		.read = vcnl4000_read_near_level,
	},
	{ /* sentinel */ }
};

static const struct iio_chan_spec vcnl4000_channels[] = {
	{
		.type = IIO_LIGHT,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
			BIT(IIO_CHAN_INFO_SCALE),
	}, {
		.type = IIO_PROXIMITY,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.ext_info = vcnl4000_ext_info,
	}
};

static int vcnl4000_set_pm_runtime_state(struct vcnl4000_data *data, bool on)
{
	struct device *dev = &data->client->dev;
	int ret;

	if (on) {
		ret = pm_runtime_get_sync(dev);
		if (ret < 0)
			pm_runtime_put_noidle(dev);
	} else {
		pm_runtime_mark_last_busy(dev);
		ret = pm_runtime_put_autosuspend(dev);
	}

	return ret;
}

static int vcnl4000_read_raw(struct iio_dev *indio_dev,
				struct iio_chan_spec const *chan,
				int *val, int *val2, long mask)
{
	int ret;
	struct vcnl4000_data *data = iio_priv(indio_dev);

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		ret = vcnl4000_set_pm_runtime_state(data, true);
		if  (ret < 0)
			return ret;

		switch (chan->type) {
		case IIO_LIGHT:
			ret = data->chip_spec->measure_light(data, val);
			if (!ret)
				ret = IIO_VAL_INT;
			break;
		case IIO_PROXIMITY:
			ret = data->chip_spec->measure_proximity(data, val);
			if (!ret)
				ret = IIO_VAL_INT;
			break;
		default:
			ret = -EINVAL;
		}
		vcnl4000_set_pm_runtime_state(data, false);
		return ret;
	case IIO_CHAN_INFO_SCALE:
		if (chan->type != IIO_LIGHT)
			return -EINVAL;

		*val = 0;
		*val2 = data->al_scale;
		return IIO_VAL_INT_PLUS_MICRO;
	default:
		return -EINVAL;
	}
}

static const struct iio_info vcnl4000_info = {
	.read_raw = vcnl4000_read_raw,
};

static int vcnl4000_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	struct vcnl4000_data *data;
	struct iio_dev *indio_dev;
	int ret;

	indio_dev = devm_iio_device_alloc(&client->dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data = iio_priv(indio_dev);
	i2c_set_clientdata(client, indio_dev);
	data->client = client;
	data->id = id->driver_data;
	data->chip_spec = &vcnl4000_chip_spec_cfg[data->id];

	ret = data->chip_spec->init(data);
	if (ret < 0)
		return ret;

	dev_dbg(&client->dev, "%s Ambient light/proximity sensor, Rev: %02x\n",
		data->chip_spec->prod, data->rev);

	if (device_property_read_u32(&client->dev, "proximity-near-level",
				     &data->near_level))
		data->near_level = 0;

	indio_dev->dev.parent = &client->dev;
	indio_dev->info = &vcnl4000_info;
	indio_dev->channels = vcnl4000_channels;
	indio_dev->num_channels = ARRAY_SIZE(vcnl4000_channels);
	indio_dev->name = VCNL4000_DRV_NAME;
	indio_dev->modes = INDIO_DIRECT_MODE;

	ret = pm_runtime_set_active(&client->dev);
	if (ret < 0)
		goto fail_poweroff;

	ret = iio_device_register(indio_dev);
	if (ret < 0)
		goto fail_poweroff;

	pm_runtime_enable(&client->dev);
	pm_runtime_set_autosuspend_delay(&client->dev, VCNL4000_SLEEP_DELAY_MS);
	pm_runtime_use_autosuspend(&client->dev);

	return 0;
fail_poweroff:
	data->chip_spec->set_power_state(data, false);
	return ret;
}

static const struct of_device_id vcnl_4000_of_match[] = {
	{
		.compatible = "vishay,vcnl4000",
		.data = (void *)VCNL4000,
	},
	{
		.compatible = "vishay,vcnl4010",
		.data = (void *)VCNL4010,
	},
	{
		.compatible = "vishay,vcnl4020",
		.data = (void *)VCNL4010,
	},
	{
		.compatible = "vishay,vcnl4040",
		.data = (void *)VCNL4040,
	},
	{
		.compatible = "vishay,vcnl4200",
		.data = (void *)VCNL4200,
	},
	{},
};
MODULE_DEVICE_TABLE(of, vcnl_4000_of_match);

static int vcnl4000_remove(struct i2c_client *client)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(client);
	struct vcnl4000_data *data = iio_priv(indio_dev);

	pm_runtime_dont_use_autosuspend(&client->dev);
	pm_runtime_disable(&client->dev);
	iio_device_unregister(indio_dev);
	pm_runtime_set_suspended(&client->dev);

	return data->chip_spec->set_power_state(data, false);
}

static int __maybe_unused vcnl4000_runtime_suspend(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct vcnl4000_data *data = iio_priv(indio_dev);

	return data->chip_spec->set_power_state(data, false);
}

static int __maybe_unused vcnl4000_runtime_resume(struct device *dev)
{
	struct iio_dev *indio_dev = i2c_get_clientdata(to_i2c_client(dev));
	struct vcnl4000_data *data = iio_priv(indio_dev);

	return data->chip_spec->set_power_state(data, true);
}

static const struct dev_pm_ops vcnl4000_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend,
				pm_runtime_force_resume)
	SET_RUNTIME_PM_OPS(vcnl4000_runtime_suspend,
			   vcnl4000_runtime_resume, NULL)
};

static struct i2c_driver vcnl4000_driver = {
	.driver = {
		.name   = VCNL4000_DRV_NAME,
		.pm	= &vcnl4000_pm_ops,
		.of_match_table = vcnl_4000_of_match,
	},
	.probe  = vcnl4000_probe,
	.id_table = vcnl4000_id,
	.remove	= vcnl4000_remove,
};

module_i2c_driver(vcnl4000_driver);

MODULE_AUTHOR("Peter Meerwald <pmeerw@pmeerw.net>");
MODULE_DESCRIPTION("Vishay VCNL4000 proximity/ambient light sensor driver");
MODULE_LICENSE("GPL");
