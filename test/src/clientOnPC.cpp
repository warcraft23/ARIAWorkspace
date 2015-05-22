/*
 * clientOnPC.cpp
 *
 *  Created on: 2015年5月15日
 *      Author: edward
 */

/*
 * This program is running on PC acting as a controller to control the HV.
 * It sends the signal to the HV and then the HV will go as the command said.
 * In ArNetworking System,it is the client connecting to the server on HV.
 * usage: ./clientOnPC -rh [hostname of HV] -rp [port of HV]
 */

#include "Aria.h"
#include "ArNetworking.h"

class InputHandler{
public:
		/*
		 * @param client  Our client networking object
		 * @param keyHandler  Key handler to register command callbacks with
		 */
	  InputHandler(ArClientBase *client, ArKeyHandler *keyHandler);
	  virtual ~InputHandler(void);

	  /// Up arrow key handler: drive the robot forward
	  void up(void);

	  /// Down arrow key handler: drive the robot backward
	  void down(void);

	  /// Left arrow key handler: turn the robot left
	  void left(void);

	  /// Right arrow key handler: turn the robot right
	  void right(void);

	  /// Move the robot laterally right  (q key)
	  void lateralLeft(void);

	  /// Move the robot laterally right  (e key)
	  void lateralRight(void);

	  /// Stop the robot
	  void stop(void);

	  /// Send drive request to the server with stored values
	  void sendInput(void);

	  /// Send a request to enable "safe drive" mode on the server
	  void safeDrive();

	  /// Send a request to disable "safe drive" mode on the server
	  void unsafeDrive();

	  // Change the value of myPrinting
	  void debugChange();

	  void listData();

	  void logTrackingTerse();
	  void logTrackingVerbose();
	  void resetTracking();
	protected:
	  ArClientBase *myClient;
	  ArKeyHandler *myKeyHandler;

	  /// Set this to true in the constructor to print out debugging information
	  bool myPrinting;

	  /// Current translation value (a percentage of the  maximum speed)
	  double myTransRatio;

	  /// Current rotation ration value (a percentage of the maximum rotational velocity)
	  double myRotRatio;

	  /// Current rotation ration value (a percentage of the maximum rotational velocity)
	  double myLatRatio;

	  /** Functor objects, given to the key handler, which then call our handler
	   * methods above */
	  ///@{
	  ArFunctorC<InputHandler> myUpCB;
	  ArFunctorC<InputHandler> myDownCB;
	  ArFunctorC<InputHandler> myLeftCB;
	  ArFunctorC<InputHandler> myRightCB;
	  ArFunctorC<InputHandler> myStopCB;
	  ArFunctorC<InputHandler> myLateralLeftCB;
	  ArFunctorC<InputHandler> myLateralRightCB;
	  ArFunctorC<InputHandler> mySafeDriveCB;
	  ArFunctorC<InputHandler> myUnsafeDriveCB;
	  ArFunctorC<InputHandler> myListDataCB;
	  ArFunctorC<InputHandler> myLogTrackingTerseCB;
	  ArFunctorC<InputHandler> myLogTrackingVerboseCB;
	  ArFunctorC<InputHandler> myResetTrackingCB;
	  ArFunctorC<InputHandler> myDebugChangingCB;

	  ///@}
};

InputHandler::InputHandler(ArClientBase *client,ArKeyHandler *keyHandler) :
		myClient(client),
		myKeyHandler(keyHandler),
		myPrinting(false),
		myTransRatio(0.0),
		myRotRatio(0.0),
		myLatRatio(0.0),
		myUpCB(this,&InputHandler::up),
		myLeftCB(this,&InputHandler::left),
		myRightCB(this,&InputHandler::right),
		myLateralLeftCB(this, &InputHandler::lateralLeft),
		myLateralRightCB(this, &InputHandler::lateralRight),
		mySafeDriveCB(this, &InputHandler::safeDrive),
		myUnsafeDriveCB(this, &InputHandler::unsafeDrive),
		myListDataCB(this, &InputHandler::listData),
		myLogTrackingTerseCB(this, &InputHandler::logTrackingTerse),
		myLogTrackingVerboseCB(this, &InputHandler::logTrackingVerbose),
		myResetTrackingCB(this, &InputHandler::resetTracking),
		myDebugChangingCB(this,&InputHandler::debugChange),
		myStopCB(this,&InputHandler::stop),
		myDownCB(this,&InputHandler::down)
	{
	/* Add our functor objects to the key handler, associated with the appropriate
	   * keys: */
	  myKeyHandler->addKeyHandler(ArKeyHandler::UP, &myUpCB);
	  myKeyHandler->addKeyHandler(ArKeyHandler::DOWN, &myDownCB);
	  myKeyHandler->addKeyHandler(ArKeyHandler::LEFT, &myLeftCB);
	  myKeyHandler->addKeyHandler(ArKeyHandler::RIGHT, &myRightCB);
	  myKeyHandler->addKeyHandler(ArKeyHandler::SPACE,&myStopCB);
	  myKeyHandler->addKeyHandler('q', &myLateralLeftCB);
	  myKeyHandler->addKeyHandler('e', &myLateralRightCB);
	  myKeyHandler->addKeyHandler('s', &mySafeDriveCB);
	  myKeyHandler->addKeyHandler('u', &myUnsafeDriveCB);
	  myKeyHandler->addKeyHandler('l', &myListDataCB);
	  myKeyHandler->addKeyHandler('t', &myLogTrackingTerseCB);
	  myKeyHandler->addKeyHandler('v', &myLogTrackingVerboseCB);
	  myKeyHandler->addKeyHandler('r', &myResetTrackingCB);
	  myKeyHandler->addKeyHandler('c',&myDebugChangingCB);
	}

InputHandler::~InputHandler(void){}

void InputHandler::up(void){
	if(myPrinting){
		printf("You Press Forward!\n");
	}
	myTransRatio=100;
}

void InputHandler::down(void){
	if(myPrinting){
		printf("You Press Backward!\n");
	}
	myTransRatio=-100;
}

void InputHandler::left(void){
	if(myPrinting){
		printf("You Press Left!\n");
	}
	myRotRatio=100;
}

void InputHandler::right(void){
	if(myPrinting){
		printf("You Press Right\n!");
	}
	myRotRatio=-100;
}

void InputHandler::stop(void){
	if(myPrinting){
			printf("You Press Stop\n!");
	}
	myTransRatio=0;
	myRotRatio=0;
	myLatRatio=0;
}

void InputHandler::lateralLeft(void){
	if(myPrinting){
		printf("You Press lateralLeft!\n");
	}
	myLatRatio=100;
}

void InputHandler::lateralRight(void){
	if(myPrinting){
		printf("You Press lateralRight!\n");
	}
	myLatRatio=-100;
}

void InputHandler::safeDrive()
{
  /* Construct a request packet. The data is a single byte, with value
   * 1 to enable safe drive, 0 to disable. */
  ArNetPacket p;
  p.byteToBuf(1);

  /* Send the packet as a single request: */
  if(myPrinting)
    printf("Sending setSafeDrive 1.\n");
  myClient->requestOnce("setSafeDrive",&p);
  if(myPrinting)
    printf("\nSent enable safe drive.\n");
}

void InputHandler::unsafeDrive()
{
  /* Construct a request packet. The data is a single byte, with value
   * 1 to enable safe drive, 0 to disable. */
  ArNetPacket p;
  p.byteToBuf(0);

  /* Send the packet as a single request: */
  if(myPrinting)
    printf("Sending setSafeDrive 0.\n");
  myClient->requestOnce("setSafeDrive",&p);
  if(myPrinting)
    printf("\nSent disable safe drive command. Your robot WILL run over things if you're not careful.\n");
}

void InputHandler::debugChange(){
	if(myPrinting){
		printf("myPrinting changes to FALSE\n");
		myPrinting=false;
	}else{
		printf("myPrinting changes to TRUE\n");
		myPrinting=true;
	}
}

void InputHandler::sendInput(void){
		/* This method is called by the main function to send a ratioDrive
	   * request with our current velocity values. If the server does
	   * not support the ratioDrive request, then we abort now: */
	  if(!myClient->dataExists("ratioDrive")) return;

	  /* Construct a ratioDrive request packet.  It consists
	   * of three doubles: translation ratio, rotation ratio, and an overall scaling
	   * factor. */
	  ArNetPacket packet;
	  packet.doubleToBuf(myTransRatio);
	  packet.doubleToBuf(myRotRatio);
	  packet.doubleToBuf(50); // use half of the robot's maximum.
	  packet.doubleToBuf(myLatRatio);
	  if (myPrinting)
	    printf("Sending\n");
	  myClient->requestOnce("ratioDrive", &packet);
//	  myTransRatio = 0;
//	  myRotRatio = 0;
//	  myLatRatio = 0;

}
void InputHandler::listData()
{
  myClient->logDataList();
}

void InputHandler::logTrackingTerse()
{
  myClient->logTracking(true);
}

void InputHandler::logTrackingVerbose()
{
  myClient->logTracking(false);
}

void InputHandler::resetTracking()
{
  myClient->resetTracking();
}
void escape(){
	printf("esc pressed, shutting down aria\n");
	Aria::shutdown();
}

int main(int argc, char** argv) {
	//init Aria
	Aria::init();

	//ClientBase
	ArClientBase client;

	/* Aria components use this to get options off the command line: */
	ArArgumentParser parser(&argc, argv);
//	ArArgumentParser parser(&argc, argv);

	/* This will be used to connect our client to the server, including
	 * various bits of handshaking (e.g. sending a password, retrieving a list
	 * of data requests and commands...)
	 * It will get the hostname from the -host command line argument: */
	ArClientSimpleConnector clientConnector(&parser);

	parser.loadDefaultArguments();

	/* Check for -help, and unhandled arguments: */
	if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed()) {
		Aria::logOptions();
		exit(0);
	}

	/* Connect our client object to the remote server: */
	if (!clientConnector.connectClient(&client)) {
		if (client.wasRejected())
			printf("Server '%s' rejected connection, exiting\n",
					client.getHost());
		else
			printf("Could not connect to server '%s', exiting\n",
					client.getHost());
		exit(1);
	}
	printf("Connected to server.\n");

	client.setRobotName(client.getHost()); // include server name in log messages

	ArKeyHandler keyHandler;
	Aria::setKeyHandler(&keyHandler);

	ArGlobalFunctor escapeCB(&escape);
	keyHandler.addKeyHandler(ArKeyHandler::ESCAPE,&escapeCB);

	client.runAsync();

	InputHandler inputHandler(&client,&keyHandler);
	inputHandler.unsafeDrive();

	/* Use ArClientBase::dataExists() to see if the "ratioDrive" request is available on the
	   * currently connected server.  */
	  if(!client.dataExists("ratioDrive") )
	      printf("Warning: server does not have ratioDrive command, can not use drive commands!\n");
	  else
	    printf("Keys are:\nUP: Forward\nDOWN: Backward\nLEFT: Turn Left\nRIGHT: Turn Right\nSPACE: Stop\n");
	  printf("s: Enable safe drive mode (if supported).\nu: Disable safe drive mode (if supported).\nl: list all data requests on server\n\nDrive commands use 'ratioDrive'.\nt: logs the network tracking tersely\nv: logs the network tracking verbosely\nr: resets the network tracking\nc: change the value to show the debuf information.\n\n");

	  while (client.getRunningWithLock())
	    {
	      keyHandler.checkKeys();
	      inputHandler.sendInput();
	      ArUtil::sleep(10);
	    }

	/* The client stopped running, due to disconnection from the server, general
	 * Aria shutdown, or some other reason. */
	client.stopRunning();
	Aria::shutdown();

	return 0;
}

