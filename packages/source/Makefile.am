FILES =						\
	files/$(PACKAGE)-$(VERSION).tar.gz	\
	files/$(PACKAGE)-$(VERSION).zip

all:

release: download upload

ensure-rsync-path:
	@if test -z "$(RSYNC_PATH)"; then				\
	  echo "--with-rsync-path configure option must be specified.";	\
	  false;							\
	fi

ensure-latest-archive: $(FILES)
	cd files &&							\
	ln -sf $(PACKAGE)-$(VERSION).tar.gz $(PACKAGE)-latest.tar.gz;	\
	ln -sf $(PACKAGE)-$(VERSION).zip $(PACKAGE)-latest.zip

download: ensure-rsync-path
	rsync							\
	  -avz							\
	  --progress						\
	  --delete						\
	  $(RSYNC_PATH)/source/groonga-normalizer-mysql/	\
	  files

upload: ensure-rsync-path ensure-latest-archive $(FILES)
	rsync						\
	  -avz						\
	  --progress					\
	  --delete					\
	  files/					\
	  $(RSYNC_PATH)/source/groonga-normalizer-mysql

files/$(PACKAGE)-$(VERSION).tar.gz: $(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	mkdir -p files
	cp -p $< $@

files/$(PACKAGE)-$(VERSION).zip: files/$(PACKAGE)-$(VERSION).tar.gz
	rm -rf $(PACKAGE)-$(VERSION)
	tar xvzf files/$(PACKAGE)-$(VERSION).tar.gz
	zip -r $@ $(PACKAGE)-$(VERSION)
	rm -rf $(PACKAGE)-$(VERSION)
