/****************************************************************
 *
 * Copyright (c) 2011
 * All rights reserved.
 *
 * Hochschule Bonn-Rhein-Sieg
 * University of Applied Sciences
 * Computer Science Department
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Author:
 * Jan Paulus, Nico Hochgeschwender, Michael Reckhaus, Azamat Shakhimardanov
 * Supervised by:
 * Gerhard K. Kraetzschmar
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * This sofware is published under a dual-license: GNU Lesser General Public 
 * License LGPL 2.1 and BSD license. The dual-license implies that users of this
 * code may choose which terms they prefer.
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Hochschule Bonn-Rhein-Sieg nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version or the BSD license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL and the BSD license for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License LGPL and BSD license along with this program.
 *
 ****************************************************************/
extern "C" {
#include "ethercattype.h"
#include "nicdrv.h"
#include "ethercatbase.h"
#include "ethercatmain.h"
#include "ethercatconfig.h"
#include "ethercatcoe.h"
#include "ethercatdc.h"
#include "ethercatprint.h"
}
#include "youbot/EthercatMasterWithoutThread.hpp"

namespace youbot {

EthercatMaster* EthercatMaster::instance = 0;
EthercatMaster::EthercatMaster() {
  // Bouml preserved body begin 000D1AF1


    ethernetDevice = "eth0";
    mailboxTimeout = 4000; //micro sec
    ethercatTimeout = 500; //micro sec
    configfile = NULL;

    //initialize to zero
    for (unsigned int i = 0; i < 4096; i++) {
      IOmap_[i] = 0;
    }
    //read ethercat parameters form config file
    configfile = new ConfigFile(this->configFileName, this->configFilepath);

    // configfile.setSection("EtherCAT");
    configfile->readInto(ethernetDevice, "EtherCAT", "EthernetDevice");
    configfile->readInto(ethercatTimeout, "EtherCAT", "EtherCATTimeout_[usec]");
    configfile->readInto(mailboxTimeout, "EtherCAT", "MailboxTimeout_[usec]");



  // Bouml preserved body end 000D1AF1
}

EthercatMaster::~EthercatMaster() {
  // Bouml preserved body begin 000D1BF1
    this->closeEthercat();
    if (configfile != NULL)
      delete configfile;
  // Bouml preserved body end 000D1BF1
}

///creates a instance of the singleton EthercatMaster if there is none and returns a reference to it
///@param configFile configuration file name incl. the extension
///@param configFilePath the path where the configuration is located with a / at the end
EthercatMaster& EthercatMaster::getInstance(const std::string configFile, const std::string configFilePath)
{
  // Bouml preserved body begin 000D1C71
    if (!instance) {
      configFileName = configFile;
      configFilepath = configFilePath;
      instance = new EthercatMaster();
      instance->initializeEthercat();

    }
    return *instance;

  // Bouml preserved body end 000D1C71
}

/// destroy the singleton instance by calling the destructor
void EthercatMaster::destroy()
{
  // Bouml preserved body begin 000D1CF1
    if (instance) {
      delete instance;
    }
    instance = NULL;

  // Bouml preserved body end 000D1CF1
}

///return the quantity of ethercat slave which have an input/output buffer
unsigned int EthercatMaster::getNumberOfSlaves() const {
  // Bouml preserved body begin 000D1D71
    return this->nrOfSlaves;
  // Bouml preserved body end 000D1D71
}

void EthercatMaster::AutomaticSendOn(const bool enableAutomaticSend) {
  // Bouml preserved body begin 000D1DF1
    LOG(trace) << "automatic send is not possible if the EtherCAT master has no thread";

    return;
  // Bouml preserved body end 000D1DF1
}

void EthercatMaster::AutomaticReceiveOn(const bool enableAutomaticReceive) {
  // Bouml preserved body begin 000D1E71
    LOG(trace) << "automatic receive is not possible if the EtherCAT master has no thread";
    return;
  // Bouml preserved body end 000D1E71
}

///provides all ethercat slave informations from the SOEM driver
///@param ethercatSlaveInfos ethercat slave informations
void EthercatMaster::getEthercatDiagnosticInformation(std::vector<ec_slavet>& ethercatSlaveInfos) {
  // Bouml preserved body begin 000D1EF1
    ethercatSlaveInfos = this->ethercatSlaveInfo;
    for (unsigned int i = 0; i < ethercatSlaveInfos.size(); i++) {
      ethercatSlaveInfos[i].inputs = NULL;
      ethercatSlaveInfos[i].outputs = NULL;
    }
  // Bouml preserved body end 000D1EF1
}

///sends ethercat messages to the motor controllers
/// returns a true if everything it OK and returns false if something fail
bool EthercatMaster::sendProcessData() {
  // Bouml preserved body begin 000D2471

    for (unsigned int i = 0; i < processDataBuffer.size(); i++) {
      //fill output buffer (send data)
      *(ethercatOutputBufferVector[i]) = (processDataBuffer[i]).stctOutput;
    }

    //send data to ethercat
    if (ec_send_processdata() == 0) {
      return false;
    }
    
    return true;

  // Bouml preserved body end 000D2471
}

/// receives ethercat messages from the motor controllers
/// returns a true if everything it OK and returns false if something fail
bool EthercatMaster::receiveProcessData() {
  // Bouml preserved body begin 000D5D71

    //receive data from ethercat
    if (ec_receive_processdata(this->ethercatTimeout) == 0) {
      return false;
    }

    for (unsigned int i = 0; i < processDataBuffer.size(); i++) {
      //fill input buffer (receive data)
      (processDataBuffer[i]).stctInput = *(ethercatInputBufferVector[i]);
    }

    return true;

  // Bouml preserved body end 000D5D71
}

/// checks if an error has occurred in the soem driver
/// returns a true if an error has occurred
bool EthercatMaster::isErrorInSoemDriver() {
  // Bouml preserved body begin 000D5DF1
   
    return ec_iserror();

  // Bouml preserved body end 000D5DF1
}

///establishes the ethercat connection
void EthercatMaster::initializeEthercat() {
  // Bouml preserved body begin 000D1F71

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ethernetDevice.c_str())) {
      LOG(info) << "Initializing EtherCAT on " << ethernetDevice;
      /* find and auto-config slaves */
      if (ec_config(TRUE, &IOmap_) > 0) {

        LOG(trace) << ec_slavecount << " slaves found and configured.";

        /* wait for all slaves to reach Pre OP state */
        /*ec_statecheck(0, EC_STATE_PRE_OP,  EC_TIMEOUTSTATE);
        if (ec_slave[0].state != EC_STATE_PRE_OP ){
        printf("Not all slaves reached pre operational state.\n");
        ec_readstate();
        //If not all slaves operational find out which one
          for(int i = 1; i<=ec_slavecount ; i++){
            if(ec_slave[i].state != EC_STATE_PRE_OP){
              printf("Slave %d State=%2x StatusCode=%4x : %s\n",
              i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
            }
          }
        }
         */

        /* distributed clock is not working
        //Configure distributed clock
        if(!ec_configdc()){
          LOG(warning) << "no distributed clock is available";
        }else{

          uint32 CyclTime = 4000000;
          uint32 CyclShift = 0;
          for (int i = 1; i <= ec_slavecount; i++) {
            ec_dcsync0(i, true, CyclTime, CyclShift);
          }

        }
         */

        /* wait for all slaves to reach SAFE_OP state */
        ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
        if (ec_slave[0].state != EC_STATE_SAFE_OP) {
          LOG(warning) << "Not all slaves reached safe operational state.";
          ec_readstate();
          //If not all slaves operational find out which one
          for (int i = 1; i <= ec_slavecount; i++) {
            if (ec_slave[i].state != EC_STATE_SAFE_OP) {
              LOG(info) << "Slave " << i << " State=" << ec_slave[i].state << " StatusCode=" << ec_slave[i].ALstatuscode << " : " << ec_ALstatuscode2string(ec_slave[i].ALstatuscode);

            }
          }
        }


        //Read the state of all slaves
        //ec_readstate();

        LOG(trace) << "Request operational state for all slaves";

        ec_slave[0].state = EC_STATE_OPERATIONAL;
        // request OP state for all slaves
        /* send one valid process data to make outputs in slaves happy*/
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        /* request OP state for all slaves */
        ec_writestate(0);
        // wait for all slaves to reach OP state

        ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);
        if (ec_slave[0].state == EC_STATE_OPERATIONAL) {
          LOG(trace) << "Operational state reached for all slaves.";
        } else {
          throw std::runtime_error("Not all slaves reached operational state.");

        }

      } else {
        throw std::runtime_error("No slaves found!");
      }

    } else {
      throw std::runtime_error("No socket connection on " + ethernetDevice + "\nExcecute as root");
    }



    std::string baseJointControllerName = "TMCM-174";
    std::string manipulatorJointControllerName = "TMCM-174";
    std::string actualSlaveName;
    nrOfSlaves = 0;
    YouBotSlaveMsg emptySlaveMsg;


    configfile->readInto(baseJointControllerName, "BaseJointControllerName");
    configfile->readInto(manipulatorJointControllerName, "ManipulatorJointControllerName");

    //reserve memory for all slave with a input/output buffer
    for (int cnt = 1; cnt <= ec_slavecount; cnt++) {
      //     printf("Slave:%d Name:%s Output size:%3dbits Input size:%3dbits State:%2d delay:%d.%d\n",
      //             cnt, ec_slave[cnt].name, ec_slave[cnt].Obits, ec_slave[cnt].Ibits,
      //             ec_slave[cnt].state, (int) ec_slave[cnt].pdelay, ec_slave[cnt].hasdc);

      ethercatSlaveInfo.push_back(ec_slave[cnt]);

      actualSlaveName = ec_slave[cnt].name;
      if ((actualSlaveName == baseJointControllerName || actualSlaveName == manipulatorJointControllerName) && ec_slave[cnt].Obits > 0 && ec_slave[cnt].Ibits > 0) {
        nrOfSlaves++;
        //   joints.push_back(YouBotJoint(nrOfSlaves));

        processDataBuffer.push_back(emptySlaveMsg);
        ethercatOutputBufferVector.push_back((SlaveMessageOutput*) (ec_slave[cnt].outputs));
        ethercatInputBufferVector.push_back((SlaveMessageInput*) (ec_slave[cnt].inputs));
        YouBotSlaveMailboxMsg emptyMailboxSlaveMsg(cnt);
        firstMailboxBufferVector.push_back(emptyMailboxSlaveMsg);
        int i = 0;
        bool b = false;
        upperLimit.push_back(i);
        lowerLimit.push_back(i);
        limitActive.push_back(b);
        jointLimitReached.push_back(b);
        inverseMovementDirection.push_back(b);


      }
    }



    if (nrOfSlaves > 0) {
      LOG(info) << nrOfSlaves << " EtherCAT slaves found";
    } else {
      throw std::runtime_error("No EtherCAT slave could be found");
      return;
    }


    return;
  // Bouml preserved body end 000D1F71
}

void EthercatMaster::setJointLimits(const int lowerJointLimit, const int upperJointLimit, const bool inverseMovement, const bool activateLimit, const unsigned int& jointNumber) {
  // Bouml preserved body begin 000D1FF1
    upperLimit[jointNumber - 1] = upperJointLimit;
    lowerLimit[jointNumber - 1] = lowerJointLimit;
    limitActive[jointNumber - 1] = activateLimit;
    inverseMovementDirection[jointNumber - 1] = inverseMovement;

  // Bouml preserved body end 000D1FF1
}

///closes the ethercat connection
bool EthercatMaster::closeEthercat() {
  // Bouml preserved body begin 000D2071


    // Request safe operational state for all slaves
    ec_slave[0].state = EC_STATE_SAFE_OP;

    /* request SAFE_OP state for all slaves */
    ec_writestate(0);

    //stop SOEM, close socket
    ec_close();

    return true;
  // Bouml preserved body end 000D2071
}

///stores a ethercat message to the buffer
///@param msgBuffer ethercat message
///@param jointNumber joint number of the sender joint
void EthercatMaster::setMsgBuffer(const YouBotSlaveMsg& msgBuffer, const unsigned int jointNumber) {
  // Bouml preserved body begin 000D20F1

    processDataBuffer[jointNumber - 1].stctOutput = msgBuffer.stctOutput;

  // Bouml preserved body end 000D20F1
}

///get a ethercat message form the buffer
///@param msgBuffer ethercat message
///@param jointNumber joint number of the receiver joint
YouBotSlaveMsg EthercatMaster::getMsgBuffer(const unsigned int jointNumber) {
  // Bouml preserved body begin 000D2171

    YouBotSlaveMsg returnMsg;
    returnMsg = processDataBuffer[jointNumber - 1];
    return returnMsg;
  // Bouml preserved body end 000D2171
}

///stores a mailbox message in a buffer which will be sent to the motor controllers
///@param msgBuffer ethercat mailbox message
///@param jointNumber joint number of the sender joint
void EthercatMaster::setMailboxMsgBuffer(const YouBotSlaveMailboxMsg& msgBuffer, const unsigned int jointNumber) {
  // Bouml preserved body begin 000D21F1
  
    firstMailboxBufferVector[jointNumber - 1].stctOutput = msgBuffer.stctOutput;
    sendMailboxMessage(firstMailboxBufferVector[jointNumber - 1]);
    return;
  // Bouml preserved body end 000D21F1
}

///gets a mailbox message form the buffer which came form the motor controllers
///@param msgBuffer ethercat mailbox message
///@param jointNumber joint number of the receiver joint
bool EthercatMaster::getMailboxMsgBuffer(YouBotSlaveMailboxMsg& mailboxMsg, const unsigned int jointNumber) {
  // Bouml preserved body begin 000D2271
    bool returnvalue = receiveMailboxMessage(firstMailboxBufferVector[jointNumber - 1]);
    mailboxMsg.stctInput = firstMailboxBufferVector[jointNumber - 1].stctInput;
    return returnvalue;
  // Bouml preserved body end 000D2271
}

///sends the mailbox Messages which have been stored in the buffer
///@param mailboxMsg ethercat mailbox message
bool EthercatMaster::sendMailboxMessage(const YouBotSlaveMailboxMsg& mailboxMsg) {
  // Bouml preserved body begin 000D22F1
    mailboxBufferSend[0] = mailboxMsg.stctOutput.moduleAddress;
    mailboxBufferSend[1] = mailboxMsg.stctOutput.commandNumber;
    mailboxBufferSend[2] = mailboxMsg.stctOutput.typeNumber;
    mailboxBufferSend[3] = mailboxMsg.stctOutput.motorNumber;
    mailboxBufferSend[4] = mailboxMsg.stctOutput.value >> 24;
    mailboxBufferSend[5] = mailboxMsg.stctOutput.value >> 16;
    mailboxBufferSend[6] = mailboxMsg.stctOutput.value >> 8;
    mailboxBufferSend[7] = mailboxMsg.stctOutput.value & 0xff;
    if (ec_mbxsend(mailboxMsg.getSlaveNo(), &mailboxBufferSend, mailboxTimeout)) {
      return true;
    } else {
      return false;
    }
  // Bouml preserved body end 000D22F1
}

///receives mailbox messages and stores them in the buffer
///@param mailboxMsg ethercat mailbox message
bool EthercatMaster::receiveMailboxMessage(YouBotSlaveMailboxMsg& mailboxMsg) {
  // Bouml preserved body begin 000D2371
    if (ec_mbxreceive(mailboxMsg.getSlaveNo(), &mailboxBufferReceive, mailboxTimeout)) {
      mailboxMsg.stctInput.replyAddress = (int) mailboxBufferReceive[0];
      mailboxMsg.stctInput.moduleAddress = (int) mailboxBufferReceive[1];
      mailboxMsg.stctInput.status = (int) mailboxBufferReceive[2];
      mailboxMsg.stctInput.commandNumber = (int) mailboxBufferReceive[3];
      mailboxMsg.stctInput.value = (mailboxBufferReceive[4] << 24 | mailboxBufferReceive[5] << 16 | mailboxBufferReceive[6] << 8 | mailboxBufferReceive[7]);
      return true;
    }
    return false;
  // Bouml preserved body end 000D2371
}

void EthercatMaster::checkJointLimits() {
  // Bouml preserved body begin 000D23F1
    SlaveMessageOutput* jointMsgOut;
    SlaveMessageInput* jointMsgIN;

    for (unsigned int jointNo = 0; jointNo < processDataBuffer.size(); jointNo++) {
      jointMsgOut = ethercatOutputBufferVector[jointNo];
      jointMsgIN = ethercatInputBufferVector[jointNo];

      //check if for joint limits
      bool limitReached = false;
      bool reachedUpperLimit = false;
      bool reachedLowerLimit = false;

      if (limitActive[jointNo]) {

        if (jointMsgIN->actualPosition > upperLimit[jointNo]) {
          reachedUpperLimit = true;
        }
        if (jointMsgIN->actualPosition < lowerLimit[jointNo]) {
          reachedLowerLimit = true;
        }

        if (reachedUpperLimit || reachedLowerLimit) {

          limitReached = true;

          switch (jointMsgOut->controllerMode) {
            case POSITION_CONTROL:
              if (!(jointMsgOut->value < upperLimit[jointNo] && jointMsgOut->value > lowerLimit[jointNo])) {
                limitReached = true;
              } else {
                limitReached = false;
              }
              break;
            case VELOCITY_CONTROL:
            case PWM_MODE:
            case CURRENT_MODE:
              if ((jointMsgOut->value >= 0 && reachedUpperLimit) || (jointMsgOut->value <= 0 && reachedLowerLimit)) {
                limitReached = true;
              } else {
                limitReached = false;
              }
              break;
            default:
              limitReached = true;
              break;

          }
        }

        if (limitReached == true) {

          jointMsgOut->controllerMode = PWM_MODE;
          jointMsgOut->value = 0;
          //    LOG(info) << "limit reached Joint " << jointNo + 1;

          if (jointLimitReached[jointNo] == false) {
            LOG(error) << "Joint " << jointNo + 1 << " exceeded the joint limit! Upper limit: " << upperLimit[jointNo] << " lower limit: " << lowerLimit[jointNo] << " position: " << jointMsgIN->actualPosition;
            //       throw std::runtime_error(ss.str());
            jointLimitReached[jointNo] = true;
          }
        } else {
          if (jointLimitReached[jointNo] == true) {
            LOG(info) << "Joint " << jointNo + 1 << " is not in the limit any more";
          }
          jointLimitReached[jointNo] = false;

        }
      }
    }
  // Bouml preserved body end 000D23F1
}

void EthercatMaster::parseYouBotErrorFlags(const YouBotSlaveMsg& messageBuffer) {
  // Bouml preserved body begin 000D24F1
    std::stringstream errorMessageStream;
    errorMessageStream << " ";
    std::string errorMessage;
    errorMessage = errorMessageStream.str();


    if (messageBuffer.stctInput.errorFlags & OVER_CURRENT) {
      LOG(error) << errorMessage << "got over current";
      //    throw JointErrorException(errorMessage + "got over current");
    }

    if (messageBuffer.stctInput.errorFlags & UNDER_VOLTAGE) {
      LOG(error) << errorMessage << "got under voltage";
      //    throw JointErrorException(errorMessage + "got under voltage");
    }

    if (messageBuffer.stctInput.errorFlags & OVER_VOLTAGE) {
      LOG(error) << errorMessage << "got over voltage";
      //   throw JointErrorException(errorMessage + "got over voltage");
    }

    if (messageBuffer.stctInput.errorFlags & OVER_TEMPERATURE) {
      LOG(error) << errorMessage << "got over temperature";
      //   throw JointErrorException(errorMessage + "got over temperature");
    }

    if (messageBuffer.stctInput.errorFlags & MOTOR_HALTED) {
      //   LOG(info) << errorMessage << "is halted";
      //   throw JointErrorException(errorMessage + "is halted");
    }

    if (messageBuffer.stctInput.errorFlags & HALL_SENSOR_ERROR) {
      LOG(error) << errorMessage << "got hall sensor problem";
      //   throw JointErrorException(errorMessage + "got hall sensor problem");
    }

    //    if (messageBuffer.stctInput.errorFlags & ENCODER_ERROR) {
    //      LOG(error) << errorMessage << "got encoder problem";
    //      //   throw JointErrorException(errorMessage + "got encoder problem");
    //    }
    //
    //     if (messageBuffer.stctInput.errorFlags & INITIALIZATION_ERROR) {
    //      LOG(error) << errorMessage << "got inizialization problem";
    //      //   throw JointErrorException(errorMessage + "got motor winding problem");
    //    }

    if (messageBuffer.stctInput.errorFlags & PWM_MODE_ACTIVE) {
      //  LOG(error) << errorMessage << "has PWM mode active";
      //   throw JointErrorException(errorMessage + "the cycle time is violated");
    }

    if (messageBuffer.stctInput.errorFlags & VELOCITY_MODE) {
      //   LOG(info) << errorMessage << "has velocity mode active";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    if (messageBuffer.stctInput.errorFlags & POSITION_MODE) {
      //   LOG(info) << errorMessage << "has position mode active";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    if (messageBuffer.stctInput.errorFlags & TORQUE_MODE) {
      //   LOG(info) << errorMessage << "has torque mode active";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    //    if (messageBuffer.stctInput.errorFlags & EMERGENCY_STOP) {
    //      LOG(info) << errorMessage << "has emergency stop active";
    //      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    //    }
    //
    //    if (messageBuffer.stctInput.errorFlags & FREERUNNING) {
    //   //   LOG(info) << errorMessage << "has freerunning active";
    //      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    //    }

    if (messageBuffer.stctInput.errorFlags & POSITION_REACHED) {
      //    LOG(info) << errorMessage << "has position reached";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    if (messageBuffer.stctInput.errorFlags & INITIALIZED) {
      //  LOG(info) << errorMessage << "is initialized";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    if (messageBuffer.stctInput.errorFlags & TIMEOUT) {
      LOG(error) << errorMessage << "has a timeout";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

    if (messageBuffer.stctInput.errorFlags & I2T_EXCEEDED) {
      LOG(error) << errorMessage << "exceeded I2t";
      //   throw JointErrorException(errorMessage + "need to initialize the sinus commutation");
    }

  // Bouml preserved body end 000D24F1
}

std::string EthercatMaster::configFileName;

std::string EthercatMaster::configFilepath;


} // namespace youbot
