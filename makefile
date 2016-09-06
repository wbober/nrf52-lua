.NOTPARALLEL:

#fixme
all: liblua.a libspiffs.a libnrf52.a shell

shell:
	$(MAKE) -f mk/shell.mk

%.a:
	$(MAKE) -f mk/$*.mk

include mk/rules.mk

.PHONY: all shell