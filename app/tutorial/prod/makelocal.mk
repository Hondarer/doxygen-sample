include $(APP_DIR)/c-platform/prod/runtime-bundle.mk

c-platform-runtime-bundle: src

default build: c-platform-runtime-bundle

clean: c-platform-runtime-clean
