get-deps:
	sudo apt-add-repository -y ppa:terry.guo/gcc-arm-embedded
	sudo apt-get update -qq
	sudo apt-get install -qq gcc-arm-none-eabi
