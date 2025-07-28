SHELL := /bin/bash
.ONESHELL:
MAKEFILES :=
MAKEFLAGS += --no-print-directory --warn-undefined-variables

BOLD := $(shell printf '\033[1m')
RESET := $(shell printf '\033[0m')
RED := $(shell printf '\033[31;2m')
GREEN := $(shell printf '\033[32m')
YELLOW := $(shell printf '\033[33m')
CLEAR_LINE := $(shell printf '\033[2K')
CURSOR_UP := $(shell printf '\033[1A')
PHONY := all clean fclean re

# Additional pretty printing variables
TOTAL_FILES := $(words $(SRCS))
CURRENT_FILE := 0
PROGRESS_BAR_WIDTH := 40

define update_progress
    @printf "$(CLEAR_LINE)"
    @printf "\r$(YELLOW)Compiling: $(RESET)["
    @$(eval CURRENT_FILE=$(shell echo $$(($(CURRENT_FILE) + 1))))
    @$(eval PERCENTAGE=$(shell echo $$(($(CURRENT_FILE) * 100 / $(TOTAL_FILES)))))
    @$(eval FILLED=$(shell echo $$(($(PERCENTAGE) * $(PROGRESS_BAR_WIDTH) / 100))))
    @$(eval EMPTY=$(shell echo $$(($(PROGRESS_BAR_WIDTH) - $(FILLED)))))
    @printf "%*s" ${FILLED} "" | tr ' ' '='
    @printf "%*s" ${EMPTY} "" | tr ' ' ' '
    @printf "] %3d%% %s" $(PERCENTAGE) "$<"
endef

NAME := ircserv
CXX := c++
OPTIM_FLAGS := -O3 -march=native
CXXFLAGS = -Wall -Wextra -Werror -pedantic -std=c++98 $(OPTIM_FLAGS)

#headers directories
SRCS_DIR	:= src
OBJ_DIR		:= obj
HDRS_DIR	:= include

DIRS = $(addprefix $(OBJ_DIR)/, .)

INCLUDES := -I$(HDRS_DIR)

SRCS := $(addprefix $(SRCS_DIR)/,\
		Server.cpp \
		)

OBJS := $(SRCS:($SRCS_DIR)%.c=$(OBJ_DIR)/%.o)

HDRS := $(addprefix $(HDRS_DIR)/,\
		 \
		)

.PHONY: all clean fclean re sanitize debug

all: $(NAME) $(HDRS)

# Create obj directory
$(DIRS):
	@mkdir -p $@

$(NAME): $(OBJS) $(HDRS) Makefile
	@printf "\n$(BOLD)Linking $(NAME)$(RESET)\n"
	$(CXX) $(CXXFLAGS) $(OBJS) $(INCLUDES) -o $@ && \
	printf "\n$(GREEN)$(BOLD)Build successful!$(RESET)\n" || \
	printf "$(RED)$(BOLD)Build failed!$(RESET)\n"

# Compile object files
$(OBJ_DIR)/%.o: $(SRCS_DIR)%.c | $(DIRS)
	#$(call update_progress)
	@printf "$(YELLOW)Compiling $(RESET)"
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(NAME)
	./$(NAME)

debug: CXXFLAGS += -DDEBUG -g3
debug: re

clean:
	@printf "$(BOLD)Cleaning object files...$(RESET)\n"
	@rm -dRf $(OBJ_DIR)
	@rm -f $(OBJ_DIR)/.flags

fclean: clean
	@printf "$(BOLD)Cleaning executables...$(RESET)\n"
	@rm -f $(NAME)

re: fclean all
