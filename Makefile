TARGET_EXEC ?= robby

BUILD_DIR ?= ./build
SRC_DIRS ?= ./src

SRCS := $(wildcard $(SRC_DIRS)/*.c)
OBJS := $(SRCS:$(SRC_DIRS)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS += -std=c99 -Wall -fopenmp
##  Debug
#CFLAGS += -g -O0 -gdwarf-2
##  Release
CFLAGS += -Ofast -march=native -g -gdwarf-2

CC = gcc

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIRS)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)
