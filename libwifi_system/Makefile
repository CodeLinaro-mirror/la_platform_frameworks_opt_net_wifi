LIBNAME=libwifi-system.so.0

CXXFLAGS += -c
CPPFLAGS += -Iinclude
LDFLAGS += -lbase
#-lcrypto -lcutils
LDFLAGS += -shared -Wl,-soname,$(LIBNAME)

SRCS = hostapd_manager.cpp supplicant_manager.cpp

CXXOBJECTS := $(SRCS:.cpp=.o)

$(LIBNAME): $(CXXOBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o *.so*

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)
