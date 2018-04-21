// SPDX-Licence-Identifier: GPL-2.0-only

#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <sound/core.h>
#include <sound/jack.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "tegra_asoc_utils.h"
#include "wm8994.h"

struct tegra_wm8994 {
	int gpio_hp_det;
	int gpio_ear_sel;
	int gpio_int_mic_en;
	int gpio_ext_mic_en;
	struct tegra_asoc_utils_data util_data;
};

static int tegra_wm8994_hw_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_card *card = rtd->card;
	struct tegra_wm8994 *machine = snd_soc_card_get_drvdata(card);
	unsigned int srate, mclk;
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
		dev_err(card->dev, "can't configure clocks\n");
		return err;
	}

	err = snd_soc_dai_set_sysclk(codec_dai, WM8994_SYSCLK_MCLK1, mclk,
				     SND_SOC_CLOCK_IN);
	if (err < 0) {
		dev_err(card->dev, "codec_dai clock not set\n");
		return err;
	}

	return 0;
}

static const struct snd_soc_ops tegra_wm8994_ops = {
	.hw_params = tegra_wm8994_hw_params,
};

static struct snd_soc_jack tegra_wm8994_hp_jack;

static struct snd_soc_jack_pin tegra_wm8994_hp_jack_pins[] = {
	{
		.pin = "Headphone Jack",
		.mask = SND_JACK_HEADPHONE,
	},
};

static struct snd_soc_jack_gpio tegra_wm8994_hp_jack_gpio = {
	.name = "headphone detect",
	.report = SND_JACK_HEADPHONE,
	.debounce_time = 150,
	.invert = 1,
};

static struct snd_soc_jack tegra_wm8994_mic_jack;

static struct snd_soc_jack_pin tegra_wm8994_mic_jack_pins[] = {
	{
		.pin = "Mic Jack",
		.mask = SND_JACK_MICROPHONE,
	},
};

static int tegra_wm8994_event_ext_mic(struct snd_soc_dapm_widget *w,
				      struct snd_kcontrol *k, int event)
{
	struct snd_soc_dapm_context *dapm = w->dapm;
	struct snd_soc_card *card = dapm->card;
	struct tegra_wm8994 *machine = snd_soc_card_get_drvdata(card);

	if (gpio_is_valid(machine->gpio_ext_mic_en))
		gpio_set_value_cansleep(machine->gpio_ext_mic_en,
					SND_SOC_DAPM_EVENT_ON(event));

	if (gpio_is_valid(machine->gpio_int_mic_en))
		gpio_set_value_cansleep(machine->gpio_int_mic_en,
					!SND_SOC_DAPM_EVENT_ON(event));

	/* internal/external mic switch */
	if (gpio_is_valid(machine->gpio_ear_sel))
		gpio_set_value_cansleep(machine->gpio_ear_sel,
					SND_SOC_DAPM_EVENT_ON(event));

	return 0;
}

static const struct snd_soc_dapm_widget tegra_wm8994_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Int Spk", NULL),
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic Jack", tegra_wm8994_event_ext_mic),
	SND_SOC_DAPM_SPK("Earpiece Spk", NULL),
};

static const struct snd_kcontrol_new tegra_wm8994_controls[] = {
	SOC_DAPM_PIN_SWITCH("Int Spk"),
};

static int tegra_wm8994_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_component *component = codec_dai->component;
	struct snd_soc_card *card = rtd->card;
	struct tegra_wm8994 *machine = snd_soc_card_get_drvdata(card);

	if (gpio_is_valid(machine->gpio_hp_det)) {
		tegra_wm8994_hp_jack_gpio.gpio = machine->gpio_hp_det;

		snd_soc_card_jack_new(rtd->card, "Headphone Jack",
				      SND_JACK_HEADPHONE, &tegra_wm8994_hp_jack,
				      tegra_wm8994_hp_jack_pins,
				      ARRAY_SIZE(tegra_wm8994_hp_jack_pins));

		snd_soc_jack_add_gpios(&tegra_wm8994_hp_jack, 1,
				       &tegra_wm8994_hp_jack_gpio);
	}

	snd_soc_card_jack_new(rtd->card, "Mic Jack", SND_JACK_MICROPHONE,
			      &tegra_wm8994_mic_jack,
			      tegra_wm8994_mic_jack_pins,
			      ARRAY_SIZE(tegra_wm8994_mic_jack_pins));

	wm8994_mic_detect(component, &tegra_wm8994_mic_jack, 1);

	return 0;
}

static int tegra_wm8994_remove(struct snd_soc_card *card)
{
	struct snd_soc_pcm_runtime *rtd =
				snd_soc_get_pcm_runtime(card, card->dai_link);
	struct snd_soc_dai *codec_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_component *component = codec_dai->component;

	wm8994_mic_detect(component, NULL, 1);

	return 0;
}

SND_SOC_DAILINK_DEFS(aif1,
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_CODEC("wm8994-codec", "wm8994-aif1")),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link tegra_wm8994_dai = {
	.name = "WM8994",
	.stream_name = "WM8994 PCM",
	.init = tegra_wm8994_init,
	.ops = &tegra_wm8994_ops,
	.dai_fmt = SND_SOC_DAIFMT_I2S |
		   SND_SOC_DAIFMT_NB_NF |
		   SND_SOC_DAIFMT_CBS_CFS,
	SND_SOC_DAILINK_REG(aif1),
};

static struct snd_soc_card snd_soc_tegra_wm8994 = {
	.name = "tegra-wm8994",
	.owner = THIS_MODULE,
	.dai_link = &tegra_wm8994_dai,
	.num_links = 1,
	.remove = tegra_wm8994_remove,
	.controls = tegra_wm8994_controls,
	.num_controls = ARRAY_SIZE(tegra_wm8994_controls),
	.dapm_widgets = tegra_wm8994_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(tegra_wm8994_dapm_widgets),
	.fully_routed = true,
};

static int tegra_wm8994_audio_probe(struct platform_device *pdev)
{
	struct snd_soc_card *card = &snd_soc_tegra_wm8994;
	struct device_node *np = pdev->dev.of_node;
	struct tegra_wm8994 *machine;
	int err;

	machine = devm_kzalloc(&pdev->dev, sizeof(*machine), GFP_KERNEL);
	if (!machine)
		return -ENOMEM;

	card->dev = &pdev->dev;
	platform_set_drvdata(pdev, card);
	snd_soc_card_set_drvdata(card, machine);

	machine->gpio_ear_sel = of_get_named_gpio(np, "nvidia,ear-sel-gpios",
						  0);
	if (machine->gpio_ear_sel == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (gpio_is_valid(machine->gpio_ear_sel)) {
		err = devm_gpio_request_one(&pdev->dev, machine->gpio_ear_sel,
					    GPIOF_OUT_INIT_HIGH, "ear_sel");
		if (err) {
			dev_err(card->dev, "can't get ear_sel gpio\n");
			return err;
		}
		gpio_set_value_cansleep(machine->gpio_ear_sel, 0);
	}

	machine->gpio_hp_det = of_get_named_gpio(np, "nvidia,hp-det-gpios", 0);
	if (machine->gpio_hp_det == -EPROBE_DEFER)
		return -EPROBE_DEFER;

	machine->gpio_int_mic_en = of_get_named_gpio(np,
						"nvidia,int-mic-en-gpios", 0);
	if (machine->gpio_int_mic_en == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (gpio_is_valid(machine->gpio_int_mic_en)) {
		/* disable int mic; enable signal is active-high */
		err = devm_gpio_request_one(&pdev->dev,
					    machine->gpio_int_mic_en,
					    GPIOF_OUT_INIT_LOW, "int_mic_en");
		if (err) {
			dev_err(card->dev, "can't get int_mic_en gpio\n");
			return err;
		}
		gpio_set_value_cansleep(machine->gpio_int_mic_en, 1);
	}

	machine->gpio_ext_mic_en = of_get_named_gpio(np,
						"nvidia,ext-mic-en-gpios", 0);
	if (machine->gpio_ext_mic_en == -EPROBE_DEFER)
		return -EPROBE_DEFER;
	if (gpio_is_valid(machine->gpio_ext_mic_en)) {
		/* enable ext mic; enable signal is active-low */
		err = devm_gpio_request_one(&pdev->dev,
					    machine->gpio_ext_mic_en,
					    GPIOF_OUT_INIT_LOW, "ext_mic_en");
		if (err) {
			dev_err(card->dev, "can't get ext_mic_en gpio\n");
			return err;
		}
		gpio_set_value_cansleep(machine->gpio_ext_mic_en, 0);
	}

	err = snd_soc_of_parse_card_name(card, "nvidia,model");
	if (err)
		return err;

	err = snd_soc_of_parse_audio_routing(card, "nvidia,audio-routing");
	if (err)
		return err;

	tegra_wm8994_dai.codecs->of_node = of_parse_phandle(np,
						"nvidia,audio-codec", 0);
	if (!tegra_wm8994_dai.codecs->of_node) {
		dev_err(&pdev->dev,
			"property 'nvidia,audio-codec' missing or invalid\n");
		return -EINVAL;
	}

	tegra_wm8994_dai.cpus->of_node = of_parse_phandle(np,
						"nvidia,i2s-controller", 0);
	if (!tegra_wm8994_dai.cpus->of_node) {
		dev_err(&pdev->dev,
			"property 'nvidia,i2s-controller' missing or invalid\n");
		return -EINVAL;
	}

	tegra_wm8994_dai.platforms->of_node = tegra_wm8994_dai.cpus->of_node;

	err = tegra_asoc_utils_init(&machine->util_data, &pdev->dev);
	if (err) {
		dev_err(&pdev->dev, "tegra_asoc_utils_init failed (%d)\n", err);
		return err;
	}

	err = snd_soc_register_card(card);
	if (err) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", err);
		return err;
	}

	return 0;
}

static int tegra_wm8994_driver_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id tegra_wm8994_of_match[] = {
	{ .compatible = "nvidia,tegra-audio-wm8994", },
	{},
};
MODULE_DEVICE_TABLE(of, tegra_wm8994_of_match);

static struct platform_driver tegra_wm8994_driver = {
	.driver = {
		.name = "tegra-snd-wm8994",
		.pm = &snd_soc_pm_ops,
		.of_match_table = tegra_wm8994_of_match,
	},
	.probe = tegra_wm8994_audio_probe,
	.remove = tegra_wm8994_driver_remove,
};
module_platform_driver(tegra_wm8994_driver);

MODULE_AUTHOR("Sergey Larin <cerg2010cerg2010@mail.ru>");
MODULE_DESCRIPTION("ASoC Tegra WM8994");
MODULE_LICENSE("GPL v2");
