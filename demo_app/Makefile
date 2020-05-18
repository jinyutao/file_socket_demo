
TARGET := file_sock

CC ?=$(CROSS_COMPILE)gcc
AR ?=$(CROSS_COMPILE)ar

lib_path=../lib

override CFLAGS += -I$(SDKTARGETSYSROOT)/usr/include \
    -I. -I.. -I$(lib_path)/filesocketwrapper/include -I$(lib_path)/include/ 
override LNK_FLAGS += -lpthread -lstdc++ -L$(lib_path)/lib -lfilesocketwrapper

#注意每行后面不要有空格，否则会算到目录名里面，导致问题
SRC_DIR = src
BUILD_DIR = tmp
OBJ_DIR = $(BUILD_DIR)/obj
DEPS_DIR = $(BUILD_DIR)/deps
IPK_DIR =  $(BUILD_DIR)/ipk

#这里添加其他头文件路径
INC_DIR = \
	-I./include \
	-I./src \
	-I.

#这里添加编译参数
CC_FLAGS += $(INC_DIR) $(CFLAGS) -g -std=c++11

#这里递归遍历3级子目录
DIRS := $(shell find $(SRC_DIR) -maxdepth 3 -type d)

#将每个子目录添加到搜索路径
VPATH = $(DIRS)

#查找src_dir下面包含子目录的所有cpp文件
SOURCES   = $(foreach dir, $(DIRS), $(wildcard $(dir)/*.cpp))
OBJS   = $(addprefix $(OBJ_DIR)/,$(patsubst %.cpp,%.o,$(notdir $(SOURCES))))
DEPS  = $(addprefix $(DEPS_DIR)/, $(patsubst %.cpp,%.d,$(notdir $(SOURCES))))
$(TARGET):$(OBJS)
	$(CC) $^ $(LNK_FLAGS) -o $@
#编译之前要创建OBJ目录，确保目录存在
$(OBJ_DIR)/%.o:%.cpp
	@if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;
	$(CC) -c $(CC_FLAGS) -o $@ $<
#编译之前要创建DEPS目录，确保目录存在
#前面加@表示隐藏命令的执行打印
$(DEPS_DIR)/%.d:%.cpp
	@if [ ! -d $(DEPS_DIR) ]; then mkdir -p $(DEPS_DIR); fi; \
	set -e; rm -f $@; \
	$(CC) -MM $(CC_FLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ipk : $(TARGET).ipk

# ref: https://stackoverflow.com/questions/17369127/extracting-and-creating-ipk-files
#      https://bitsum.com/creating_ipk_packages.htm
$(TARGET).ipk:$(TARGET) ipk_info/control
	@rm -rf $(IPK_DIR)
	@mkdir -p $(IPK_DIR)/user/bin
	cp $(TARGET) $(IPK_DIR)/user/bin
	tar czvf control.tar.gz ipk_info/control
	tar -C $(IPK_DIR) -czvf data.tar.gz .
	echo 2.0 > debian-binary
	ar r $(TARGET).ipk control.tar.gz data.tar.gz debian-binary
	rm debian-binary data.tar.gz control.tar.gz

#前面加-表示忽略错误
-include $(DEPS)
.PHONY : clean
clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TARGET).ipk

