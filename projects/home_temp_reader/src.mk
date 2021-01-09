#See No-OS/tool/scripts/src_model.mk for variable description

SRC_DIRS +=	$(PROJECT)/srcs		\
		$(PLATFORM_DRIVERS)	\
		$(NO-OS)/util		\
		$(NO-OS)/network	\
		$(INCLUDE)

LIBRARIES += mqtt
CFLAGS += DISABLE_SECURE_SOCKET