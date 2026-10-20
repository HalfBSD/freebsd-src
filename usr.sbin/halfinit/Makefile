PACKAGE= runtime
PROG= halfinit
SRCS= launcher.c
MAN= halfinit.8
LIBADD= halfbsd halfproc lua pthread m
CFLAGS+= -I${SRCTOP}/contrib/lua/src -I${SRCTOP}/lib/liblua -I${SRCTOP}/lib/libhalfbsd
LDFLAGS+= -Wl,-E
FILESGROUPS= MODULES DEFINITIONS EXAMPLES
MODULES= modules/init.lua modules/client.lua modules/service.lua modules/config.lua \
	modules/dependency.lua modules/supervisor.lua modules/control.lua modules/json.lua \
	modules/providers.lua modules/adapters.lua modules/boot.lua modules/handoff.lua modules/fixture.lua modules/singleboot.lua
MODULESDIR= /usr/lib/halfbsd
DEFINITIONS= services/system.lua
DEFINITIONSDIR= /usr/lib/halfbsd/services
EXAMPLES= examples/services.lua examples/boot-system.lua examples/desktop-user.lua
EXAMPLESDIR= /usr/share/examples/halfbsd
WARNS= 6

# Administrator configuration is seeded once, never replaced by installworld.
# Hard-link publication is atomic and cannot overwrite a parallel install/edit.
afterinstall: install-boot-config
install-boot-config: .PHONY
	@set -eu; directory=${DESTDIR:Q}/etc/halfbsd; \
	if [ -L "$$directory" ]; then echo "Refusing symlink: $$directory" >&2; exit 1; fi; \
	${INSTALL} -d -o ${BINOWN} -g ${BINGRP} -m 0755 "$$directory"; \
	config="$$directory/system.lua"; \
	if [ -e "$$config" ] || [ -L "$$config" ]; then exit 0; fi; \
	temporary=$$(mktemp "$$directory/.system.lua.XXXXXX"); \
	trap 'rm -f "$$temporary"' EXIT HUP INT TERM; \
	${INSTALL} -o ${BINOWN} -g ${BINGRP} -m 0600 \
	    ${.CURDIR:Q}/examples/boot-system.lua "$$temporary"; \
	case ${DESTDIR:Q}/ in /|//) \
	    hostname_value=$$(/bin/hostname); \
	    case "$$hostname_value" in ""|*[!a-zA-Z0-9_.-]*) ;; *) \
	        if [ "$${#hostname_value}" -le 255 ]; then \
	            sed "s/hostname='halfbsd'/hostname='$$hostname_value'/" \
	                ${.CURDIR:Q}/examples/boot-system.lua > "$$temporary"; \
	        fi ;; esac ;; esac; \
	if ! ln "$$temporary" "$$config" 2>/dev/null; then \
	    if [ ! -e "$$config" ] && [ ! -L "$$config" ]; then \
	        echo "Cannot publish $$config" >&2; exit 1; \
	    fi; \
	fi
.include <bsd.prog.mk>
