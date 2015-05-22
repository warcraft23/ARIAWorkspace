#include "Aria.h"

/* Example showing how two ArRobot objects can be used in one program.
 
  Normally, a program will only connect to one robot, and will only have
  one ArRobot object.   However, it is possible to connect to more than
  one robot from the same program -- for example, this may be useful for doing 
  multirobot experiments in a simulator, or to control remote robots with
  wireless-serial bridge devices from an offboard computer. This program 
  demonstrates this by connecting to two robots over TCP connections. 
  (Old systems with dedicated serial-radio links for each robot would require 
  modifying this program to use serial connections instead.)

  Specify the hostnames of each robot with -rh1 and -rh2 command line arguments
  (defaults are "localhost" for each). Specify port numbers for each with
  -rp1 and -rp2 command line arguments (defaults are 8101 for each if 
  the hostnames are different, or 8101 and 8102 if the hostnames are the same).

  This program will just use the keybroad to control two robots running on the ground in
   the same way.

   TO DO:
   Use GPS module to get the absolute position of each robot.Then when the second robot
   comes to the position where the first one turned,it turns. It can be done with the check
   of position.
*/

/*
 * Get the real position(x,y) from SimStatPacket
 * */
//int x=0,y=0;

//bool RealXYGetterHandler(ArRobotPacket* pkt){
//	 if(	pkt->getID() != 0x62)
//		 return false; // SIMSTAT has id 0x62
//	 printf("SIMSTAT pkt received:\n"    );
//	 char a = pkt->bufToByte();  // unused byte
//	 char b = pkt->bufToByte();  // unused byte
//	 ArTypes::UByte4 flags = pkt->bufToUByte4();
//	 printf("\tFlags=0x%x\n", flags);
//	 int simint = pkt->bufToUByte2();
//	 int realint = pkt->bufToUByte2();
//	 int lastint = pkt->bufToUByte2();
//	 printf("\tSimInterval=%d, RealInterval=%d, LastInterval=%d.\n", simint, realint, lastint);
//	 //get the real coordinate x
//	 int realX = pkt->bufToByte4();
//	 x=realX;
//
//	 //get the real coordinate y
//	 int realY = pkt->bufToByte4();
//	 y=realY;
//
//
//	 int realZ = pkt->bufToByte4();
//	 int realTh = pkt->bufToByte4();
//	 printf("\tTrue Pose = (%d, %d, %d, %d)\n", realX, realY, realZ, realTh);
//	 if(flags & ArUtil::BIT1)
//	 {
//		 double lat = pkt->bufToByte4()/10e6;
//		 double lon = pkt->bufToByte4()/10e6;
//		 double alt = pkt->bufToByte4()/100;
//		 printf("\tLatitude = %f deg., Longitude = %f deg., Altitude = %f m\n", lat, lon, alt);
//	 }
//	 else
//	 {
//		 puts("No geoposition.");
//	 }
//	 return true;
//
//}


int main(int argc, char** argv)
{
//  int ret;
  std::string str;
//  int curRobot;


  Aria::init();

  // get hostnames and port numbers for connecting to the robots.
  ArArgumentParser argParser(&argc, argv);
  const char* host1 = argParser.checkParameterArgument("-rh1");
  if(!host1) host1 = "localhost";
  const char* host2 = argParser.checkParameterArgument("-rh2");
  if(!host2) host2 = "localhost";

  int port1 = 8101;
  int port2 = 8101;
  if(strcmp(host1, host2) == 0 )
  {
    // same host, it must be using two ports (but can still override below with -rp2)
    port2++;
  }

  bool argSet = false;
  argParser.checkParameterArgumentInteger("-rp1", &port1, &argSet);  
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp1", &port1);
  argSet = false;
  argParser.checkParameterArgumentInteger("-rp2", &port2, &argSet);
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp2", &port2);
  
  if(!argParser.checkHelpAndWarnUnparsed())
  {
    ArLog::log(ArLog::Terse, "Usage: twoRobotWander [-rh1 <hostname1>] [-rh2 <hostname2>] [-rp1 <port1>] [-rp2 <port2>]\n"\
      "\t<hostname1> Is the network host name of the first robot."\
          " Default is localhost (for the simulator).\n"\
      "\t<hostname2> Is the network host name of the second robot."\
          " Default is localhost (for the simulator).\n"\
      "\t<port1> Is the TCP port number of the first robot. Default is 8101.\n"\
      "\t<port2> Is the TCP port number of the second robot. Default is 8102 if"\
        " both robots have the same hostname, or 8101 if they differ.\n\n");
    return 1;
  }

//  // Global Key Handler
//  ArKeyHandler keyHandler;
//
//  //
//  // The first robot's objects
//  //
//  // the first robot connection
//  ArTcpConnection con1;
//  // the first robot
//  ArRobot robot1;
//  // sonar, must be added to the first robot
//  ArSonarDevice sonar1;
//  // the actions we'll use to wander for the first robot
//  ArActionStallRecover recover1;
//  ArActionBumpers bumpers1;
//  ArActionAvoidFront avoidFront1;
//  ArActionConstantVelocity constantVelocity1("Constant Velocity", 400);
////  double relPos1x,relPos1y;
////  int absolutePos1x,absoltePos1y;//real coordinates of robot1
//  //
//  // The second robot's objects
//  //
//  // the second robot connection
//  ArTcpConnection con2;
//  // the second robot
//  ArRobot robot2;
//  // sonar, must be added to the second robot
//  ArSonarDevice sonar2;
//  // the actions we'll use to wander for the second robot
//  ArActionStallRecover recover2;
//  ArActionBumpers bumpers2;
//  ArActionAvoidFront avoidFront2;
//  ArActionConstantVelocity constantVelocity2("Constant Velocity", 400);
////  double relPos2x,relPos2y;
////  int absolutePos2x,absoltePos2y;//real coordinates of robot2
//
//  // mandatory init
//  Aria::init();
//
//  //
//  // Lets get robot 1 going
//  //
//
//  // open the connection, if this fails exit
//  ArLog::log(ArLog::Normal, "Connecting to first robot at %s:%d...", host1, port1);
//  if ((ret = con1.open(host1, port1)) != 0)
//  {
//    str = con1.getOpenMessage(ret);
//    printf("Open failed to robot 1: %s\n", str.c_str());
//    Aria::exit(1);
//    return 1;
//  }
//
//  //ARIA set the key handler
//  Aria::setKeyHandler(&keyHandler);
//
//  // add the sonar to the robot
//  robot1.addRangeDevice(&sonar1);
//
//  // set the device connection on the robot
//  robot1.setDeviceConnection(&con1);
//
//  // try to connect, if we fail exit
//  if (!robot1.blockingConnect())
//  {
//    printf("Could not connect to robot 1... exiting\n");
//    Aria::exit(1);
//    return 1;
//  }
//
//  // turn on the motors, turn off amigobot sounds
//  robot1.comInt(ArCommands::ENABLE, 1);
//  robot1.comInt(ArCommands::SOUNDTOG, 0);
//
//  // add the actions
////  robot1.addAction(&recover1, 100);
////  robot1.addAction(&bumpers1, 75);
////  robot1.addAction(&avoidFront1, 50);
////  robot1.addAction(&constantVelocity1, 25);
//
//  // add the packet handler
////  robot1.addPacketHandler(new ArGlobalRetFunctor1<bool,ArRobotPacket *>(&RealXYGetterHandler,ArListPos::FIRST));
//
//  //robot1 attaches the key handler
//  robot1.attachKeyHandler(&keyHandler);
//
//  //
//  // Lets get robot 2 going
//  //
//
//  // open the connection, if this fails exit
//  ArLog::log(ArLog::Normal, "Connecting to second robot at %s:%d...", host2, port2);
//  if ((ret = con2.open(host2, port2)) != 0)
//  {
//    str = con2.getOpenMessage(ret);
//    printf("Open failed to robot 2: %s\n", str.c_str());
//    Aria::exit(1);
//    return 1;
//  }
//
//  // add the sonar to the robot
//  robot2.addRangeDevice(&sonar2);
//
//  // set the device connection on the robot
//  robot2.setDeviceConnection(&con2);
//
//  // try to connect, if we fail exit
//  if (!robot2.blockingConnect())
//  {
//    printf("Could not connect to robot 2... exiting\n");
//    Aria::exit(1);
//    return 1;
//  }
//
//  // turn on the motors, turn off amigobot sounds
//  robot2.comInt(ArCommands::ENABLE, 1);
//  robot2.comInt(ArCommands::SOUNDTOG, 0);
//
//  // add the actions
////  robot2.addAction(&recover2, 100);
////  robot2.addAction(&bumpers2, 75);
////  robot2.addAction(&avoidFront2, 50);
////  robot2.addAction(&constantVelocity2, 25);
//  // add the packet handler
////  robot2.addPacketHandler(new ArGlobalRetFunctor1<bool,ArRobotPacket *>(&RealXYGetterHandler,ArListPos::FIRST));
//
//  //robot2 attaches the key handler
//  robot2.attachKeyHandler(&keyHandler);
//
//  // start the robots running. true so that if we lose connection to either
//  // robot, the run stops.
//  robot1.runAsync(true);
//  robot2.runAsync(true);
//
//  	ArUtil::sleep(10000);
//
//    robot1.waitForRunExit();
//    robot2.waitForRunExit();
//  // As long as both robot loops are running, lets alternate between the
//  // two wandering around.
////  curRobot=1;
////  while (robot1.isRunning() && robot2.isRunning())
////  {
////    ArUtil::sleep(10000);
////    if (curRobot == 1)
////    {
////      robot1.stop();
//////      relPos1x= robot1.getX();
//////      relPos1y= robot1.getY();
////      ArLog::log(ArLog::Normal,"Robot1 Stops!Robot2 Please!");
////      ArLog::log(ArLog::Normal,"Robot1: My Pos is (%.2f,%.2f)",relPos1x,relPos1y);
////      robot2.clearDirectMotion();
////
////      curRobot=2;
////    }
////    else
////    {
////      robot2.stop();
//////      relPos2x= robot2.getX();
//////      relPos2y= robot2.getY();
////      ArLog::log(ArLog::Normal,"Robot2 Stops!Robot1 Please!");
////      ArLog::log(ArLog::Normal,"Robot2: My Pos is (%.2f,%.2f)",relPos1x,relPos1y);
////      robot1.clearDirectMotion();
////      curRobot=1;
////    }
////  }
////
  // now exit

  ArRobot robot1,robot2;
  ArTcpConnection tcpConn1,tcpConn2;
  ArKeyHandler keyHandler;

  Aria::setKeyHandler(&keyHandler);

  
  ArSonarDevice sonarDev1,sonarDev2;
  
  //for robot1
  tcpConn1.open(host1,port1);
  robot1.addRangeDevice(&sonarDev1);
  robot1.setDeviceConnection(&tcpConn1);
  
  if(!robot1.blockingConnect()){
	  	  printf("Could not connect to robot 1... exiting\n");
	  	  Aria::exit(1);
	  	  return 1;
  }

  robot1.attachKeyHandler(&keyHandler);
  robot1.comInt(ArCommands::ENABLE,	1);
//  robot1.enableMotors();
  robot1.comInt(ArCommands::SOUNDTOG,0);


  
  //for robot2
  tcpConn2.open(host2,port2);
  robot2.addRangeDevice(&sonarDev2);
  robot2.setDeviceConnection(&tcpConn2);

   if(!robot2.blockingConnect()){
	   	   printf("Could not connect to robot 2... exiting\n");
	       Aria::exit(1);
	       return 1;
   }
   robot1.comInt(ArCommands::SOUNDTOG,0);
   robot2.attachKeyHandler(&keyHandler);

   //robots run
   robot1.runAsync(true);
   robot2.runAsync(true);

   //ArUtil::sleep(1000);

   robot2.lock();

    ArModeTeleop teleop2(&robot2,"Teleop for Robot2",'r','R');
    teleop2.activate();
    robot2.comInt(ArCommands::ENABLE,	1);

    robot2.unlock();

		robot1.lock();

		//   ArModeSonar sonar(&robot1,"sonar",'s','S');
		ArModeTeleop teleop1(&robot1,"Teleop for Robot1",'t','T');
		teleop1.activate();
		robot1.comInt(ArCommands::ENABLE,	1);
		robot1.setVel2(30,0);
		robot1.unlock();

		robot2.lock();

		//   ArModeSonar sonar(&robot1,"sonar",'s','S');
		
		robot2.comInt(ArCommands::ENABLE,	1);
		robot2.setVel2(30,0);
		robot2.unlock();

	

  robot1.waitForRunExit();
  robot2.waitForRunExit();
  
  
  
  Aria::exit(0);
  return 0;
}
