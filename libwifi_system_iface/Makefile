LIBNAME=libwifi-system-iface.so.0

CXXFLAGS += -c -std=c++17
CPPFLAGS += -Iinclude
CPPFLAGS += -DENABLE_LOG
LDFLAGS += -lqti_log_util
LDFLAGS += -shared -lbase -Wl,-soname,$(LIBNAME)

#CPPFLAGS += -Wall -Werror

SRCS = interface_tool.cpp

CXXOBJECTS := $(SRCS:.cpp=.o)

$(LIBNAME): $(CXXOBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o *.so

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)
