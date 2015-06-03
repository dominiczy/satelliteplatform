# ####################################################
# MSP430 Simulator
# ####################################################

TARGET=msp430lt
INC_DIR := -I. -I$(ARCHC_PATH)/include/archc -I$(SYSTEMC)/include -I$(TLM_PATH)

SRCS := msp430.cpp  msp430_arch.cpp  msp430_arch_ref.cpp  msp430_isa.cpp msp430_gdb_funcs.cpp msp430_intr_handlers.cpp
OBJS := $(SRCS:.cpp=.o)

#------------------------------------------------------
.SILENT:
#------------------------------------------------------
.SUFFIXES: .cc .cpp .o
#-----------------------------------------------------
lib: all
	ar r lib$(TARGET).a $(OBJS)
#------------------------------------------------------
all: msp430.cpp $(OBJS)
#------------------------------------------------------
clean:
	rm -f $(OBJS) *~ *.o *.a
#------------------------------------------------------
distclean: clean
	if test -f Makefile.archc; then $(MAKE) -f Makefile.archc distclean; fi;
#------------------------------------------------------
.cpp.o:
	$(CC) $(CFLAGS) $(INC_DIR) -c $<
#------------------------------------------------------
.cc.o:
	$(CC) $(CFLAGS) $(INC_DIR) -c $<
#------------------------------------------------------
msp430.cpp:
	echo --- No simulator found, using acsim to generate one.
	$(ARCHC_PATH)/bin/acsim msp430.ac -gdb
	$(MAKE) all
