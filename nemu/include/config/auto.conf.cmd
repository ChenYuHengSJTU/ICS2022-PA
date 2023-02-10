deps_config := \
	src/device/Kconfig \
	src/memory/Kconfig \
	/home/cyh627/ics2022/nemu/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
