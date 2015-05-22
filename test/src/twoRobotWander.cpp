/*
Adept MobileRobots Robotics Interface for Applications (ARIA)
Copyright (C) 2004-2005 ActivMedia Robotics LLC
Copyright (C) 2006-2010 MobileRobots Inc.
Copyright (C) 2011-2014 Adept Technology

     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2 of the License, or
     (at your option) any later version.

     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

If you wish to redistribute ARIA under different terms, contact 
Adept MobileRobots for information about a commercial version of ARIA at 
robots@mobilerobots.com or 
Adept MobileRobots, 10 Columbia Drive, Amherst, NH 03031; +1-603-881-7960
*/


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

#include "Aria.h"
#include <stdio.h>
#include <signal.h>
#include <termios.h>
using namespace std;

#define KEYCODE_R 0x43
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_SPACE 0x20
#define KEYCODE_Q 0x71

//Console File Description
int kfd= 0;
double leftVel,rightVel;
struct termios cooked,raw;
ArRobot robot1,robot2,robot3;

void quit(int sig){
	tcsetattr(kfd,TCSANOW,&cooked);
	Aria::exit(0);
	exit(0);
}

//the loop to get the input from the keyboard
void keyloop(){
	char c;
	bool readyForChange=false;

	//get the attribute of the console
	//then change it
	tcgetattr(kfd,&cooked);
	memcpy(&raw,&cooked,sizeof (struct termios));
	//set the ICANON&ECHO bit 0
	raw.c_lflag&=~(ICANON|ECHO);

	raw.c_cc[VEOL]=1;
	raw.c_cc[VEOF]=2;

	tcsetattr(kfd,TCSANOW,&raw);
	puts("Reading from keyboard");
	puts("------------------------------");
	puts("Use arrow to move the robot.");

	while(true){
		// get next event from the keyboard


		if(read(kfd,&c,1)<0){
			perror("read():");
			exit(-1);
		}
		leftVel=rightVel=0;
		switch(c){
		case KEYCODE_L:
			puts("U press the left key.");
			leftVel=0.0;
			rightVel=100.0;
			readyForChange=true;
			break;
		case KEYCODE_R:
			puts("U press the right key.");
			leftVel=100.0;
			rightVel=0.0;
			readyForChange=true;
			break;
		case KEYCODE_U:
			puts("U press the up key.");
			leftVel=rightVel=300.0;
			readyForChange=true;
			break;
		case KEYCODE_D:
			puts("U press the down key.");
			leftVel=rightVel=-300.0;
			readyForChange=true;
			break;
		case KEYCODE_Q:
			puts("U press the quit key.");
			signal(SIGINT,quit);
			break;
		case KEYCODE_SPACE:
			puts("U press the stop key.");
//			robot1.lock();
//			robot1.unlock();
//			robot2.lock();
//			robot2.unlock();
			leftVel=rightVel;
			readyForChange=true;
			break;
		}
		if(	readyForChange){
			robot1.lock();
			robot1.setVel2(leftVel,rightVel);
			robot1.unlock();
			robot2.lock();
			robot2.setVel2(leftVel,rightVel);
			robot2.unlock();
//			robot3.lock();
//			robot3.setVel2(leftVel,rightVel);
//			robot3.unlock();
			readyForChange=false;
		}else{
			//puts("U have not pressed the arrow key. ");
			readyForChange=false;
		}
	}
}

int main(int argc, char** argv)
{

  string str;
  Aria::init();

  // get hostnames and port numbers for connecting to the robots.
  ArArgumentParser argParser(&argc, argv);
  const char* host1 = argParser.checkParameterArgument("-rh1");
  if(!host1) host1 = "localhost";
  const char* host2 = argParser.checkParameterArgument("-rh2");
  if(!host2) host2 = "localhost";
//  const char* host3 = argParser.checkParameterArgument("-rh3");
//  if(!host2) host3 = "localhost";

  int port1 = 8101;
  int port2 = 8101;
//  int port3 = 8101;
  if(strcmp(host1, host2) == 0  )
  {
    // same host, it must be using two ports (but can still override below with -rp2)
    port2++;
//    port3+=2;
  }

  bool argSet = false;
  argParser.checkParameterArgumentInteger("-rp1", &port1, &argSet);  
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp1", &port1);
  argSet = false;
  argParser.checkParameterArgumentInteger("-rp2", &port2, &argSet);
  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp2", &port2);
//  argSet = false;
//  argParser.checkParameterArgumentInteger("-rp3", &port3, &argSet);
//  if(!argSet) argParser.checkParameterArgumentInteger("-rrtp3", &port3);
  
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




  ArTcpConnection tcpConn1,tcpConn2;

  
  ArSonarDevice sonarDev1,sonarDev2;
  
  //for robot1
  tcpConn1.open(host1,port1);
  robot1.addRangeDevice(&sonarDev1);
  robot1.setDeviceConnection(&tcpConn1);
  ArPose pose;
  robot1.setEncoderPose(pose);
  robot1.setRawEncoderPose(pose);

  
  if(!robot1.blockingConnect()){
	  	  printf("Could not connect to robot 1... exiting\n");
	  	  Aria::exit(1);
	  	  return 1;
  }

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


   //robots run
   robot1.runAsync(true);
   robot2.runAsync(true);

   ArUtil::sleep(1000);

   robot2.lock();
   robot2.comInt(ArCommands::ENABLE,	1);
   robot2.unlock();
   robot1.lock();
   robot1.comInt(ArCommands::ENABLE,	1);
   robot1.unlock();

   signal(SIGINT,quit);
   keyloop();

  robot1.waitForRunExit();
  robot2.waitForRunExit();
  
  
  
  Aria::exit(0);
  return(0);
}
