LIBNAME=libwifi-hal.so.0

CXXFLAGS += -c -std=c++17 -fPIC
CPPFLAGS += -Wno-unused-function -Iinclude
CPPFLAGS += -DENABLE_LOG
LDFLAGS += -lqti_log_util
LDFLAGS += -lcutils -lnl-3 -lwifi-hal-qcom
LDFLAGS += -shared -Wl,-soname,$(LIBNAME)

#CPPFLAGS += -Wall -Werror -llog -lutils

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
