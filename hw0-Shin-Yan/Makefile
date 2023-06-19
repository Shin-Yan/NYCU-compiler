all: update-info

.PHONY: restore update-info clean
clean: restore

IMAGE_NAME = compiler-s20-hw0
DOCKERHUB_HOST_ACCOUNT=ianre657
IMAGE_FULLNAME = ${DOCKERHUB_HOST_ACCOUNT}/${IMAGE_NAME}:latest

update-info:
	python3 src/update-info.py

restore:
	python3 src/update-info.py --restore --config "res/default_info.ini"

docker-pull:
	docker pull ${IMAGE_FULLNAME}

# Docker
# ========================================================
# ██████╗  ██████╗  ██████╗██╗  ██╗███████╗██████╗
# ██╔══██╗██╔═══██╗██╔════╝██║ ██╔╝██╔════╝██╔══██╗
# ██║  ██║██║   ██║██║     █████╔╝ █████╗  ██████╔╝
# ██║  ██║██║   ██║██║     ██╔═██╗ ██╔══╝  ██╔══██╗
# ██████╔╝╚██████╔╝╚██████╗██║  ██╗███████╗██║  ██║
# ╚═════╝  ╚═════╝  ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝
# ========================================================


.PHONY: docker-build docker-push activate

# Do not named user and group the same, this would cause error in entrypoint.sh
#	because we create the group before user exist which allowing name-crash in useradd command
CONTAINER_USERNAME = student
CONTAINER_GROUPNAME = studentg

HOST_NAME = compiler-s20
HOMEDIR = /home/$(CONTAINER_USERNAME)

# ===================== end Docker args

docker-push: docker-build
	docker login && docker tag ${IMAGE_NAME} ${IMAGE_FULLNAME} && docker push ${IMAGE_FULLNAME}

docker-build:
	${MAKE} \
		IMAGE_NAME=${IMAGE_NAME} \
		CONTAINER_USERNAME=${CONTAINER_USERNAME}\
		CONTAINER_GROUPNAME=${CONTAINER_GROUPNAME}\
		CONTAINER_HOMEDIR=${HOMEDIR}\
		HOMEDIR=${HOMEDIR} \
		-C docker

activate:
	python3 docker/activate_docker.py \
		--username ${CONTAINER_USERNAME} \
		--homedir ${HOMEDIR} \
		--imagename ${IMAGE_FULLNAME} \
		--hostname ${HOST_NAME}