# この makefile は、msys や cygwin などの Unix 互換環境上で利用することを想定している。
# ビルドには xdev68k が必要。
# https://github.com/yosshin4004/xdev68k

# 必要な環境変数が定義されていることを確認する。
ifndef XDEV68K_DIR
	$(error ERROR : XDEV68K_DIR is not defined.)
endif

# デフォルトサフィックスを削除
.SUFFIXES:

# ビルド対象の CPU
CPU = 68060

# 各種コマンド短縮名
ATOMIC = perl ${XDEV68K_DIR}/util/atomic.pl
CXX = ${XDEV68K_DIR}/m68k-toolchain/bin/m68k-elf-g++
CC = ${XDEV68K_DIR}/m68k-toolchain/bin/m68k-elf-gcc
GAS2HAS = perl ${XDEV68K_DIR}/util/x68k_gas2has.pl -cpu $(CPU) -inc doscall.inc -inc iocscall.inc
RUN68 = $(ATOMIC) ${XDEV68K_DIR}/run68/run68
HAS = $(RUN68) ${XDEV68K_DIR}/x68k_bin/HAS060.X
# ヒープサイズを9MB、スタックサイズを256KB確保
HLK = $(RUN68) ${XDEV68K_DIR}/x68k_bin/hlk301.x -d _HEAP_SIZE=a00000 -d _STACK_SIZE=40000

# 最終生成物
EXECUTABLE = X68QUAKE.X
TARGET_FILES = $(EXECUTABLE)

# ヘッダ検索パス
INCLUDE_FLAGS =  -I${XDEV68K_DIR}/include/xc -I${XDEV68K_DIR}/include/xdev68k


# コンパイルフラグ
COMMON_FLAGS = -m$(CPU) -fomit-frame-pointer -fno-strict-aliasing -fno-defer-pop $(INCLUDE_FLAGS)
WARN_FLAGS = -Wall -Wno-unused -Wno-builtin-declaration-mismatch
CFLAGS = $(COMMON_FLAGS) $(WARN_FLAGS) -fcall-used-d2 -fcall-used-a2 -finput-charset=cp932 -fexec-charset=cp932 -fverbose-asm
CXXFLAGS = $(CFLAGS) $(WARN_FLAGS) -fno-rtti -fno-exceptions -std=gnu++17

# *.c ソースファイル
C_SRCS = cd_null.c chase.c cl_demo.c cl_input.c cl_main.c cl_parse.c cl_tent.c cmd.c common.c console.c crc.c cvar.c d_edge.c d_fill.c d_init.c d_modech.c d_part.c d_polyse.c d_scan.c d_sky.c d_sprite.c d_surf.c d_vars.c d_zpoint.c draw.c host_cmd.c host.c in_x68k.c keys.c mathlib.c menu.c model.c net_loop.c net_main.c net_none.c net_vcr.c nonintel.c pr_cmds.c pr_edict.c pr_exec.c r_aclip.c r_alias.c r_bsp.c r_draw.c r_edge.c r_efrag.c r_light.c r_main.c r_misc.c r_part.c r_sky.c r_sprite.c r_surf.c r_vars.c sbar.c screen.c snd_null.c sv_main.c sv_move.c sv_phys.c sv_user.c sys_x68k.c vid_x68k.c view.c wad.c world.c zone.c

# *.cpp ソースファイル
CPP_SRCS =

# *.s ソースファイル
ASM_SRCS =

# リンク対象のライブラリファイル
LIBS =\
	${XDEV68K_DIR}/lib/xc/BASLIB.L \
	${XDEV68K_DIR}/lib/xc/CLIB.L \
	${XDEV68K_DIR}/lib/xc/DOSLIB.L \
	${XDEV68K_DIR}/lib/xc/IOCSLIB.L \
	${XDEV68K_DIR}/lib/xc/FLOATFNC.L \
	${XDEV68K_DIR}/lib/m68k_elf/m$(CPU)/libgcc.a \
	${XDEV68K_DIR}/lib/m68k_elf/m$(CPU)/libstdc++.a \

# 中間ファイル生成用ディレクトリ
INTERMEDIATE_DIR = _build/m$(CPU)

# オブジェクトファイル
OBJS =	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.c,%.o,$(C_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.cpp,%.o,$(CPP_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.s,%.o,$(ASM_SRCS)))

# 依存関係ファイル
DEPS =	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.c,%.d,$(C_SRCS))) \
	$(addprefix $(INTERMEDIATE_DIR)/,$(patsubst %.cpp,%.d,$(CPP_SRCS)))

# HLK に入力するリンクリスト
HLK_LINK_LIST = $(INTERMEDIATE_DIR)/_lk_list.tmp

# デフォルトのターゲット
all : $(TARGET_FILES)

# 依存関係ファイルの include
-include $(DEPS)

# 中間生成物の削除
clean : 
	rm -f $(TARGET_FILES)
	rm -rf $(INTERMEDIATE_DIR)

# 実行ファイルの生成
#	HLK に長いパス文字を与えることは難しい。
#	回避策としてリンク対象ファイルを $(INTERMEDIATE_DIR) 以下にコピーし、
#	短い相対パスを用いてリンクを実行させる。
$(EXECUTABLE) : $(OBJS) $(LIBS)
	mkdir -p $(INTERMEDIATE_DIR)
	rm -f $(HLK_LINK_LIST)
	@for FILENAME in $(OBJS); do\
		echo $$FILENAME >> $(HLK_LINK_LIST); \
	done
	@for FILENAME in $(LIBS); do\
		cp $$FILENAME $(INTERMEDIATE_DIR)/`basename $$FILENAME`; \
		echo $(INTERMEDIATE_DIR)/`basename $$FILENAME` >> $(HLK_LINK_LIST); \
	done
	$(HLK) -i $(HLK_LINK_LIST) -o $(EXECUTABLE)

# *.c ソースのコンパイル
$(INTERMEDIATE_DIR)/%.o : %.c makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(CC) -S $(CFLAGS) -o $(INTERMEDIATE_DIR)/$*.m68k-gas.s -MT $(INTERMEDIATE_DIR)/$*.o -MD -MP -MF $(INTERMEDIATE_DIR)/$*.d $<
	$(GAS2HAS) -i $(INTERMEDIATE_DIR)/$*.m68k-gas.s -o $(INTERMEDIATE_DIR)/$*.s
	rm -f $(INTERMEDIATE_DIR)/$*.m68k-gas.s
	$(HAS) -e -u -w0 $(INCLUDE_FLAGS) $(INTERMEDIATE_DIR)/$*.s -o $(INTERMEDIATE_DIR)/$*.o

# *.cpp ソースのコンパイル
$(INTERMEDIATE_DIR)/%.o : %.cpp makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(CXX) -S $(CXXFLAGS) -o $(INTERMEDIATE_DIR)/$*.m68k-gas.s -MT $(INTERMEDIATE_DIR)/$*.o -MD -MP -MF $(INTERMEDIATE_DIR)/$*.d $<
	$(GAS2HAS) -i $(INTERMEDIATE_DIR)/$*.m68k-gas.s -o $(INTERMEDIATE_DIR)/$*.s
	rm -f $(INTERMEDIATE_DIR)/$*.m68k-gas.s
	$(HAS) -e -u -w0 $(INCLUDE_FLAGS) $(INTERMEDIATE_DIR)/$*.s -o $(INTERMEDIATE_DIR)/$*.o

# *.s ソースのアセンブル
$(INTERMEDIATE_DIR)/%.o : %.s makefile
	mkdir -p $(dir $(INTERMEDIATE_DIR)/$*.o)
	$(HAS) -e -u -w0 $(INCLUDE_FLAGS) $*.s -o $(INTERMEDIATE_DIR)/$*.o

