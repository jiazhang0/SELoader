LIB_NAME := libSELoader
LIB_TARGET := $(LIB_NAME).a

OBJS_$(LIB_NAME) := \
	Memory.o \
	File.o \
	Protocol.o \
	Image.o \
	Variable.o \
	Device.o \
	DevicePath.o \
	Stall.o \
	Console.o \
	ResetSystem.o \
	Pkcs7Verify.o \
	Hash.o \
	Signature.o \
	SecurityPolicy.o \
	UefiSecureBoot.o \
	MokVerify.o \
	Mok2Verify.o \
	Sap.o \
	Sap2.o \
	EfiLibrary.o

LIBS_$(LIB_NAME) := \
	BaseLibrary/libBaseLibrary.a

all: $(LIB_TARGET) $(LIBS_$(LIB_NAME)) Makefile

clean:
	@$(RM) $(LIB_TARGET) $(OBJS_$(LIB_NAME)) \
	    $(addsuffix .*, $(LIB_TARGET))
	@$(MAKE) -C BaseLibrary clean

$(LIB_TARGET): $(OBJS_$(LIB_NAME))
	@$(AR) rcs $@ $^

BaseLibrary/libBaseLibrary.a:
	@$(MAKE) -C BaseLibrary
