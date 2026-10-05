################################################################################
#
#  X6100 GUI application
#
################################################################################

X6100_GUI_VERSION = 02f524068f8ec19031331bab38f06eb212efd47b
X6100_GUI_SITE = https://github.com/gdyuldin/x6100_gui
X6100_GUI_SITE_METHOD = git
X6100_GUI_GIT_SUBMODULES = YES
X6100_GUI_LICENSE = GPLv2

X6100_GUI_INSTALL_STAGING = YES

X6100_GUI_DEPENDENCIES = rhvoice libpng aether-x6100-control ft8lib

$(eval $(cmake-package))
