# TODO - it would be nice if LPATH could be set by the Makefile that
# includes us, since that has to now our path anyway.
LPATH := convert

LSRC := $(wildcard $(LPATH)/*.cc)
LPRODUCTS := 

# Common rules
include common.mk
