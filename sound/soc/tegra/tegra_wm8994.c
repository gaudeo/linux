// SPDX-Licence-Identifier: GPL-2.0-only

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>

#include "tegra_asoc_utils.h"

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
#include "tegra20_das.h"
#endif

#define DRV_NAME "tegra-snd-wm8994"

struct tegra_wm8994 {
	struct tegra_asoc_utils_data util_data;
};

static int tegra_wm8994_hw_params(struct snd_pcm_substream *substream,
					struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_card *card = rtd->card;
	struct tegra_wm8994 *machine = snd_soc_card_get_drvdata(card);
	int srate, mclk, i2s_daifmt;
	int err;

	srate = params_rate(params);
	switch (srate) {
	case 64000:
	case 88200:
	case 96000:
		mclk = 128 * srate;
		break;
	default:
		mclk = 256 * srate;
		break;
	}
	/* FIXME: Codec only requires >= 3MHz if OSR==0 */
	while (mclk < 6000000)
		mclk *= 2;

	err = tegra_asoc_utils_set_rate(&machine->util_data, srate, mclk);
	if (err < 0) {
		if (!(machine->util_data.set_mclk % mclk))
			mclk = machine->util_data.set_mclk;
		else {
			dev_err(card->dev, "Can't configure clocks\n");
			return err;
		}
	}

	i2s_daifmt = SND_SOC_DAIFMT_NB_NF |
		     SND_SOC_DAIFMT_CBS_CFS;

	i2s_daifmt |= SND_SOC_DAIFMT_I2S;

	err = snd_soc_dai_set_fmt(codec_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(card->dev, "codec_dai fmt not set\n");
		return err;
	}

	err = snd_soc_dai_set_fmt(cpu_dai, i2s_daifmt);
	if (err < 0) {
		dev_err(card->dev, "cpu_dai fmt not set\n");
		return err;
	}
	/* Need to check clk id */
	err = snd_soc_dai_set_sysclk(codec_dai, 1, mclk,
					SND_SOC_CLOCK_IN);
	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

#ifdef CONFIG_ARCH_TEGRA_2x_SOC
	err = tegra20_das_connect_dac_to_dap(TEGRA20_DAS_DAP_SEL_DAC1,
					TEGRA20_DAS_DAP_ID_1);
	if (err < 0) {
		dev_err(card->dev, "failed to set dap-dac path\n");
		return err;
	}

	err = tegra20_das_connect_dap_to_dac(TEGRA20_DAS_DAP_ID_1,
					TEGRA20_DAS_DAP_SEL_DAC1);
	if (err < 0) {
		dev_err(card->dev, "failed to set dac-dap path\n");
		return err;
	}
#endif
	return 0;
}

static struct snd_soc_ops tegra_wm8994_ops = {
	.hw_params = tegra_wm8994_hw_params,
};

static int tegra_wm8994_init(struct snd_soc_pcm_runtime *rtd)
{
	return 0;
}

SND_SOC_DAILINK_DEFS(aif1,
		DAILINK_COMP_ARRAY(COMP_CPU("tegra20-i2s.0")),
		DAILINK_COMP_ARRAY(COMP_CODEC("wm8994-codec", "wm8994-aif1")),
		DAILINK_COMP_ARRAY(COMP_PLATFORM("tegra-pcm-audio")));

static struct snd_soc_dai_link tegra_wm8994_dai[] = {
	{
		.name = "WM8994",
		.stream_name = "WM8994 PCM",
		.init = tegra_wm8994_init,
		.ops = &tegra_wm8994_ops,
		SND_SOC_DAILINK_REG(aif1),
	},
};

static struct snd_soc_card snd_soc_tegra_wm8994 = {
	.name = "tegra-wm8994",
	.owner = THIS_MODULE,
	.dai_link = tegra_wm8994_dai,
	.num_links = ARRAY_SIZE(tegra_wm8994_dai),
};

static int tegra_wm8994_audio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct snd_soc_card *card = &snd_soc_tegra_wm8994;
	struct tegra_wm8994 *machine;
	int ret;

	if (!pdev->dev.platform_data && !pdev->dev.of_node) {
		dev_err(&pdev->dev, "No platform data supplied\n");
		return -EINVAL;
	}

	machine = devm_kzalloc(&pdev->dev, sizeof(struct tegra_wm8994),
			       GFP_KERNEL);
	if (!machine) {
		dev_err(&pdev->dev, "Can't allocate tegra_wm8994 struct\n");
		ret = -ENOMEM;
		goto err;
	}

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, machine);

	ret = snd_soc_of_parse_card_name(card, "nvidia,model");
	if (ret)
		goto err;


	tegra_wm8994_dai[0].codecs->of_node = of_parse_phandle(np,
						"nvidia,audio-codec", 0);
	if (!tegra_wm8994_dai[0].codecs->of_node) {
		dev_err(&pdev->dev,
			"Property 'nvidia,audio-codec' missing or invalid\n");
		ret = -EINVAL;
		goto err;
	}

	tegra_wm8994_dai[0].cpus->of_node = of_parse_phandle(np,
			"nvidia,i2s-controller", 0);
	if (!tegra_wm8994_dai[0].cpus->of_node) {
		dev_err(&pdev->dev,
			"Property 'nvidia,i2s-controller' missing or invalid\n");
		ret = -EINVAL;
		goto err;
	}

	tegra_wm8994_dai[0].platforms->of_node = tegra_wm8994_dai[0].cpus->of_node;

	ret = tegra_asoc_utils_init(&machine->util_data, &pdev->dev);
	if (ret) {
		dev_err(&pdev->dev, "tegra_asoc_utils_init failed (%d)\n",
			ret);
		goto err;
	}

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n",
			ret);
		goto err_fini_utils;
	}

	return 0;

err_fini_utils:
	tegra_asoc_utils_fini(&machine->util_data);
err:
	return ret;
}

static int tegra_wm8994_driver_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	struct tegra_wm8994 *machine = snd_soc_card_get_drvdata(card);

	snd_soc_unregister_card(card);

	tegra_asoc_utils_fini(&machine->util_data);

	return 0;
}

static const struct of_device_id tegra_wm8994_of_match[] = {
	{ .compatible = "nvidia,tegra-audio-wm8994", },
	{},
};

static struct platform_driver tegra_wm8994_driver = {
	.driver = {
		.name = DRV_NAME,
		.pm = &snd_soc_pm_ops,
		.of_match_table = tegra_wm8994_of_match,
	},
	.probe = tegra_wm8994_audio_probe,
	.remove = tegra_wm8994_driver_remove,
};

module_platform_driver(tegra_wm8994_driver);

MODULE_AUTHOR("Sergey Larin <cerg2010cerg2010@mail.ru>");
MODULE_DESCRIPTION("ASoC Tegra WM8994");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);
MODULE_DEVICE_TABLE(of, tegra_wm8994_of_match);
