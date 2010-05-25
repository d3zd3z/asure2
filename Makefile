# Build it.

objs = asure.o hash.o dir.o
LDLIBS = -lcrypto
CXXFLAGS = -g -MMD -O3 -frepo -Wall

asure: $(objs)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.d %.o: %.cc
	$(COMPILE.cc) -o $*.o $<

clean:
	rm -rf asure *.o *.d *.rpo

-include $(patsubst %.o,%.d,$(objs))
