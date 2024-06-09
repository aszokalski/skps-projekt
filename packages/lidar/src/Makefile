CXXFLAGS += -Wall -Wextra -Wpedantic

all: lidar connection

libVL53L0X_Rasp.a:
	API_DIR=../VL53L0X_API $(MAKE) -C VL53L0X_rasp


lidar: main.cpp libVL53L0X_Rasp.a
	$(CXX) -I VL53L0X_rasp/platform/inc -I VL53L0X_API/Api/core/inc $(CXXFLAGS) -o main main.cpp -LVL53L0X_rasp/bin -lVL53L0X_Rasp -lrt


connection: connection.cpp
	$(CXX) $(CXXFLAGS) -o connection connection.cpp -lrt -std=c++2a


clean:
	rm -f main connection