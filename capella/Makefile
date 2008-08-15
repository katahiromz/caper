.SUFFIXES: .o .c .cpp .d

CC		= g++
CPPFLAGS	= -g -Wall -DLINUX
TARGET		= capella
OBJS		= $(TARGET).o capella_dependency.o capella_generate_boost.o capella_generate_cpp.o capella_generate_dot.o capella_generate_stub.o
DEPENDDIR	= ./depend

VPATH = ./depend

.c.d:
	@$(CC) $(CDBFLG) $(CXXFLAGS) $(INC) $(MACHINE) $(ENVIRON) -MM $< > $(DEPENDDIR)/$*.d.t
	@cp $(DEPENDDIR)/$*.d.t $(DEPENDDIR)/$*.d
	@sed -e 's/$*\.o/$*\.d/g' $(DEPENDDIR)/$*.d.t >> $(DEPENDDIR)/$*.d
	@rm $(DEPENDDIR)/$*.d.t
	@echo '$*.o : $*.d' >> $(DEPENDDIR)/$*.d	

.cpp.d:
	@$(CC) $(CDBFLG) $(CXXFLAGS) $(INC) $(MACHINE) $(ENVIRON) -MM $< > $(DEPENDDIR)/$*.d.t
	@cp $(DEPENDDIR)/$*.d.t $(DEPENDDIR)/$*.d
	@sed -e 's/$*\.o/$*\.d/g' $(DEPENDDIR)/$*.d.t >> $(DEPENDDIR)/$*.d
	@rm $(DEPENDDIR)/$*.d.t
	@echo '$*.o : $*.d' >> $(DEPENDDIR)/$*.d	

.cpp.o:
	$(CC) $(CPPFLAGS) -c -o $@ $<

.c.o:
	$(CC) $(CPPFLAGS) -c -o $@ $<

all:
	mkdir -p depend
	make depend -s
	make $(TARGET) -s

depend: $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f $(DEPENDDIR)/*.d

-include $(addprefix $(DEPENDDIR)/,$(OBJS:.o=.d))