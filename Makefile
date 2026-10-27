IMG:=mymoulette
OUT:=out
BIN:=$(OUT)/mymoulette
UNAME_M := $(shell uname -m)
SRC := src/main.c \
	   src/isolation/seccomp.c \
	   src/isolation/capabilities.c \
	   src/controller/controller.c \
	   src/utils/utils.c \
	   src/request/request.c \
	   src/request/extract.c \
	   src/request/buildenv.c \
	   src/namespace/namespace.c \
	   src/child/child.c \
	   src/namespace/mount.c \
	   src/namespace/setup_rootfs.c 	
OBJ := $(SRC:.c=.o)
CFLAGS := -Wall -Wextra -Werror -pedantic -std=c99 -I src
LDLIBS := -lseccomp -lcap -lcjson -lcurl

ifeq ($(UNAME_M),aarch64)
  PLATFORM := linux/arm64/v8
else ifeq ($(UNAME_M),arm64)
  PLATFORM := linux/arm64/v8
else
  PLATFORM := linux/amd64
endif

.PHONY: all clean bonus debug test failrendu

all: $(BIN)

$(BIN): build_docker

debug: build_docker_test

test:
	@bash tests/run_tests.sh

bonus: $(BIN)
	sudo setcap cap_dac_override=+ep $(BIN)

build_docker: $(SRC)
	@echo "PLATFORM = $(PLATFORM)"
	@mkdir -p $(OUT)
	docker buildx build --platform=$(PLATFORM) --load -t $(IMG) .
	docker create --platform=$(PLATFORM) --name $(IMG)_container $(IMG)
	docker cp $(IMG)_container:/src/out/mymoulette $(BIN)
	docker rm -f $(IMG)_container

build_docker_test: $(SRC)
	@echo "PLATFORM = $(PLATFORM)"
	@mkdir -p $(OUT)
	docker buildx build --platform=$(PLATFORM) -f tests/Dockerfile.test --load -t $(IMG) .
	docker create --platform=$(PLATFORM) --name $(IMG)_container $(IMG)
	docker cp $(IMG)_container:/src/out/mymoulette $(BIN)
	docker rm -f $(IMG)_container

# =============== MAKE RULES USED BY THE DOCKER CONTAINER ===============

build_bin_test: CFLAGS += -g -fsanitize=address
build_bin_test: $(OBJ)
	gcc $(CFLAGS) -o $(BIN) $(OBJ) $(LDLIBS)

build_bin: $(OBJ)
	gcc $(CFLAGS) -o $(BIN) $(OBJ) $(LDLIBS) $(LDFLAGS)

clean:
	$(RM) -r $(OUT)

failrendu:
	$(MAKE) debug -B
	$(MAKE) bonus
	$(MAKE) test || exit 1
	$(MAKE) clean
	@mkdir -p /tmp/groupe40
	@cp -r README.md Dockerfile Makefile src tests /tmp/groupe40/
	@cd /tmp && tar -czf groupe40.tar.gz groupe40
	@mv /tmp/groupe40.tar.gz .
	@rm -rf /tmp/groupe40
	@echo "Archive groupe40.tar.gz created"
	@echo "Content:"
	@tar -tzf groupe40.tar.gz
