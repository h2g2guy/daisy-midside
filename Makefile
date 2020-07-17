# Project Name
TARGET = MidSide

# Sources
CPP_SOURCES = midside.cpp oscilloscope.cpp

# Library Locations
LIBDAISY_DIR = ../libdaisy
DAISYSP_DIR = ../daisysp

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

