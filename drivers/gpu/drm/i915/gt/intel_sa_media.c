// SPDX-License-Identifier: MIT
/*
 * Copyright © 2021 Intel Corporation
 */

#include <drm/drm_managed.h>

#include "i915_drv.h"
#include "gt/intel_gt.h"
#include "gt/intel_sa_media.h"
#include "gt/iov/intel_iov.h"

int intel_sa_mediagt_setup(struct intel_gt *gt, unsigned int id,
			   phys_addr_t phys_addr, u32 gsi_offset)
{
	struct drm_i915_private *i915 = gt->i915;
	struct intel_uncore *uncore;
	int err;

	uncore = drmm_kzalloc(&i915->drm, sizeof(*uncore), GFP_KERNEL);
	if (!uncore)
		return -ENOMEM;

	uncore->gsi_offset = gsi_offset;

	gt->irq_lock = to_gt(i915)->irq_lock;
	intel_gt_common_init_early(gt);
	intel_uncore_init_early(uncore, gt);
	intel_iov_init_early(&gt->iov);

	/*
	 * Standalone media shares the general MMIO space with the primary
	 * GT.  We'll re-use the primary GT's mapping.
	 */
	uncore->regs = i915->uncore.regs;
	if (drm_WARN_ON(&i915->drm, uncore->regs == NULL))
		return -EIO;

	gt->uncore = uncore;
	gt->phys_addr = phys_addr;

	err = intel_iov_init_mmio(&gt->iov);
	if (unlikely(err))
		return err;

	/*
	 * For current platforms we can assume there's only a single
	 * media GT and cache it for quick lookup.
	 */
	drm_WARN_ON(&i915->drm, i915->media_gt);
	i915->media_gt = gt;

	return 0;
}
