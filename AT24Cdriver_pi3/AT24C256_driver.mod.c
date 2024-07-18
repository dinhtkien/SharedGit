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
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0x36948ecd, "i2c_register_driver" },
	{ 0x92997ed8, "_printk" },
	{ 0xa10a1794, "of_n_addr_cells" },
	{ 0x38b32101, "of_n_size_cells" },
	{ 0x49b71f84, "of_get_property" },
	{ 0xe2d5255a, "strcmp" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x7920dca, "__devm_regmap_init_i2c" },
	{ 0x9858f364, "get_random_u8" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0xb3b58d73, "regmap_write" },
	{ 0xcbd7fbc1, "regmap_read" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xec85562e, "i2c_del_driver" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "regmap-i2c");

MODULE_ALIAS("of:N*T*CAT24C256");
MODULE_ALIAS("of:N*T*CAT24C256C*");

MODULE_INFO(srcversion, "46017234139A1B3449C609B");
