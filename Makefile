FQBN=$(shell  LC_ALL=C head *.ino | grep FQBN:  | sed -e "s/^.*FQBN://")
PORT=$(shell  LC_ALL=C head *.ino | grep PORT:  | sed -e "s/^.*PORT://")
DESC=$(shell  LC_ALL=C head *.ino | grep DESC:  | sed -e "s/^.*DESC://")
BOARD=$(shell LC_ALL=C head *.ino | grep BOARD: | sed -e "s/^.*BOARD://")
AUTHOR=$(shell LC_ALL=C head *.ino | grep AUTHOR: | sed -e "s/^.*AUTHOR://")
LICENSE=$(shell LC_ALL=C head *.ino | grep LICENSE: | sed -e "s/^.*LICENSE://")
PROJECT=$(shell basename $$PWD)


GITHUBUSER=plocher

CURRENTTAG=$(shell git describe --tags 2>/dev/null)
TAG=$(shell git rev-list --tags --max-count=1 2>/dev/null)
XALLTAGS=$(shell for tag in `git rev-list --tags  --date-order 2>/dev/null`; do git describe --tags $$tag 2>/dev/null; done)
ALLTAGS=$(shell git for-each-ref --sort=-refname refs/tags | grep " tag\t" | sed -e "s/^..* tag.refs\/tags\///")
VERSION=$(shell git describe --tags ${TAG} 2>/dev/null)
MAJOR=$(shell LC_ALL=C echo ${VERSION} | cut -c1-1)
MINOR=$(shell LC_ALL=C /usr/bin/printf "%.*f\n" 1 ${VERSION} | sed -e "s/^[0-9][0-9]*\.//")
NEXTMAJOR="$(shell expr ${MAJOR} + 1).0"
NEXTDOTMINOR=$(shell expr ${MINOR} + 1)
NEXTMINOR="${MAJOR}.${NEXTDOTMINOR}"
MESSAGE=$(shell git log ${VERSION}.. --pretty=\"%s\" 2>/dev/null)
EXISTS=$(shell git ls-remote https://github.com/${GITHUBUSER}/${PROJECT} >/dev/null 2>&1 ; echo $$?)



USER=plocher
COMPANY=SPCoast Sketch
INITIALVERSION=1.0


all:id compile upload

id:
	@echo $(PROJECT) $(DESC)
	@echo "Firmware for Eagle Project: $(BOARD)"
	@echo "Written by $(AUTHOR), $(LICENSE)"
	@echo Building for $(FQBN) on port $(PORT)

debug:
	@(\
            echo "COMPANY:     ${COMPANY}"; \
            echo "USER:        ${USER}"; \
            echo "GITHUBUSER:  ${GITHUBUSER}"; \
            echo "LICENSE:     ${LICENSE}"; \
            echo "PROJECT:     ${PROJECT}"; \
            echo "INITIALVERSION:     ${INITIALVERSION}"; \
            echo "MESSAGE:     ${MESSAGE}"; \
            echo "EXISTS:      ${EXISTS}"; \
            echo "CURRENT:     ${CURRENTTAG}"; \
            echo "TAG:         ${TAG}"; \
            echo "ALLTAGS:     ${ALLTAGS}"; \
            echo "XALLTAGS:     ${XALLTAGS}"; \
            echo "VERSION:     ${VERSION}"; \
            echo "MAJOR:       ${MAJOR}"; \
            echo "MINOR:       ${MINOR}"; \
            echo "NEXTMAJOR:   ${NEXTMAJOR}"; \
            echo "NEXTMINOR:   ${NEXTMINOR}"; \
            echo "Git History: "; \
            git log --pretty=format:"%h %s" --graph | sed -e "s/^/        /g"; \
        )

compile:
	arduino-cli compile -b $(FQBN)

upload:
	arduino-cli upload -b $(FQBN) -p $(PORT)

publish:
	convert2jekyll -t ino -n "$(PROJECT)"

create: init
	-@(\
                if [ ! -d ".git" ]; then \
                        hub init -g; \
                fi; \
                git add .; \
                if [ "${EXISTS}" -ne 0 ]; then \
                        (set -x; \
                        hub create -d "$(COMPANY): $(DESC)"; \
                        git commit -m "Initial version"; \
                        git tag -a ${INITIALVERSION} -m "Initial version"; \
                        git push --set-upstream origin master; \
                        git push origin ${INITIALVERSION}; \
                        ); \
                fi; \
        )
init: INFO .gitignore

commit:
	git commit -a

push:
	git push

major:
	-git commit -a -m "Version ${NEXTMAJOR}"
	-git tag -a ${NEXTMAJOR} -m "Version ${NEXTMAJOR}"
	-git push
	-git push origin ${NEXTMAJOR}

minor:
	-git commit -a -m "Version ${NEXTMINOR}"
	-git tag -a ${NEXTMINOR} -m "Version ${NEXTMINOR}"
	-git push
	-git push origin ${NEXTMINOR}

fixtag:
	git tag -d ${VERSION}; \
        git tag -a ${VERSION} -m ${VERSION}; \
        git push --delete origin ${VERSION}; \
        git push origin ${VERSION}

status:
	@git status
	@echo "Commits since last release (${VERSION}) to GitHub - use  make minor  or  make major  to push a new version, or   make fixtag  to patch."
	@git log ${VERSION}.. --pretty=format:"    %s"
	@echo "History"
	@( \
            sch=$(PROJECT).sch; \
            brd=$(PROJECT).brd; \
            ino=$(PROJECT).ino; \
            printf "\t%-5s    %-35s %-35s %-35s\n" "Ver" "$$sch" "$$brd" "$$ino"; \
            printf "\t%-5s    %-35s %-35s %-35s\n" "===" "=================" "=================" "================="; \
            for t in $(ALLTAGS); do \
                git checkout --quiet $$t; \
                s=`if [ -f $$sch ]; then ls -lh $$sch | head -1 | cut -c30-49; else echo ==; fi`; \
                b=`if [ -f $$brd ]; then ls -lh $$brd | head -1 | cut -c30-49; else echo ==; fi`; \
                c=`if [ -f $$ino ]; then ls -lh $$ino | head -1 | cut -c30-49; else echo ==; fi`; \
                printf "\t%-5s    %-35s %-35s %-35s\n" "$$t" "$$s" "$$b" "$$c"; \
            done; \
            git checkout --quiet master; \
        )

INFO:
	@(\
                echo "title: ${PROJECT}"; \
                echo "project: ${PROJECT}"; \
                echo "designer: ${USER}"; \
                echo "author: ${USER}"; \
                echo "tags: [arduino, SPCoast]"; \
        ) > INFO

.gitignore:
	@for d in "../.." "../../.." "../../../.." ; do \
            if [ -f "$$d/eagle/Dotgitignore" ]; then \
                cp "$$d/eagle/Dotgitignore" .gitignore; \
            fi; \
        done

