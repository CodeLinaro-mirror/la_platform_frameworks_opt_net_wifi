LIBNAME=libwifi-system.so.0

CXXFLAGS += -c
CPPFLAGS += -Iinclude
CPPFLAGS += -DENABLE_LOG
LDFLAGS += -lqti_log_util
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
