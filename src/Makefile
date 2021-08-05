THE_OS := $(shell uname -s)

default:
	@echo "Detected OS: ${THE_OS}"
	$(MAKE) CC=gcc CXX=g++ \
		CXXFLAGS='$(CXXFLAGS) -D_AFXEXT -DGLOG_NO_ABBREVIATED_SEVERITIES -Wall -Wextra -Wno-deprecated-copy -pipe -O3 -g -ffast-math -flto -std=c++14 -DNDEBUG' \
		LDFLAGS='$(LDFLAGS) -flto -g' \
		mcts_main

debug:
	@echo "Detected OS: ${THE_OS}"
	$(MAKE) CC=gcc CXX=g++ \
		CXXFLAGS='$(CXXFLAGS) -D_AFXEXT -DGLOG_NO_ABBREVIATED_SEVERITIES -Wall -Wextra -Wno-deprecated-copy -pipe -Og -g -std=c++14' \
		LDFLAGS='$(LDFLAGS) -g' \
		mcts_main

clang:
	@echo "Detected OS: ${THE_OS}"
	$(MAKE) CC=clang CXX=clang++ \
		CXXFLAGS='$(CXXFLAGS) -D_AFXEXT -DGLOG_NO_ABBREVIATED_SEVERITIES -Wall -Wextra -Wno-deprecated-copy -O3 -ffast-math -flto -std=c++14 -DNDEBUG' \
		LDFLAGS='$(LDFLAGS) -flto -fuse-linker-plugin' \
		mcts_main

DYNAMIC_LIBS = -lboost_filesystem -lupb -lcares -lgpr -lgrpc++ -lgrpc++_unsecure -lgrpc_unsecure -lprotobuf -lglog -lgflags -lz -labsl_base 
LIBS = -lpthread

CXXFLAGS += -I/opt/homebrew/include 
CXXFLAGS += -I./
LDFLAGS += -L/opt/homebrew/lib
CPPFLAGS += -MD -MP

sources = analyze.cc async_dist_zero_model_client.cc \
		  byo_yomi_timer.cc dist_config.pb.cc dist_zero_model.grpc.pb.cc \
		  config.cc dist_zero_model.pb.cc dist_zero_model_client.cc \
		  eval_routine.cc go_comm.cc go_state.cc leaky_bucket.cc mcts_config.cc \
		  mcts_config.pb.cc mcts_debugger.cc mcts_engine.cc mcts_main.cc mcts_monitor.cc \
		  mcts_selfplay.cc model_config.pb.cc str_utils.cc thread_conductor.cc \
		  timer.cc trt_zero_model.cc wait_group.cc

objects = $(sources:.cc=.o)
deps = $(sources:%.cc=%.d)

-include $(deps)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

mcts_main: $(objects)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS) $(DYNAMIC_LIBS)

clean:
	-$(RM) mcts_main $(objects) $(deps)

.PHONY: clean default debug clang
