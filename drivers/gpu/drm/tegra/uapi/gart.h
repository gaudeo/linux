/* SPDX-License-Identifier: GPL-2.0 */

#ifndef __TEGRA_DRM_GART_H
#define __TEGRA_DRM_GART_H

#include "drm.h"
#include "job.h"

void tegra_bo_gart_unmap_locked(struct tegra_drm *tegra, struct tegra_bo *bo);

int tegra_drm_job_map_gart_locked(struct tegra_drm *tegra,
				  struct tegra_bo **bos,
				  unsigned int num_bos,
				  unsigned long *bos_write_bitmap,
				  unsigned long *bos_gart_bitmap);

void tegra_drm_job_unmap_gart_locked(struct tegra_drm *tegra,
				     struct tegra_bo **bos,
				     unsigned int num_bos,
				     unsigned long *bos_gart_bitmap);

static inline int
tegra_drm_job_map_gart(struct tegra_drm_job *drm_job,
		       struct tegra_bo **bos)
{
	struct tegra_drm *tegra = drm_job->tegra;
	int ret = 0;

	if (!IS_ENABLED(CONFIG_TEGRA_IOMMU_GART) || !tegra->has_gart)
		return 0;

	if (drm_job->num_bos) {
		mutex_lock(&tegra->mm_lock);
		ret = tegra_drm_job_map_gart_locked(tegra, bos,
						    drm_job->num_bos,
						    drm_job->bos_write_bitmap,
						    drm_job->bos_gart_bitmap);
		mutex_unlock(&tegra->mm_lock);
	}

	return ret;
}

static inline void
tegra_drm_job_unmap_gart(struct tegra_drm_job *drm_job,
			 struct tegra_bo **bos)
{
	struct tegra_drm *tegra = drm_job->tegra;

	if (!IS_ENABLED(CONFIG_TEGRA_IOMMU_GART) || !tegra->has_gart)
		return;

	if (drm_job->num_bos && !bitmap_empty(drm_job->bos_gart_bitmap,
					      drm_job->num_bos)) {
		mutex_lock(&tegra->mm_lock);
		tegra_drm_job_unmap_gart_locked(tegra, bos,
						drm_job->num_bos,
						drm_job->bos_gart_bitmap);
		mutex_unlock(&tegra->mm_lock);

		complete_all(&tegra->gart_free_up);
	}
}

static inline int
tegra_bo_check_gart_size(struct tegra_drm *tegra, struct tegra_bo *bo)
{
	size_t gart_size;

	if (!IS_ENABLED(CONFIG_TEGRA_IOMMU_GART) || !tegra->has_gart)
		return -EINVAL;

	if (bo->sgt->nents > 1) {
		gart_size = tegra->domain->geometry.aperture_end + 1 -
			    tegra->domain->geometry.aperture_start;

		if (bo->gem.size > gart_size)
			return -ENOMEM;
	}

	return 0;
}

#endif
