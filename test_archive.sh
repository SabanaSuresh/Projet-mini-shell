#!/bin/sh

REQUIRED_SRCS_INIT="Makefile README AUTHOR analex.c runtest.sh"
REQUIRED_SRCS_FINAL="minishell.c testlex.c report.pdf"
REQUIRED_BINS="git ls cc make wc echo"


# sh colors codes
GREEN='\033[0;32m'
RED='\033[0;31m'
ORANGE='\033[0;33m'
NC='\033[0m'


verify_bin(){
    for b in $REQUIRED_BINS; do
	if ! which "$b" > /dev/null; then
	    echo "${RED}ERROR:${NC} $f is missing. Cannot compile the project on this system. Aborting."
	    exit 1
	fi
    done
}

verify_files(){
    echo "Checking archive files ..."    
    for f in $REQUIRED_SRCS_INIT; do
	if [ -f "$f" ]; then
	    echo "${GREEN}Info:${NC} $f was found."
	else
	    echo "${ORANGE}WARNING:${NC} $f is missing."
	fi
    done

    echo "Checking required files ..."    
    for f in $REQUIRED_SRCS_FINAL; do
	if [ -f "$f" ]; then
	    echo "${GREEN}Info:${NC} $f was found."
	else
	    echo "${ORANGE}WARNING:${NC} $f is missing."
	fi
    done

}

read_name(){
    read -p "Enter your name : " -r REPLY
    NAME=$REPLY
    read -p "Your name is '$NAME'? [o/N]: " -r REPLY
    echo "" 
    case "$REPLY" in
	O|o|Y|y)
	    echo "Hi $NAME, your name is set."	    
	    echo $NAME > AUTHOR
	    ;;
	*)
	    echo "Aborded."
	    exit 1
	    ;;
    esac
}

test_compile(){
    echo "Compiling projet"
    if make -B minishell; then
	echo "${GREEN}Info:${NC} Compilation successful"
    else
	echo "${RED}ERROR:${NC} cannot compile minishell"
	exit 1
    fi

    echo "Checking for binary"
    if [ -f "minishell" ]; then
	echo "${GREEN}Info:${NC} Found file minishell."
    else
	echo "${RED}ERROR:${NC} no minishell file found after compilation."
	exit 1
    fi
}

test_exec(){
    echo "Testing projet"
    if [ -x "minishell" ]; then
	echo "${GREEN}Info:${NC} minishell is executable."
    else
	echo "${RED}ERROR:${NC} minishell is not executable."
	exit 1
    fi

    if ./minishell < /dev/null; then
	echo "${GREEN}Info:${NC} minishell has executed and returned with success."
    else
	echo "${ORANGE}WARNING:${NC} minishell has not been successfully executed (return code '$?')."
    fi
}

extract_author(){

    echo "Checking author name"
    if [ ! -f "AUTHOR" ]; then
	echo "${ORANGE}WARNING:${NC} author file is missing. Please create a file name AUTHOR with your name in it."
	exit 1
    elif grep -i -e '\bprenom\b' -e '\bnom\b' AUTHOR > /dev/null; then 
	echo "${ORANGE}WARNING:${NC} author name not properly set. "
    else
	echo -n "${GREEN}Info:${NC} The author of this project is: "
	grep -v "$^" AUTHOR
    fi
}

check_num_commits(){
    num_commits=$(git log --grep "Auto commit" --invert-grep | grep -c '^Author:')
    echo "${GREEN}Info:${NC} number of git commits (not counting auto commits): $num_commits"
    if [ "$num_commits" -lt 4 ]; then
	echo "${ORANGE}WARNING:${NC} you have less than 4 commits."
    fi
}

verify_bin
verify_files


extract_author
check_num_commits
test_compile
test_exec

