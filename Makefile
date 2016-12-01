CC=g++
PROG=vulkan
CXXFLAGS=-Werror -Wall -Wno-misleading-indentation -O2
LDFLAGS=-lvulkan -lglfw
SOURCES=vulkan.cpp tiny_obj_loader.cpp stb_image.cpp
SHADERS=shaders/frag.spv shaders/vert.spv
OBJS=$(SOURCES:.cpp=.o)
.DEFAULT_GOAL:=all

DEPDIR=.d
$(shell mkdir -p $(DEPDIR) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td
COMPILE = $(CXX) $(DEPFLAGS) $(CXXFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d


shaders/%.spv: shaders/shader.% 
	glslangValidator -V shaders/shader.$* -o shaders/$*.spv

%.o : %.cpp
%.o : %.cpp $(DEPDIR)/%.d
	$(COMPILE) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d


$(PROG): $(OBJS)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJS)

.PHONY: all
all: $(SHADERS) $(PROG) 

.PHONY: clean
clean:
	rm -f $(OBJS) $(PROG) $(SHADERS)

-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SOURCES)))
