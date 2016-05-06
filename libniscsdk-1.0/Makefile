all:
	@echo "Building libSDK_VerifyRegister.so ..."
	gcc -g -shared -fPIC -DPIC _API_VerifyRegister.c md5.c cJSON.c -lm -lcurl -o libSDK_VerifyRegister.so

release:
	@echo "Building release ..."
	if [ ! -d release ];then\
		mkdir release;\
	fi
	cp libSDK_VerifyRegister.so _API_VerifyRegister.h release

clean:
	@echo "Cleaning libSDK_VerifyRegister.so ..."
	rm -fr libSDK_VerifyRegister.so
