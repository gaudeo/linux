// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2012 NVIDIA CORPORATION.  All rights reserved.
 */

#include <dt-bindings/memory/tegra20-mc.h>

#include "mc.h"

static const struct tegra_mc_client tegra20_mc_clients[] = {
	{
		.id = 0x00,
		.name = "display0a",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x01,
		.name = "display0ab",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x02,
		.name = "display0b",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x03,
		.name = "display0bb",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x04,
		.name = "display0c",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x05,
		.name = "display0cb",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x06,
		.name = "display1b",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x07,
		.name = "display1bb",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x08,
		.name = "eppup",
		.reset_id = TEGRA20_MC_RESET_EPP,
	}, {
		.id = 0x09,
		.name = "g2pr",
		.reset_id = TEGRA20_MC_RESET_2D,
	}, {
		.id = 0x0a,
		.name = "g2sr",
		.reset_id = TEGRA20_MC_RESET_2D,
	}, {
		.id = 0x0b,
		.name = "mpeunifbr",
		.reset_id = TEGRA20_MC_RESET_MPEB,
	}, {
		.id = 0x0c,
		.name = "viruv",
		.reset_id = TEGRA20_MC_RESET_VI,
	}, {
		.id = 0x0d,
		.name = "avpcarm7r",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x0e,
		.name = "displayhc",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x0f,
		.name = "displayhcb",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x10,
		.name = "fdcdrd",
		.reset_id = TEGRA20_MC_RESET_3D,
	}, {
		.id = 0x11,
		.name = "g2dr",
		.reset_id = TEGRA20_MC_RESET_2D,
	}, {
		.id = 0x12,
		.name = "host1xdmar",
		.reset_id = TEGRA20_MC_RESET_HC,
	}, {
		.id = 0x13,
		.name = "host1xr",
		.reset_id = TEGRA20_MC_RESET_HC,
	}, {
		.id = 0x14,
		.name = "idxsrd",
		.reset_id = TEGRA20_MC_RESET_3D,
	}, {
		.id = 0x15,
		.name = "mpcorer",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x16,
		.name = "mpe_ipred",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x17,
		.name = "mpeamemrd",
		.reset_id = TEGRA20_MC_RESET_MPEA,
	}, {
		.id = 0x18,
		.name = "mpecsrd",
		.reset_id = TEGRA20_MC_RESET_MPEC,
	}, {
		.id = 0x19,
		.name = "ppcsahbdmar",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x1a,
		.name = "ppcsahbslvr",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x1b,
		.name = "texsrd",
		.reset_id = TEGRA20_MC_RESET_3D,
	}, {
		.id = 0x1c,
		.name = "vdebsevr",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x1d,
		.name = "vdember",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x1e,
		.name = "vdemcer",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x1f,
		.name = "vdetper",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x20,
		.name = "eppu",
		.reset_id = TEGRA20_MC_RESET_EPP,
	}, {
		.id = 0x21,
		.name = "eppv",
		.reset_id = TEGRA20_MC_RESET_EPP,
	}, {
		.id = 0x22,
		.name = "eppy",
		.reset_id = TEGRA20_MC_RESET_EPP,
	}, {
		.id = 0x23,
		.name = "mpeunifbw",
		.reset_id = TEGRA20_MC_RESET_MPEB,
	}, {
		.id = 0x24,
		.name = "viwsb",
		.reset_id = TEGRA20_MC_RESET_VI,
	}, {
		.id = 0x25,
		.name = "viwu",
		.reset_id = TEGRA20_MC_RESET_VI,
	}, {
		.id = 0x26,
		.name = "viwv",
		.reset_id = TEGRA20_MC_RESET_VI,
	}, {
		.id = 0x27,
		.name = "viwy",
		.reset_id = TEGRA20_MC_RESET_VI,
	}, {
		.id = 0x28,
		.name = "g2dw",
		.reset_id = TEGRA20_MC_RESET_2D,
	}, {
		.id = 0x29,
		.name = "avpcarm7w",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x2a,
		.name = "fdcdwr",
		.reset_id = TEGRA20_MC_RESET_3D,
	}, {
		.id = 0x2b,
		.name = "host1xw",
		.reset_id = TEGRA20_MC_RESET_HC,
	}, {
		.id = 0x2c,
		.name = "ispw",
		.reset_id = TEGRA20_MC_RESET_ISP,
	}, {
		.id = 0x2d,
		.name = "mpcorew",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x2e,
		.name = "mpecswr",
		.reset_id = TEGRA20_MC_RESET_MPEC,
	}, {
		.id = 0x2f,
		.name = "ppcsahbdmaw",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x30,
		.name = "ppcsahbslvw",
		.reset_id = TEGRA_MC_CLIENT_NO_RESET,
	}, {
		.id = 0x31,
		.name = "vdebsevw",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x32,
		.name = "vdembew",
		.reset_id = TEGRA20_MC_RESET_VDE,
	}, {
		.id = 0x33,
		.name = "vdetpmw",
		.reset_id = TEGRA20_MC_RESET_VDE,
	},
};

#define TEGRA20_MC_RESET(_name, _control, _status, _reset, _bit)	\
	{								\
		.name = #_name,						\
		.id = TEGRA20_MC_RESET_##_name,				\
		.control = _control,					\
		.status = _status,					\
		.reset = _reset,					\
		.bit = _bit,						\
	}

static const struct tegra_mc_reset tegra20_mc_resets[] = {
	TEGRA20_MC_RESET(AVPC,   0x100, 0x140, 0x104,  0),
	TEGRA20_MC_RESET(DC,     0x100, 0x144, 0x104,  1),
	TEGRA20_MC_RESET(DCB,    0x100, 0x148, 0x104,  2),
	TEGRA20_MC_RESET(EPP,    0x100, 0x14c, 0x104,  3),
	TEGRA20_MC_RESET(2D,     0x100, 0x150, 0x104,  4),
	TEGRA20_MC_RESET(HC,     0x100, 0x154, 0x104,  5),
	TEGRA20_MC_RESET(ISP,    0x100, 0x158, 0x104,  6),
	TEGRA20_MC_RESET(MPCORE, 0x100, 0x15c, 0x104,  7),
	TEGRA20_MC_RESET(MPEA,   0x100, 0x160, 0x104,  8),
	TEGRA20_MC_RESET(MPEB,   0x100, 0x164, 0x104,  9),
	TEGRA20_MC_RESET(MPEC,   0x100, 0x168, 0x104, 10),
	TEGRA20_MC_RESET(3D,     0x100, 0x16c, 0x104, 11),
	TEGRA20_MC_RESET(PPCS,   0x100, 0x170, 0x104, 12),
	TEGRA20_MC_RESET(VDE,    0x100, 0x174, 0x104, 13),
	TEGRA20_MC_RESET(VI,     0x100, 0x178, 0x104, 14),
};

static int tegra20_mc_hotreset_assert(struct tegra_mc *mc,
				      const struct tegra_mc_reset *rst)
{
	unsigned long flags;
	u32 value;

	spin_lock_irqsave(&mc->lock, flags);

	value = mc_readl(mc, rst->reset);
	mc_writel(mc, value & ~BIT(rst->bit), rst->reset);

	spin_unlock_irqrestore(&mc->lock, flags);

	return 0;
}

static int tegra20_mc_hotreset_deassert(struct tegra_mc *mc,
					const struct tegra_mc_reset *rst)
{
	unsigned long flags;
	u32 value;

	spin_lock_irqsave(&mc->lock, flags);

	value = mc_readl(mc, rst->reset);
	mc_writel(mc, value | BIT(rst->bit), rst->reset);

	spin_unlock_irqrestore(&mc->lock, flags);

	return 0;
}

static int tegra20_mc_block_dma(struct tegra_mc *mc,
				const struct tegra_mc_reset *rst)
{
	unsigned long flags;
	u32 value;

	spin_lock_irqsave(&mc->lock, flags);

	value = mc_readl(mc, rst->control) & ~BIT(rst->bit);
	mc_writel(mc, value, rst->control);

	spin_unlock_irqrestore(&mc->lock, flags);

	return 0;
}

static bool tegra20_mc_dma_idling(struct tegra_mc *mc,
				  const struct tegra_mc_reset *rst)
{
	return mc_readl(mc, rst->status) == 0;
}

static int tegra20_mc_reset_status(struct tegra_mc *mc,
				   const struct tegra_mc_reset *rst)
{
	return (mc_readl(mc, rst->reset) & BIT(rst->bit)) == 0;
}

static int tegra20_mc_unblock_dma(struct tegra_mc *mc,
				  const struct tegra_mc_reset *rst)
{
	unsigned long flags;
	u32 value;

	spin_lock_irqsave(&mc->lock, flags);

	value = mc_readl(mc, rst->control) | BIT(rst->bit);
	mc_writel(mc, value, rst->control);

	spin_unlock_irqrestore(&mc->lock, flags);

	return 0;
}

static const struct tegra_mc_reset_ops tegra20_mc_reset_ops = {
	.hotreset_assert = tegra20_mc_hotreset_assert,
	.hotreset_deassert = tegra20_mc_hotreset_deassert,
	.block_dma = tegra20_mc_block_dma,
	.dma_idling = tegra20_mc_dma_idling,
	.unblock_dma = tegra20_mc_unblock_dma,
	.reset_status = tegra20_mc_reset_status,
};

const struct tegra_mc_soc tegra20_mc_soc = {
	.clients = tegra20_mc_clients,
	.num_clients = ARRAY_SIZE(tegra20_mc_clients),
	.num_address_bits = 32,
	.client_id_mask = 0x3f,
	.intmask = MC_INT_SECURITY_VIOLATION | MC_INT_INVALID_GART_PAGE |
		   MC_INT_DECERR_EMEM,
	.reset_ops = &tegra20_mc_reset_ops,
	.resets = tegra20_mc_resets,
	.num_resets = ARRAY_SIZE(tegra20_mc_resets),
};
