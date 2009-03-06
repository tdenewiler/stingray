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
__used
__attribute__((section("__versions"))) = {
	{ 0xfeafcaca, "struct_module" },
	{ 0xa5423cc4, "param_get_int" },
	{ 0xcb32da10, "param_set_int" },
	{ 0xf4e94f59, "usb_unlink_urb" },
	{ 0x97b72e45, "usb_deregister_dev" },
	{ 0x48e7cc93, "usb_register_driver" },
	{ 0xe456bd3a, "complete" },
	{ 0xfb17e60c, "usb_free_urb" },
	{ 0xd5f7e9d0, "usb_buffer_free" },
	{ 0x37a0cba, "kfree" },
	{ 0xec5bfc35, "usb_register_dev" },
	{ 0x32b3d069, "usb_buffer_alloc" },
	{ 0x3013e115, "usb_alloc_urb" },
	{ 0x5a34a45c, "__kmalloc" },
	{ 0x2dc628d, "kmem_cache_alloc" },
	{ 0x7e34800a, "kmalloc_caches" },
	{ 0xbe499d81, "copy_to_user" },
	{ 0xe9074abe, "usb_bulk_msg" },
	{ 0x1cefe352, "wait_for_completion" },
	{ 0x827a0a5d, "usb_submit_urb" },
	{ 0x64cd5d16, "init_waitqueue_head" },
	{ 0x945bc6a7, "copy_from_user" },
	{ 0xea147363, "printk" },
	{ 0x3f1899f1, "up" },
	{ 0x9363c74b, "usb_find_interface" },
	{ 0x748caf40, "down" },
	{ 0x3f8b90d2, "usb_deregister" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=usbcore";

MODULE_ALIAS("usb:v0CD5p0003d*dc*dsc*dp*ic*isc*ip*");

MODULE_INFO(srcversion, "C2E0A78388E325A470C69A5");
