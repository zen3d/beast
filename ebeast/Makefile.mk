# This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0
include $(wildcard $>/ebeast/*.d)
ebeast/cleandirs ::= $(wildcard $>/ebeast/ $>/electron/ $>/app/)
CLEANDIRS         += $(ebeast/cleandirs)
ALL_TARGETS       += ebeast-all
CHECK_TARGETS     += ebeast-check
INSTALL_TARGETS   += ebeast-install
UNINSTALL_TARGETS += ebeast-uninstall
ebeast-all: $>/ebeast/app.rules

# This Makefile creates $>/electron/ebeast and builds the ebeast app in $>/app/.

# Running ebeast:
# - Use 'make run' to start ebeast from the development tree, this ensures that libbse
#   is picked up from the development tree (instead of an installed version) and that
#   electron finds the ebeast app/ files.
# - DevTools can be activated with Shft+Ctrl+I when run from the devleopment tree.
#   To use DevTools on the installed ebeast bundle, install electron-devtools-installer
#   as npm package in $(pkginstalldir)/bundle/app/.
# - When DevTools are enabled, Shift+Ctrl+R can initiate an ebeast page reload.

# == sources ==
ebeast/js.inputs ::= $(strip 		\
	ebeast/main.js			\
	ebeast/menus.js			\
	ebeast/window.html		\
)
ebeast/vc/js.inputs	::= $(wildcard ebeast/vc/*.js)
ebeast/vc/vue.inputs	::= $(wildcard ebeast/vc/*.vue)
ebeast/vc/scss.inputs	::= $(wildcard ebeast/vc/*.scss)
app/files.js		::= $(addprefix $>/app/,    $(notdir $(ebeast/js.inputs)))
app/vc/files.js		::= $(addprefix $>/app/vc/, $(notdir $(ebeast/vc/js.inputs)))
app/vc/files.vue	::= $(addprefix $>/app/vc/, $(notdir $(ebeast/vc/vue.inputs)))
app/vc/files.scss	::= $(addprefix $>/app/vc/, $(notdir $(ebeast/vc/scss.inputs)))
app/tree ::= $(strip 			\
	$(app/files.js)			\
	$(app/vc/files.js)		\
	$(app/vc/files.vue)		\
	$(app/vc/files.scss)		\
)
app/generated ::= $(strip 		\
	$>/app/app.scss			\
	$>/app/assets/gradient-01.png	\
	$>/app/assets/stylesheets.css	\
	$>/app/assets/components.js	\
)
# provide node_modules/ for use in other makefiles
NODE_MODULES.deps ::= $>/ebeast/npm.rules
NODE_MODULES.bin  ::= $>/ebeast/node_modules/.bin/

# == subdirs ==
include ebeast/v8bse/Makefile.mk

# == npm ==
NPM_PROGRESS = $(if $(PARALLEL_MAKE), --progress=false)
ifeq ($(MODE),debug)
ebeast/npm-install-debug = && npm install electron-devtools-installer $(NPM_PROGRESS)
else
ebeast/npm-install-debug =
endif
$>/ebeast/npm.rules: ebeast/package.json.in	| $>/ebeast/ $>/app/
	$(QECHO) MAKE $@
	$Q rm -f -r $>/ebeast/node_modules/ $>/app/node_modules/
	$Q sed	-e 's/@MAJOR@/$(VERSION_MAJOR)/g' \
		-e 's/@MINOR@/$(VERSION_MINOR)/g' \
		-e 's/@MICRO@/$(VERSION_MICRO)/g' \
		$< > $>/app/package.json
	$Q cd $>/app/ \
	  && npm install --production $(NPM_PROGRESS) \
	  && rm -f package-lock.json \
	    $(ebeast/npm-install-debug) \
	  && find . -name package.json -print0 | xargs -0 sed -r "\|$$PWD|s|^(\s*(\"_where\":\s*)?)\"$$PWD|\1\"/...|" -i
	$Q $(CP) -a $>/app/node_modules $>/app/package.json $>/ebeast/
	$Q cd $>/ebeast/ \
	  && npm install $(NPM_PROGRESS)
	$Q echo >$@

# == linting ==
ebeast/sed.uncommentjs ::= sed -nr 's,//.*$$,,g ; 1h ; 1!H ; $$ { g; s,/\*(\*[^/]|[^*])*\*/,,g ; p }' # beware, ignores quoted strings
ebeast/lint.appfiles   ::= $(app/files.js) $(app/vc/files.js) $(app/vc/files.vue)
$>/ebeast/lint.rules: $(ebeast/lint.appfiles) $(ebeast/vc/vue.inputs) | $>/ebeast/npm.rules
	$(QECHO) MAKE $@
	$Q $>/ebeast/node_modules/.bin/eslint -c ebeast/.eslintrc.js -f unix $(ebeast/lint.appfiles)
	@: # check for component pitfalls
	$Q for f in $(ebeast/vc/vue.inputs) ; do \
	  $(ebeast/sed.uncommentjs) "$$f" | sed -e "s,^,$$f: ," \
	  | grep --color=auto '\b__dirname\b' \
	  && { echo 'Error: __dirname is invalid inside Vue component files' | grep --color=auto . ; exit 9 ; } ; \
	done ; :
	$Q echo >$@
ebeast-lint: FORCE
	@rm -f $>/ebeast/lint.rules
	@$(MAKE) $>/ebeast/lint.rules

# == app ==
$>/ebeast/app.rules: $(app/tree) $(app/generated) $>/ebeast/lint.rules $>/ebeast/vue-docs.html $>/ebeast/v8bse/v8bse.node
	$(QECHO) MAKE $@
	$Q $(CP) -L $>/ebeast/v8bse/v8bse.node $>/app/assets/
	$Q rm -f -r $>/electron/ \
	  && $(CP) -a $>/ebeast/node_modules/electron/dist/ $>/electron/ \
	  && rm -fr $>/electron/resources/default_app.asar \
	  && mv $>/electron/electron $>/electron/ebeast
	$Q ln -s ../../app $>/electron/resources/app
	$Q echo >$@

# == $>/app/% ==
$>/app/assets/stylesheets.css: $>/app/app.scss $(app/vc/files.scss)	| $>/ebeast/npm.rules
	$(QGEN) # NOTE: scss source and output file locations must be final, because .map is derived from it
	$Q cd $>/app/ && ../ebeast/node_modules/.bin/node-sass app.scss assets/stylesheets.css --source-map true
$>/app/assets/gradient-01.png: $>/app/assets/stylesheets.css ebeast/Makefile.mk
	$(QGEN) # generate non-banding gradient from stylesheets.css: gradient-01 { -im-convert: "..."; }
	$Q      # see: http://www.imagemagick.org/script/command-line-options.php#noise http://www.imagemagick.org/Usage/canvas/
	$Q tr '\n' ' ' < $>/app/assets/stylesheets.css | \
	     sed -nr 's/.*\bgradient-01\s*\{[^}]*-im-convert:\s*"([^"]*)"\s*[;}].*/\1/; T; p' > $@.cli
	$Q test -s $@.cli # check that we actually found the -im-convert directive
	$Q $(IMAGEMAGICK_CONVERT) $$(cat $@.cli) $@.tmp.png
	$Q rm $@.cli && mv $@.tmp.png $@
define app/cp.EXT
$>/app/%.$1:	  ebeast/%.$1	| $>/app/vc/
	$$(QECHO) COPY $$@
	$Q $(CP) -P $$< $$@
endef
$(eval $(call app/cp.EXT ,scss))	# $>/app/%.scss: ebeast/%.scss
$(eval $(call app/cp.EXT ,html))	# $>/app/%.html: ebeast/%.html
$(eval $(call app/cp.EXT ,vue))		# $>/app/%.vue: ebeast/%.vue
$(eval $(call app/cp.EXT ,js))		# $>/app/%.js: ebeast/%.js

# == assets/components.js ==
$>/app/assets/components.js: $(app/vc/files.js) $(app/vc/files.vue)	| $>/ebeast/lint.rules
	$(QGEN)
	@: # development node_modules are required to generate assets/components.js from vc/*
	$Q cd $>/app/vc/ \
	  && rm -f node_modules \
	  && ln -s ../../ebeast/node_modules \
	  && ./node_modules/.bin/browserify --node --debug -t vueify -e ./bundle.js -o ../assets/components.js \
	  && rm -f node_modules
	@: # Note, since vc/*.js and vc/*.vue are bundled, they do not need to be installed

# == installation ==
ebeast/install: $>/ebeast/app.rules FORCE
	@$(QECHO) INSTALL '$(DESTDIR)$(pkglibdir)/{app|electron}'
	$Q rm -f -r $(DESTDIR)$(pkglibdir)/electron $(DESTDIR)$(pkglibdir)/app
	$Q $(CP) -a $>/electron $>/app $(DESTDIR)$(pkglibdir)/
install: ebeast/install
ebeast/uninstall: FORCE
	@$(QECHO) REMOVE '$(DESTDIR)$(pkglibdir)/{app|electron}'
	$Q rm -f -r $(DESTDIR)$(pkglibdir)/electron $(DESTDIR)$(pkglibdir)/app
uninstall: ebeast/uninstall

# == ebeast-run ==
# export ELECTRON_ENABLE_LOGGING=1
ebeast-run: $>/ebeast/app.rules
	test -f /usr/share/themes/Ambiance/gtk-2.0/gtkrc && export GTK2_RC_FILES='/usr/share/themes/Ambiance/gtk-2.0/gtkrc' ; \
	$>/electron/ebeast
#	LD_PRELOAD="$>/bse/libbse-$(VERSION_MAJOR).so"

# == ebeast/vue-docs.html ==
$>/ebeast/vue-docs.html: $(ebeast/vc/vue.inputs) ebeast/Makefile.mk	| $>/ebeast/
	$(QGEN)
	$Q echo -e "# Vue Components \n\n"		> $@.tmp
	@: # extract <docs/> blocks from vue files and feed them into pandoc
	$Q for f in $(sort $(ebeast/vc/vue.inputs)) ; do \
	    echo ""								>>$@.tmp ; \
	    sed -n '/^<docs>\s*$$/{ :loop n; /^<\/docs>/q; p;  b loop }' <"$$f"	>>$@.tmp \
	    || exit $$? ; \
	done
	$Q sed 's/^  // ; s/^### /\n### /' -i $@.tmp # unindent
	$Q $(PANDOC) --columns=9999 -f markdown_github+pandoc_title_block-hard_line_breaks -t html -s -o $@ $@.tmp
	$Q rm $@.tmp

# NOTE1, prefer LD_PRELOAD over LD_LIBRARY_PATH, to pick up $(builddir)/libbse *before* /usr/lib/libbse
# NOTE2, add --js-flags="--expose-gc" to the command line to enable global.gc();
# If libdbusmenu-glib.so is missing, electron 1.4.15 displays a Gtk+2 menu bar, ignoring
# the BrowserWindow.darkTheme option. Here, we preselect a commonly installed dark Gtk+2
# theme if it's present.

# == ebeast-clean ==
ebeast-clean: FORCE
	rm -f $>/ebeast/npm.rules $>/ebeast/* 2>/dev/null ; :
	rm -f -r $>/app/ $(ebeast/cleandirs)
