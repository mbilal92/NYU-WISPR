##############################################
# OpenWrt Makefile for rail
##############################################

include $(TOPDIR)/rules.mk

PKG_NAME:=rail
PKG_RELEASE:=0.1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/rail
	SECTION:=utils
	CATEGORY:=Utilities
	TITLE:=RAIL - redundant array of inexpensive links
	DEPENDS:=+libstdcpp +libc +libgcc +libdaemon 
endef

define Package/rail/description
	Enables multipath over inexpensive wireless links
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
  $(call Build/Configure/Default,--with-linux-headers=$(LINUX_DIR))
endef

define Package/rail/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rail $(1)/bin/
endef

$(eval $(call BuildPackage,rail))
