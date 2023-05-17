# SPDX-FileCopyrightText: 2022 Cosylab d.d.
#
# SPDX-License-Identifier: MIT-0

# Minimal makefile for Sphinx documentation
#

# You can set these variables from the command line, and also
# from the environment for the first two.
SPHINXOPTS    ?=
SPHINXBUILD   ?= sphinx-build
SOURCEDIR     = source
BUILDDIR      = build
SOURCE_DATE_EPOCH = 1662550710

.PHONY: all documentation clean distclean realclean help Makefile

documentation: html

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)

clean distclean realclean:
	rm -rf build

# EPICS compatibility: ignore a bunch of targets that need to be supported by
# all makefiles
all inc build install buildInstall rebuild archclean depclean cvsclean runtests tapfiles test-results clean-tests host archuninstall uninstall realuninstall:
	true

# Catch-all target: route all unknown targets to Sphinx using the new
# "make mode" option.  $(O) is meant as a shortcut for $(SPHINXOPTS).
%: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(O)
