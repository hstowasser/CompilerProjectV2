OBJ_PATH := obj
SRC_PATH := src

src := $(foreach x, $(SRC_PATH), $(wildcard $(addprefix $(x)/*,.cpp*)))
obj := $(addprefix $(OBJ_PATH)/, $(addsuffix .o, $(notdir $(basename $(src)))))

dep = $(obj:.o=.d)  # one dependency file for each source
INCLUDES = -I $(shell pwd)/include

cc := g++
CFLAGS = -g $(INCLUDES) -lstdc++ -Wall  # option to generate a .d file during compilation
LDFLAGS = -lm -g -lstdc++
COBJFLAGS := $(CFLAGS) -c

geecc: $(obj) main.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.cpp*
	$(CC) $(COBJFLAGS) -o $@ $<

-include $(dep)   # include all dep files in the makefile

.PHONY: clean
clean:
	rm -f $(obj) geecc main.o

.PHONY: cleandep
cleandep:
	rm -f $(dep) main.d