#See No-OS/tool/scripts/src_model.mk for variable description

SRC_DIRS +=	$(PROJECT)/srcs		\
		$(PLATFORM_DRIVERS)	\
		$(NO-OS)/util		\
		$(NO-OS)/network	\
		$(NO-OS)/drivers/adc/aducm3029 \
		$(INCLUDE)

LIBRARIES += mqtt
CFLAGS += -DDISABLE_SECURE_SOCKET