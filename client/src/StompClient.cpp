#include <iostream>
#include <thread>
#include <mutex>
#include <string>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include "../include/event.h"
#include "../include/dataBaseClient.h"
#include "../include/ConnectionHandler.h"
#include <string>
#include "../include/StompProtocol.h"
#include "../include/Task.h"

StompProtocol *stomp = new StompProtocol();

int main()
{
	// Main thread handles keyboard input
	stomp->readFromKeyboard();

	delete stomp;

	return 0;
}
