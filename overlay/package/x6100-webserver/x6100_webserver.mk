################################################################################
#
#  X6100 GUI
#
################################################################################

X6100_WEBSERVER_VERSION = v0.0.4
X6100_WEBSERVER_SITE = https://github.com/gdyuldin/x6100_webserver
X6100_WEBSERVER_SITE_METHOD = git
X6100_WEBSERVER_LICENSE = GPLv2
X6100_WEBSERVER_DEPENDENCIES = python-bottle-sqlite
X6100_WEBSERVER_SETUP_TYPE = flit

# WEBMODERN_START — Eva pack install (CSS + concise templates)
define X6100_WEBSERVER_INSTALL_CYBRTCH
	@webroot=$$(find $(TARGET_DIR)/usr/lib -type d -path '*/site-packages/x6100_webserver' 2>/dev/null | head -1); \
	if [ -n "$$webroot" ]; then \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/cybrtch.css $$webroot/static/css/cybrtch.css; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/base.html $$webroot/views/base.html; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/index.html $$webroot/views/index.html; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/bands.html $$webroot/views/bands.html; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/digital_modes.html $$webroot/views/digital_modes.html; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/files.html $$webroot/views/files.html; \
		$(INSTALL) -D -m 0644 $(X6100_WEBSERVER_PKGDIR)/time.html $$webroot/views/time.html; \
	fi
endef
X6100_WEBSERVER_POST_INSTALL_TARGET_HOOKS += X6100_WEBSERVER_INSTALL_CYBRTCH
# WEBMODERN_END

$(eval $(python-package))
