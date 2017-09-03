/* Copyright (c) 2010-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __MSM_PERIPHERAL_LOADER_H
#define __MSM_PERIPHERAL_LOADER_H

#include <linux/dma-attrs.h>

struct device;
struct module;
struct pil_priv;

/**
 * struct pil_desc - PIL descriptor
 * @name: string used for pil_get()
 * @dev: parent device
 * @ops: callback functions
 * @owner: module the descriptor belongs to
 * @proxy_timeout: delay in ms until proxy vote is removed
 * @flags: bitfield for image flags
 * @priv: DON'T USE - internal only
 * @attrs: DMA attributes to be used during dma allocation.
 * @proxy_unvote_irq: IRQ to trigger a proxy unvote. proxy_timeout
 * is ignored if this is set.
 * @map_fw_mem: Custom function used to map physical address space to virtual.
 * This defaults to ioremap if not specified.
 * @unmap_fw_mem: Custom function used to undo mapping by map_fw_mem.
 * This defaults to iounmap if not specified.
 */
struct pil_desc {
	const char *name;
	struct device *dev;
	const struct pil_reset_ops *ops;
	struct module *owner;
	unsigned long proxy_timeout;
	unsigned long flags;
#define PIL_SKIP_ENTRY_CHECK	BIT(0)
	struct pil_priv *priv;
	struct dma_attrs attrs;
	unsigned int proxy_unvote_irq;
	void * (*map_fw_mem)(phys_addr_t phys, size_t size, void *data);
	void (*unmap_fw_mem)(void *virt, size_t size, void *data);
	void *map_data;
};

/**
 * struct pil_image_info - info in IMEM about image and where it is loaded
 * @name: name of image (may or may not be NULL terminated)
 * @start: indicates physical address where image starts (little endian)
 * @size: size of image (little endian)
 */
struct pil_image_info {
	char name[8];
	__le64 start;
	__le32 size;
} __attribute__((__packed__));

/**
 * struct pil_reset_ops - PIL operations
 * @init_image: prepare an image for authentication
 * @mem_setup: prepare the image memory region
 * @verify_blob: authenticate a program segment, called once for each loadable
 *		 program segment (optional)
 * @proxy_vote: make proxy votes before auth_and_reset (optional)
 * @auth_and_reset: boot the processor
 * @proxy_unvote: remove any proxy votes (optional)
 * @deinit_image: restore actions performed in init_image if necessary
 * @shutdown: shutdown the processor
 */
struct pil_reset_ops {
	int (*init_image)(struct pil_desc *pil, const u8 *metadata,
			  size_t size);
	int (*mem_setup)(struct pil_desc *pil, phys_addr_t addr, size_t size);
	int (*verify_blob)(struct pil_desc *pil, phys_addr_t phy_addr,
			   size_t size);
	int (*proxy_vote)(struct pil_desc *pil);
	int (*auth_and_reset)(struct pil_desc *pil);
	void (*proxy_unvote)(struct pil_desc *pil);
	int (*deinit_image)(struct pil_desc *pil);
	int (*shutdown)(struct pil_desc *pil);
};

#ifdef CONFIG_MSM_PIL
extern int pil_desc_init(struct pil_desc *desc);
extern int pil_boot(struct pil_desc *desc);
#if defined(CONFIG_SS_3G_DDR)
extern void pil_free(struct pil_desc *desc);  //wangxiaomei add
#endif
extern void pil_shutdown(struct pil_desc *desc);
extern void pil_free_memory(struct pil_desc *desc);
extern void pil_desc_release(struct pil_desc *desc);
extern phys_addr_t pil_get_entry_addr(struct pil_desc *desc);
extern int pil_do_ramdump(struct pil_desc *desc, void *ramdump_dev);
#else
static inline int pil_desc_init(struct pil_desc *desc) { return 0; }
static inline int pil_boot(struct pil_desc *desc) { return 0; }
#if defined(CONFIG_SS_3G_DDR)
static inline void pil_free(struct pil_desc *desc) { } //wangxiaomei add
#endif
static inline void pil_shutdown(struct pil_desc *desc) { }
static inline void pil_free_memory(struct pil_desc *desc) { }
static inline void pil_desc_release(struct pil_desc *desc) { }
static inline phys_addr_t pil_get_entry_addr(struct pil_desc *desc)
{
	return 0;
}
static inline int pil_do_ramdump(struct pil_desc *desc, void *ramdump_dev)
{
	return 0;
}
#endif

#endif
