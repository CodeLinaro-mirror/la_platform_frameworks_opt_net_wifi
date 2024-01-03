LIBNAME=libwifi-hal.so.0

CXXFLAGS += -c -std=c++17 -fPIC
CPPFLAGS += -Wno-unused-function -Iinclude
LDFLAGS += -lbase -lcutils -llog -lutils -lnl-3 -lwifi-hal-qcom
LDFLAGS += -shared -Wl,-soname,$(LIBNAME)

#CPPFLAGS += -Wall -Werror

SRCS = \
       wifi_hal_common.cpp \
       driver_tool.cpp \
       hal_tool.cpp
#wifi_hal_fallback.cpp

CXXOBJECTS := $(SRCS:.cpp=.o)

$(LIBNAME): $(CXXOBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf *.o *.so

$(CXXOBJECTS): %.o: %.cpp
	$(CXX) $< -o $@  $(CXXFLAGS) $(CPPFLAGS)
