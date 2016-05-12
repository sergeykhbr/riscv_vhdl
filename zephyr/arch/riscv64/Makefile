cflags-y += $(call cc-option,-ffunction-sections,) $(call cc-option,-fdata-sections)
cflags-$(CONFIG_LTO) = $(call cc-option,-flto,)

include $(srctree)/arch/$(ARCH)/soc/$(SOC_PATH)/Makefile

KBUILD_CFLAGS += $(cflags-y)
KBUILD_CFLAGS += -Wno-unused-function
KBUILD_CXXFLAGS += $(cflags-y)
