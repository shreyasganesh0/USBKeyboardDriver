#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xf3\xe8\xf3\x11"
	"usb_submit_urb\0\0"
	"\x20\x00\x00\x00\x0b\x05\xdb\x34"
	"_raw_spin_lock_irqsave\0\0"
	"\x24\x00\x00\x00\x70\xce\x5c\xd3"
	"_raw_spin_unlock_irqrestore\0"
	"\x14\x00\x00\x00\x38\x24\xc3\xb0"
	"_dev_err\0\0\0\0"
	"\x14\x00\x00\x00\xcb\x43\x46\xab"
	"_dev_warn\0\0\0"
	"\x18\x00\x00\x00\x55\xd5\xa4\xe2"
	"usb_deregister\0\0"
	"\x14\x00\x00\x00\x36\x02\xed\x4b"
	"input_event\0"
	"\x10\x00\x00\x00\xf6\xcf\x12\xf8"
	"memscan\0"
	"\x14\x00\x00\x00\xb0\x75\x9e\x71"
	"_dev_info\0\0\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x18\x00\x00\x00\xaf\xfc\x16\x7b"
	"kmalloc_caches\0\0"
	"\x20\x00\x00\x00\xee\xfb\xb4\x10"
	"__kmalloc_cache_noprof\0\0"
	"\x20\x00\x00\x00\xa6\x89\xc5\x17"
	"input_allocate_device\0\0\0"
	"\x18\x00\x00\x00\xac\xd5\xa6\x21"
	"usb_alloc_urb\0\0\0"
	"\x1c\x00\x00\x00\x02\xe1\x4f\x08"
	"usb_alloc_coherent\0\0"
	"\x10\x00\x00\x00\x94\xb6\x16\xa9"
	"strnlen\0"
	"\x18\x00\x00\x00\x5a\x16\x6b\x47"
	"sized_strscpy\0\0\0"
	"\x1c\x00\x00\x00\xb1\x8d\x6f\x47"
	"input_free_device\0\0\0"
	"\x10\x00\x00\x00\x9c\x53\x4d\x75"
	"strlen\0\0"
	"\x10\x00\x00\x00\x38\xdf\xac\x69"
	"memcpy\0\0"
	"\x14\x00\x00\x00\x6e\x4a\x6e\x65"
	"snprintf\0\0\0\0"
	"\x20\x00\x00\x00\x13\x7d\x50\x0b"
	"input_register_device\0\0\0"
	"\x24\x00\x00\x00\x31\x14\x85\xe3"
	"device_set_wakeup_enable\0\0\0\0"
	"\x18\x00\x00\x00\xb5\x79\xca\x75"
	"__fortify_panic\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x1c\x00\x00\x00\xe8\x75\xa3\xef"
	"usb_register_driver\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x18\x00\x00\x00\x2f\x96\x28\x47"
	"usb_kill_urb\0\0\0\0"
	"\x18\x00\x00\x00\x1e\x83\x0c\x65"
	"usb_free_urb\0\0\0\0"
	"\x1c\x00\x00\x00\xa0\xe9\xdc\xd0"
	"usb_free_coherent\0\0\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x20\x00\x00\x00\x77\x0f\x2f\xae"
	"input_unregister_device\0"
	"\x18\x00\x00\x00\xde\x9f\x8a\x25"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v*p*d*dc*dsc*dp*ic03isc01ip01in*");

MODULE_INFO(srcversion, "A6D3630F5C28EA27954C952");
