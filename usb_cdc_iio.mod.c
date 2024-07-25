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

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\x56\xaa\x34\xe8"
	"usb_free_urb\0\0\0\0"
	"\x14\x00\x00\x00\x84\xf8\xad\x6b"
	"_dev_info\0\0\0"
	"\x20\x00\x00\x00\x7f\x7a\x3a\xd1"
	"devm_iio_device_alloc\0\0\0"
	"\x14\x00\x00\x00\xd8\x13\x7e\x82"
	"usb_get_dev\0"
	"\x24\x00\x00\x00\xcb\x6b\xcc\x26"
	"__devm_iio_trigger_alloc\0\0\0\0"
	"\x24\x00\x00\x00\x51\x11\x21\x77"
	"devm_iio_trigger_register\0\0\0"
	"\x28\x00\x00\x00\xc8\x81\xec\x63"
	"devm_iio_kfifo_buffer_setup_ext\0"
	"\x24\x00\x00\x00\x97\x70\x48\x65"
	"__x86_indirect_thunk_rax\0\0\0\0"
	"\x24\x00\x00\x00\x77\x3b\x28\x85"
	"__devm_iio_device_register\0\0"
	"\x14\x00\x00\x00\xd0\x44\x78\xe4"
	"usb_put_dev\0"
	"\x18\x00\x00\x00\x7b\xc7\x60\xb2"
	"usb_deregister\0\0"
	"\x1c\x00\x00\x00\x70\x85\xca\x97"
	"iio_push_to_buffers\0"
	"\x2c\x00\x00\x00\xc6\xfa\xb1\x54"
	"__ubsan_handle_load_invalid_value\0\0\0"
	"\x28\x00\x00\x00\xb9\x1e\xcd\xe8"
	"iio_trigger_validate_own_device\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x1c\x00\x00\x00\xba\xc2\xf8\xca"
	"usb_register_driver\0"
	"\x18\x00\x00\x00\x67\xcc\xe2\xa8"
	"usb_bulk_msg\0\0\0\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x18\x00\x00\x00\x36\xb4\x7e\x19"
	"usb_alloc_urb\0\0\0"
	"\x18\x00\x00\x00\xa4\x65\xe6\x84"
	"usb_submit_urb\0\0"
	"\x18\x00\x00\x00\x83\xfa\x0c\xf5"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "industrialio,kfifo_buf");

MODULE_ALIAS("usb:v0483pF125d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "6490F78D17A7A4BD42BF412");
