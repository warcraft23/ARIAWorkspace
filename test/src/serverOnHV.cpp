/*
 * serverOnHV.cpp
 *
 *  Created on: 2015年5月15日
 *      Author: edward
 */

/*
 * This program is running on HV.
 * It receives the signal of command from the program clientOnPC with WIFI.
 * It gets the command from the signal and controls the HV robot with serial port.
 * It also connects to the FV with WIFI and sends the command to control it.
 * In ArNetworking System,this program acts as a server to receive the signal from PC ,
 * control the HV,and control the FV.
 * Usage: ./serverOnHV -rhHV [hostname of HV] -rpHV [port of HV] -rhFV1 [hostname of FV1] -rpFV1 [port of FV1]
 */

#include "Aria.h"
#include "ArNetworking.h"



class PrintRatio{
public:
	void printRatio(ArServerModeRatioDrive *modeRatioDrive,ArServerBase *server);
protected:
	ArActionRatioInput *ratioInput;
};
void PrintRatio::printRatio(ArServerModeRatioDrive *modeRatioDrive,ArServerBase *server){
	ratioInput=modeRatioDrive->getActionRatioInput();
	double rotRatio,transRatio;
	rotRatio=ratioInput->getRotRatio();
	transRatio=ratioInput->getTransRatio();
	printf("rotRatio:%.1f transRatio:%.1f\n",rotRatio,transRatio);
	ArUtil::sleep(1000);
}

class PrintMaxVel{
public:
	void printMaxVel(ArRobot *robot);
protected:
	ArResolver::ActionMap *actionMap;
	ArAction *moveParameterAction;
	ArResolver::ActionMap::iterator it;
	ArActionDesired *moveParameterDesired;
	double maxVel;
	double maxNegVel;
	double maxRotVel;
};
void PrintMaxVel::printMaxVel(ArRobot *robot){
	robot->lock();
	actionMap=robot->getActionMap();
	robot->unlock();
	it=actionMap->find(1);
	moveParameterAction=(*it).second;
	moveParameterDesired=moveParameterAction->getDesired();
	maxVel=moveParameterDesired->getMaxVel();
	maxNegVel=moveParameterDesired->getMaxNegVel();
	maxRotVel=moveParameterDesired->getMaxRotVel();
	printf("MAX: Vel %.1f NVel %.1f RotVel %.1f \n",maxVel,maxNegVel,maxRotVel);
	ArUtil::sleep(1000);
}

class PrintCurMaxVel{
public:
	void printCurMaxVel(ArRobot *robot);
protected:
	double transVel,rotVel;
	double maxTransVel,maxRotVel,maxNegTransVel;

};
void PrintCurMaxVel::printCurMaxVel(ArRobot *robot){
	robot->lock();
	transVel= robot->getVel();
	robot->unlock();
	robot->lock();
	rotVel=robot->getRotVel();
	robot->unlock();
	printf("Vel %.1f RotVel %.1f \n",transVel,rotVel);
	robot->lock();
	maxTransVel= robot->getAbsoluteMaxTransVel();
	robot->unlock();
	robot->lock();
	maxRotVel=robot->getAbsoluteMaxRotVel();
	robot->unlock();
	robot->lock();
	maxNegTransVel=robot->getAbsoluteMaxTransNegVel();
	robot->unlock();
	printf("MaxVel %.1f MaxNegVel %.1f RotVel %.1f \n",maxTransVel,maxNegTransVel,maxRotVel);
	ArUtil::sleep(1000);
}

class FVControler{
public:
	void forwardVelControl(ArRobot *robotSrc,ArRobot *robotDst);
protected:
	double curVelSrc,curRotVelSrc;
};
void FVControler::forwardVelControl(ArRobot *robotSrc,ArRobot *robotDst){
	//get the transformation velocity of src robot
	robotSrc->lock();
	curVelSrc=robotSrc->getVel();
	robotSrc->unlock();
	//get the rotation velocity of src robot
	robotSrc->lock();
	curRotVelSrc=robotSrc->getRotVel();
	robotSrc->unlock();
	//set the transformation velocity of dst robot
	robotDst->lock();
	robotDst->setVel(curVelSrc);
	robotDst->unlock();
	//set the rotation velocity of dst robot
	robotDst->lock();
	robotDst->setRotVel(curRotVelSrc);
	robotDst->unlock();
}

int main(int argc, char** argv) {
	Aria::init();

	//set up parser
	ArArgumentParser parser(&argc, argv);
	ArArgumentParser argParser(&argc,argv);

	//load default arguments
	parser.loadDefaultArguments();

	//robot
	ArRobot robotHV, robotFV1;

	//robotHV for HV

	//tcp connection for HV
	ArTcpConnection tcpHVConn;

	int portHV=8101;
	const char* hostHV=argParser.checkParameterArgument("-rhHV");
	if(!hostHV){
		hostHV = "localhost";
		portHV=8101;
	}

	//port Number
	bool argSetHV = false;
	argParser.checkParameterArgumentInteger("-rpHV",&portHV,&argSetHV);
	if(!argSetHV) argParser.checkParameterArgumentInteger("-rrtpHV",&portHV);

	tcpHVConn.open(hostHV,portHV);
	robotHV.setDeviceConnection(&tcpHVConn);
	if(!robotHV.blockingConnect()){
			printf("Could not Connect to RobotHV !\n");
	}

	//ArRobotConnector robotHVConnector(&parser, &robotHV);

	ArAnalogGyro gyo(&robotHV);

	/*if (!robotHVConnector.connectRobot()) {
		printf("Could not connect to robot... exiting\n");
		exit(1);
	}*/
	ArDataLogger dataLogger(&robotHV, "dataLogHV.txt");
	dataLogger.addToConfig(Aria::getConfig());

	//server
	ArServerBase serverOnHV;

	//ArLaserConnector laserHVConnector(&parser, &robotHV, &robotHVConnector);
	ArServerSimpleOpener simpleOpener(&parser);

//	ArClientSwitchManager clientSwitchManager(&serverOnHV, &parser);

// parse the command line... fail and print the help if the parsing fails
// or if the help was requested
	if (!Aria::parseArgs() || !parser.checkHelpAndWarnUnparsed()) {
		Aria::logOptions();
		Aria::exit(1);
	}

	// first open the server up
	if (!simpleOpener.open(&serverOnHV)) {
		if (simpleOpener.wasUserFileBad())
			printf("Bad user/password/permissions file\n");
		else
			printf("Could not open server port\n");
		exit(1);
	}

	ArSonarDevice sonarDevHV;
	robotHV.addRangeDevice(&sonarDevHV);




	// modes for controlling robot movement
	ArServerModeStop modeStop(&serverOnHV, &robotHV);
	ArServerModeRatioDrive modeRatioDrive(&serverOnHV, &robotHV);
	modeStop.addAsDefaultMode();
	modeStop.activate();

	// set up the simple commands
	ArServerHandlerCommands commands(&serverOnHV);
	ArServerSimpleComUC uCCommands(&commands, &robotHV); // send commands directly to microcontroller
	ArServerSimpleComMovementLogging loggingCommands(&commands, &robotHV); // control debug logging

	ArServerHandlerConfig serverHandlerConfig(&serverOnHV, Aria::getConfig()); // make a config handler
	ArLog::addToConfig(Aria::getConfig()); // let people configure logging

	modeRatioDrive.addToConfig(Aria::getConfig(), "Teleop settings"); // able to configure teleop settings
	modeRatioDrive.addControlCommands(&commands);

	/////////////////////////////////////////////////////////////////////
	//          		RobotFV1
	/////////////////////////////////////////////////////////////////////


	int portFV1=8101;

	//hostname of FV1
	const char* hostFV1=argParser.checkParameterArgument("-rhFV1");
	if(!hostFV1) {
		hostFV1 = "localhost";
		portFV1=8102;
	}

	//port number
	bool argSet = false;
	argParser.checkParameterArgumentInteger("-rpFV1",&portFV1,&argSet);
	if(!argSet) argParser.checkParameterArgumentInteger("-rrtp",&portFV1);

	//tcp connection for FV1
	ArTcpConnection tcpFVConn1;

	ArSonarDevice sonarDeviceFV1;

	//connect robotFV1
	tcpFVConn1.open(hostFV1,portFV1);
	robotFV1.addRangeDevice(&sonarDeviceFV1);
	robotFV1.setDeviceConnection(&tcpFVConn1);

	if(!robotFV1.blockingConnect()){
		printf("Could not Conncect to RobotFV1 !\n");
	}

	robotFV1.comInt(ArCommands::SOUNDTOG,0);

	//////////////////////////////////////////////////////
	//   	RobotFV1 ends
	/////////////////////////////////////////////////////

	robotHV.runAsync(true);
	robotFV1.runAsync(true);

	// log whatever we wanted to before the runAsync
	simpleOpener.checkAndLog();

//	PrintRatio printRatio;
//	ArFunctor2C<PrintRatio,ArServerModeRatioDrive *,ArServerBase *> printRatioCB(printRatio,&PrintRatio::printRatio,&modeRatioDrive,&serverOnHV);
//	serverOnHV.addCycleCallback(&printRatioCB);
//	PrintMaxVel printMaxVel;
//	ArFunctor1C<PrintMaxVel,ArRobot *> printMaxVelCB(printMaxVel,&PrintMaxVel::printMaxVel,&robotHV);
//	serverOnHV.addCycleCallback(&printMaxVelCB);
//	PrintCurMaxVel printCMVel;
//	ArFunctor1C<PrintCurMaxVel,ArRobot *> printCMVelCB(printCMVel,&PrintCurMaxVel::printCurMaxVel,&robotHV);
//	serverOnHV.addCycleCallback(&printCMVelCB);
	FVControler controler;
	ArFunctor2C<FVControler,ArRobot *,ArRobot *> controlerCB(controler,&FVControler::forwardVelControl,&robotHV,&robotFV1);
	serverOnHV.addCycleCallback(&controlerCB);


	// now let it spin off in its own thread
	serverOnHV.runAsync();

	printf("Server on HV is now running...\n");

	// Add a key handler so that you can exit by pressing
	// escape. Note that a key handler prevents you from running
	// a program in the background on Linux, since it expects an
	// active terminal to read keys from; remove this if you want
	// to run it in the background.
	ArKeyHandler *keyHandler;
	if ((keyHandler = Aria::getKeyHandler()) == NULL) {
		keyHandler = new ArKeyHandler;
		Aria::setKeyHandler(keyHandler);
		robotHV.lock();
		robotHV.attachKeyHandler(keyHandler);
		robotHV.unlock();
		printf("To exit, press escape.\n");
	}
	robotHV.lock();
	robotHV.enableMotors();
	robotHV.unlock();

	robotFV1.lock();
	robotFV1.enableMotors();
	robotFV1.unlock();

	robotHV.waitForRunExit();
	robotFV1.waitForRunExit();

	//robotFV1 for FV1
	Aria::exit(0);
	return 0;
}
