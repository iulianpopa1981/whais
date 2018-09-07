UNIT:=dbs

UNIT_EXES:=
UNIT_LIBS:=
UNIT_LIBS+=wslpastra
UNIT_SHLS:=wpastra

wpastra_INC:=
wpastra_SRC:=pastra/ps_values.cpp pastra/ps_container.cpp pastra/ps_table.cpp\
		   	pastra/ps_dbsmgr.cpp pastra/ps_serializer.cpp pastra/ps_varstorage.cpp\
		   	pastra/ps_blockcache.cpp pastra/ps_textstrategy.cpp pastra/ps_arraystrategy.cpp\
		   	pastra/ps_btree_index.cpp pastra/ps_btree_fields.cpp pastra/ps_templatetable.cpp\
		   	pastra/ps_exception.cpp pastra/ps_valtranslator.cpp

wpastra_cmn_DEF:=WVER_MAJ=1 WVER_MIN=0
wpastra_DEF:=USE_CUSTOM_SHL USE_DBS_SHL DBS_EXPORTING $(wpastra_cmn_DEF)
wpastra_LIB:=utils/wslutils custom/wslcppmemalloc
wpastra_SHL:=custom/wcustom
	   
wpastra_MAJ=.1
wpastra_MIN=.0

wslpastra_DEF:=$(wpastra_cmn_DEF)
wslpastra_SRC:=$(wpastra_SRC)
wslpastra_INC:=$(wpastra_INC)

ifeq ($(BUILD_TESTS),yes)
-include ./$(UNIT)/test/test.mk
endif

$(foreach exe, $(UNIT_EXES), $(eval $(call add_output_executable,$(exe),$(UNIT))))
$(foreach shl, $(UNIT_SHLS), $(eval $(call add_output_shared_lib,$(shl),$(UNIT),$($(shl)_MAJ),$($(shl)_MIN))))
$(foreach lib, $(UNIT_LIBS), $(eval $(call add_output_library,$(lib),$(UNIT))))

