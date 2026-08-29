# BEGIN makefw-subdirs
SUBDIRS := \
	libsrc \
	src
# END makefw-subdirs

include $(APP_DIR)/c-platform/prod/runtime-bundle.mk

# struct-meta-gen は src のビルド中に実行するため、実行時ライブラリを先に配置する。
# clean は SUBDIRS 経由で src を辿るため、この依存を clean 時に付けると
# 依存 app の成果物削除後にコピーして失敗する。
ifeq ($(filter clean,$(MAKECMDGOALS)),)
src: c-platform-runtime-bundle
endif

default build: c-platform-runtime-bundle

clean: c-platform-runtime-clean
