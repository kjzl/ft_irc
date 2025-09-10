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
# Use recursive expansion for TOTAL_FILES so SRCS can be defined later without warnings
TOTAL_FILES = $(words $(SRCS))
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

INCLUDES := -I$(HDRS_DIR) -Iinclude/commands

SRCS := $(addprefix $(SRCS_DIR)/,\
		main.cpp \
		Server.cpp \
		Client.cpp \
		Message.cpp \
		MessageType.cpp \
		CaseMappedString.cpp \
		Command.cpp \
		Channel.cpp \
		MessageQueue.cpp \
		MessageQueueManager.cpp \
		commands/NickCommand.cpp \
		commands/PassCommand.cpp \
		commands/UserCommand.cpp \
		commands/PrivmsgCommand.cpp \
		commands/JoinCommand.cpp \
		commands/KickCommand.cpp \
		commands/QuitCommand.cpp \
		commands/InviteCommand.cpp \
		commands/TopicCommand.cpp \
		commands/ModeCommand.cpp \
		commands/WhoCommand.cpp \
		commands/UnknownCommand.cpp \
		)

OBJS := $(SRCS:($(SRCS_DIR))/%.cpp=$(OBJ_DIR)/%.o)

HDRS := $(addprefix $(HDRS_DIR)/,\
		CaseMappedString.hpp \
		Channel.hpp \
		Client.hpp \
		Command.hpp \
		MessageType.hpp \
		IrcUtils.hpp \
		Message.hpp \
		Server.hpp \
		commands/NickCommand.hpp \
		commands/PassCommand.hpp \
		commands/UserCommand.hpp \
		commands/PrivmsgCommand.hpp \
		commands/JoinCommand.hpp \
		commands/KickCommand.hpp \
		commands/QuitCommand.hpp \
		commands/InviteCommand.hpp \
		commands/TopicCommand.hpp \
		commands/ModeCommand.hpp \
		commands/WhoCommand.hpp \
		commands/UnknownCommand.hpp \
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
$(OBJ_DIR)/%.o: $(SRCS_DIR)/%.cpp | $(DIRS)
	#$(call update_progress)
	@printf "$(YELLOW)Compiling $(RESET)"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

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
