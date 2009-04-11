#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x91dff6c7, "struct_module" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0xcb32da10, "param_set_int" },
	{ 0xf780bb1, "usb_register_dev" },
	{ 0x1f9c3035, "usb_buffer_alloc" },
	{ 0x6a43e6dd, "usb_alloc_urb" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xee9a9f93, "kmem_cache_alloc" },
	{ 0x564edcb, "kmalloc_caches" },
	{ 0x91330550, "usb_unlink_urb" },
	{ 0x7061fa77, "usb_free_urb" },
	{ 0x66071ae0, "usb_buffer_free" },
	{ 0x37a0cba, "kfree" },
	{ 0x892335e2, "usb_deregister_dev" },
	{ 0x7e43c47, "wait_for_completion" },
	{ 0xd358731, "usb_submit_urb" },
	{ 0xffd3c7, "init_waitqueue_head" },
	{ 0xd6c963c, "copy_from_user" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xbb055ee3, "usb_bulk_msg" },
	{ 0x60a4461c, "__up_wakeup" },
	{ 0x932b5c92, "usb_find_interface" },
	{ 0x96b27088, "__down_failed" },
	{ 0xda4008e6, "cond_resched" },
	{ 0xb98b7751, "usb_register_driver" },
	{ 0xadb7a023, "complete" },
	{ 0x1b7d4074, "printk" },
	{ 0xf2db50ca, "usb_deregister" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=usbcore";

MODULE_ALIAS("usb:v0CD5p0003d*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "C2E0A78388E325A470C69A5");
