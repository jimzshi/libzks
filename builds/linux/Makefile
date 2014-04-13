CXX=clang++ -std=c++1y -stdlib=libc++
CXXFLAG=-g -O2 -fPIC -finline-functions -Wall -W -Wshadow -Wpointer-arith \
	   -Wcast-qual -Wwrite-strings -Werror -Wno-unused-parameter \
	   -Wno-unused-function -Woverloaded-virtual
AR=ar
LD=$(CXX)
RM=rm -rf
MKDIR=mkdir -p

ROOT=../..
SRC_DIR=$(ROOT)/src
TEST_DIR=$(ROOT)/unittest
OUTPUT_DIR=output
OUTPUT_INC=$(OUTPUT_DIR)/include
OUTPUT_LIB=$(OUTPUT_DIR)/lib

TMP=build
TARGET=libzks
HEADER=$(wildcard $(SRC_DIR)/*.h)
SRC=$(wildcard $(SRC_DIR)/*.cpp)
OBJ=$(subst $(SRC_DIR),$(TMP),$(patsubst %.cpp,%.o,$(SRC)))
DEP=$(patsubst %.o,%.d,$(OBJ))
SO_TARGET=$(TMP)/$(TARGET).so
A_TARGET=$(TMP)/$(TARGET).a

TMP_TEST=test
TEST_TARGET=$(TMP_TEST)/test_zks
TEST_HEADER=$(wildcard $(TEST_DIR)/*.h)
TEST_SRC=$(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ=$(subst $(TEST_DIR),$(TMP_TEST),$(patsubst %.cpp,%.o,$(TEST_SRC)))
TEST_DEP=$(patsubst %.o,%.d,$(TEST_OBJ))
TEST_LD=-L$(OUTPUT_LIB) -lzks -lpthread
TEST_CXXFLAG=$(CXXFLAG) -I$(SRC_DIR) 

all : pre-build $(A_TARGET) post-build
test : all $(TEST_TARGET) 

_debug_ :
	echo $(HEADER)
	echo $(SRC)
	echo $(OBJ)
	echo $(DEP)
	echo $(SO_TARGET)
	echo $(A_TARGET)
	echo "TEST:"
	echo $(TEST_HEADER)
	echo $(TEST_SRC)
	echo $(TEST_OBJ)
	echo $(TEST_DEP)
	echo $(TEST_LD)
	echo $(TEST_CXXFLAG)

pre-build :
	$(MKDIR) $(OUTPUT_INC) $(OUTPUT_LIB) $(TMP) $(TMP_TEST)

-include $(DEP)
-include $(TEST_DEP)

$(TMP)/%.o : $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAG) -c $< -o $@
	$(CXX) -MM -MT $@ -MF $(@:.o=.d) $<

$(A_TARGET) : $(OBJ)
	$(AR) rcs $@ $^
	
$(TMP_TEST)/%.o : $(TEST_DIR)/%.cpp
	$(CXX) $(TEST_CXXFLAG) -c $< -o $@
	$(CXX) $(TEST_CXXFLAG) -MM -MT $@ -MF $(@:.o=.d) $<

$(TEST_TARGET) : $(TEST_OBJ)
	$(CXX) $^ $(A_TARGET) -o $@ $(TEST_LD)

post-build :
	cp $(HEADER) $(OUTPUT_INC)
	cp $(A_TARGET) $(OUTPUT_LIB)
	
clean :
	$(RM) $(TMP) $(TMP_TEST) $(OUTPUT_DIR)

.PHONY: all pre-build post-build clean _debug_