all: main

VL53L0X_rasp/bin/libVL53L0X_Rasp.a:
	API_DIR=../VL53L0X_API $(MAKE) -C VL53L0X_rasp

main : main.cpp VL53L0X_rasp/bin/libVL53L0X_Rasp.a
	$(CXX) -I VL53L0X_API/Api/core/inc -I VL53L0X_rasp/platform/inc $(CXXFLAGS) -o main main.cpp -L VL53L0X_rasp/bin -l VL53L0X_Rasp

clean:
	rm -f *.o main
