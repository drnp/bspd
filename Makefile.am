## Process this file with automake to produce Makefile.in
## Created by Anjuta

SUBDIRS = deps src po

dist_doc_DATA = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS


INTLTOOL_FILES = intltool-extract.in \
	intltool-merge.in \
	intltool-update.in

EXTRA_DIST = \
	$(INTLTOOL_FILES)

DISTCLEANFILES = intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache


# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(docdir)

install: install-recursive
	@echo "Installing support-files"
	@mkdir -p $(prefix)/etc --mode=0755
	@mkdir -p $(prefix)/log --mode=0755
	@mkdir -p $(prefix)/script --mode=0755
	@mkdir -p $(prefix)/run --mode=0755
	@cp --backup $(srcdir)/scripts/bspd.runtime.conf.in $(prefix)/etc/bspd.runtime.conf
	@echo ""
	@echo "************************************************************"
	@echo "*             THANK YOU FOR CHOOSING BS.PLAY               *"
	@echo "*==========================================================*"
	@echo "* BS.Play official site:                                   *"
	@echo "*          http://bsp.bsgroup.org/                         *"
	@echo "* BS.Group:                                                *"
	@echo "*          http://www.bsgroup.org/                         *"
	@echo "* Author:                                                  *"
	@echo "*          Dr.NP <np@bsgroup.org>                          *"
	@echo "************************************************************"
	@echo ""
