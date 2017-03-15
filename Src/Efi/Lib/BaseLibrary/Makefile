LIB_NAME := libBaseLibrary
LIB_TARGET := $(LIB_NAME).a

OBJS_$(LIB_NAME) := \
	MemCpy.o \
	MemMove.o \
	MemCmp.o \
	MemSet.o \
	MemDup.o \
	StrDup.o \
	StrAppend.o \
	StrnCpy.o \
	StrCaseCmp.o \
	StrChr.o \
	StrnChr.o \
	StrrChr.o \
	StrStr.o \
	StrrStr.o \
	StrEndsWith.o \
	StrnLen.o \
	StrStr.o

all: $(LIB_TARGET) Makefile

clean:
	@$(RM) $(LIB_TARGET) $(OBJS_$(LIB_NAME)) \
	    $(addsuffix .*, $(LIB_TARGET))

$(LIB_TARGET): $(OBJS_$(LIB_NAME))
	$(AR) rcs $@ $^
