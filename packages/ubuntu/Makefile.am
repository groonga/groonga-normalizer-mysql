UBUNTU_CODE_NAMES = bionic,focal,hirsute,impish
UBUNTU_VERSIONS = 18.04,20.04,21.04,21.10
SOURCE = ../$(PACKAGE)-$(VERSION).tar.gz

all:

ensure-configuration:
	@if test -z "$(LAUNCHPAD_UPLOADER_PGP_KEY)"; then				\
	  echo "--with-launchpad-uploader-pgp-key configure option must be specified.";	\
	  false;									\
	fi
	@if test -z "$(GROONGA_SOURCE_PATH)"; then				 \
	  echo "--with-groonga-source-path configure option must be specified."; \
	  false;								 \
	fi

upload: source ensure-configuration
	$(GROONGA_SOURCE_PATH)/packages/ubuntu/upload.rb	\
	  --package '$(PACKAGE)'				\
	  --version '$(VERSION)'				\
	  --source-archive '$(SOURCE)'				\
	  --ubuntu-code-names '$(UBUNTU_CODE_NAMES)'		\
	  --ubuntu-versions '$(UBUNTU_VERSIONS)'		\
	  --debian-directory '$(srcdir)/../debian/'		\
	  --pgp-sign-key '$(LAUNCHPAD_UPLOADER_PGP_KEY)'

source: $(SOURCE)

$(SOURCE):
	ln -s $(abs_top_builddir)/$(PACKAGE)-$(VERSION).tar.gz $(SOURCE)
