include $(TOPDIR)/rules.mk

PKG_NAME:=lidar
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/$(PKG_NAME)
  SECTION:=projekt
  CATEGORY:=Projekt
  TITLE:=lidar
  DEPENDS:=+libstdcpp +libc
endef

# Package description; a more verbose description on what our package does
define Package/$(PKG_NAME)/description
  Lidar app for project.
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)/VL53L0X_rasp" clean
	$(MAKE) -C "$(PKG_BUILD_DIR)" clean
	$(MAKE) -C $(PKG_BUILD_DIR) \
			CXX="$(TARGET_CXX)" \
			CC="$(TARGET_CC)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)"
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/main $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,$(PKG_NAME)))
