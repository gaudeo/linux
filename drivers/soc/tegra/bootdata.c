// SPDX-License-Identifier: GPL-2.0

#include <linux/init.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/types.h>

#include <soc/tegra/bootdata.h>
#include <soc/tegra/common.h>

/*
 * spare_bct[] will be released once kernel is booted, hence not wasting
 * kernel space if BCT is missing. The tegra_bct can't be allocated during
 * of BCT setting up because it's too early for the slab allocator.
 */
static u8 __initdata spare_bct[SZ_8K];
static u8 *tegra_bct;

static ssize_t boot_config_table_read(struct file *filp,
				      struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t off, size_t count)
{
	memcpy(buf, tegra_bct + off, count);
	return count;
}
static BIN_ATTR_RO(boot_config_table, 0);

static int __init tegra_bootdata_bct_sysfs_init(void)
{
	if (!bin_attr_boot_config_table.size)
		return 0;

	tegra_bct = kmalloc(GFP_KERNEL, bin_attr_boot_config_table.size);
	if (!tegra_bct)
		return -ENOMEM;

	memcpy(tegra_bct, spare_bct, bin_attr_boot_config_table.size);

	return sysfs_create_bin_file(tegra_soc_kobj,
				     &bin_attr_boot_config_table);
}
late_initcall(tegra_bootdata_bct_sysfs_init)

void __init tegra_bootdata_bct_setup(void __iomem *bct_ptr, size_t bct_size)
{
	memcpy_fromio(spare_bct, bct_ptr, bct_size);
	bin_attr_boot_config_table.size = bct_size;
}
