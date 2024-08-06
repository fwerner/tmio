BUILDDIR:=build

.PHONY: setup compile install uninstall clean debug release

all: compile

$(BUILDDIR):
	meson setup $(BUILDDIR)

clean:
	@rm -r $(BUILDDIR)

compile: $(BUILDDIR)
	meson compile -C $(BUILDDIR)

local: $(BUILDDIR)
	meson configure -Dprefix=${HOME}/.local $(BUILDDIR)

debug release: $(BUILDDIR)
	meson configure --buildtype $@ $(BUILDDIR)
	$(MAKE) compile

install: compile
	meson install -C $(BUILDDIR)

uninstall: $(BUILDDIR)
	cd $(BUILDDIR) && meson --internal uninstall

test: compile
	meson test -C $(BUILDDIR) -v --suite default
