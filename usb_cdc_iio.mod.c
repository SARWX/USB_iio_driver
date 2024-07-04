#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

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


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x503d6783, "usb_register_driver" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x92997ed8, "_printk" },
	{ 0xe281a3d2, "usb_bulk_msg" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xc5171ad5, "usb_alloc_urb" },
	{ 0x5dc82014, "usb_submit_urb" },
	{ 0x8cc133d6, "usb_free_urb" },
	{ 0xcf6c5808, "devm_iio_device_alloc" },
	{ 0x4128d092, "usb_get_dev" },
	{ 0xfe988cbb, "__devm_iio_trigger_alloc" },
	{ 0x38824869, "devm_iio_trigger_register" },
	{ 0x186d96e4, "devm_iio_kfifo_buffer_setup_ext" },
	{ 0x7981293a, "usb_put_dev" },
	{ 0xf864e4b0, "__devm_iio_device_register" },
	{ 0x183ef54e, "iio_get_time_ns" },
	{ 0x761d18bb, "iio_push_to_buffers" },
	{ 0xbbc6a8fe, "usb_deregister" },
	{ 0x4b5dd0ff, "iio_trigger_validate_own_device" },
	{ 0x78a319e7, "module_layout" },
};

MODULE_INFO(depends, "industrialio,kfifo_buf");

MODULE_ALIAS("usb:v0483pF125d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "AC86F3B7CF46800D034F6D6");
