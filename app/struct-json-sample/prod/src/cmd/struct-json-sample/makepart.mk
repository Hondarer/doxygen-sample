# ライブラリの指定
LIBS += com_util cjson struct_json

# structgen (ヘッダー解析ツール) の実行体。
# prod/src/cmd/makelocal.mk の SUBDIRS 宣言順により、必ずこの makepart.mk の
# 評価より前に structgen のビルドが完了している。
ifdef PLATFORM_LINUX
STRUCTGEN_BIN := $(MYAPP_DIR)/prod/cbin/structgen
else ifdef PLATFORM_WINDOWS
STRUCTGEN_BIN := $(MYAPP_DIR)/prod/cbin/structgen.exe
endif

# 解析対象ヘッダーを静的に宣言する。ヘッダー内の typedef struct をすべて変換する。
# 生成される .c / .h のパスはこの宣言からパース時点で決定論的に導出できるため、
# 実行時発見 (pre-build フックやワイルドカード) は不要。
STRUCTGEN_HEADERS := sample_types.h

# GENDIR は _flags.mk (このファイルより後に読み込まれる) で定義されるため、
# ここでは変数を参照できない。GENDIR は常にワークスペース共通の "gen" なので、
# リテラルを直接使う (framework/makefw/docs/makeparts.md の例 6 を参照)。
_structgen_gendir := gen

_structgen_stem = $(basename $(1))

# ヘッダーごとに .c を生成する。structgen は同名の .h も副作用として書き出す。
# 複数ターゲットを 1 行に並べると GNU Make が規則を複製して 2 回起動するため、
# 正本は .c だけにし、.h は .c に依存させる。
# gen/*.c -> obj/*.o のコンパイルは framework 側の _flex_bison_compile.mk が
# GENDIR_EXTRA_C 経由で汎用的に扱う。
define _STRUCTGEN_RULE
$(_structgen_gendir)/$(call _structgen_stem,$(1))_meta.c: $(1) $(STRUCTGEN_BIN) | $(_structgen_gendir)
	@echo "structgen --header $(1)"
	$(STRUCTGEN_BIN) --header $(1) --out $$@
$(_structgen_gendir)/$(call _structgen_stem,$(1))_meta.h: $(_structgen_gendir)/$(call _structgen_stem,$(1))_meta.c
	@test -f $$@
endef
$(foreach h,$(STRUCTGEN_HEADERS),$(eval $(call _STRUCTGEN_RULE,$(h))))

GENDIR_EXTRA_C += $(foreach h,$(STRUCTGEN_HEADERS),$(_structgen_gendir)/$(call _structgen_stem,$(h))_meta.c)

# struct-json-sample.c が生成ヘッダーを #include するため、初回ビルドでも
# ヘッダー生成が先行するよう明示依存を置く。OBJDIR は makepart.mk 評価時点では
# 未定義なので、GENDIR と同様にリテラル obj を使う。
obj/struct-json-sample.o obj/struct-json-sample.obj: \
	$(foreach h,$(STRUCTGEN_HEADERS),$(_structgen_gendir)/$(call _structgen_stem,$(h))_meta.h)
