# BEGIN makefw-subdirs
SUBDIRS := \
	libsrc \
	src

include $(APP_DIR)/c-platform/prod/runtime-bundle.mk

# struct-meta-gen は src のビルド中に実行するため、実行時ライブラリを先に配置する。
src: c-platform-runtime-bundle

default build: c-platform-runtime-bundle

clean: c-platform-runtime-clean
# END makefw-subdirs
