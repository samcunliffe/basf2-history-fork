#include "daq/slc/hvcontrol/cdc/CdcHVControlCallback.h"

#include "daq/slc/system/LogFile.h"

// add
//#include <string>
#include <string.h>
#include <iostream>
#include <unistd.h>

// 20150220

using namespace Belle2;

void CdcHVControlCallback::initialize() throw()
{
  const DBObject& config(getConfig().get());
  const DBObjectList& c_crate_v(config.getObjects("crate"));
  for (size_t i = 0; i < c_crate_v.size(); i++) {
    int crateid = i + 1;
    const DBObject& c_crate(c_crate_v[i]);
    const DBObjectList& c_channel_v(c_crate.getObjects("channel"));
    for (size_t j = 0; j < c_channel_v.size(); j++) {
      const DBObject& c_channel(c_channel_v[j]);
      int slot = c_channel.getInt("slot");
      int channel = c_channel.getInt("channel");
      LogFile::debug("crate : %d, slot : %d, channel: %d", crateid, slot, channel);
    }
  }
  // crate power ON
  // Mpod ON
  //  system("snmpset -v 2c -m +WIENER-CRATE-MIB -c private 192.168.0.21 sysMainSwitch.0 i 1");
  //  printf(" !! crate power ON !!\n");
  //
  float v = 0.;
  float cur = 0.;
  int crate = 1;
  int slot = 1;
  int channel = 1;
  int nch = 100 * (slot - 1) + (channel - 1);
  int nch2 = 100 * (2 - 1) + (channel - 1);
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  float voltage = 0.;
  float current = 0.;
  for (int i = 0; i < 5; i++) {
    nch = 100 * (slot - 1) + i;
    nch2 = 100 * (2 - 1) + i;

    v = getConfig().getChannel(crate, slot, channel).getVoltageDemand();
    //    printf("get from table %f, %d\n", v, nch);
    //  sleep(5);
    // set
    voltage = v;
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch, voltage);
    system(buf);
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch2, voltage);
    system(buf);
    cur = getConfig().getChannel(crate, slot, channel).getCurrentLimit();
    current = cur / 1000000.;
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch, current);
    system(buf);
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch2, current);
    system(buf);
  }
  // ramp up/down
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", 0, voltage - 10.);
  system(buf);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", 100, voltage - 15.);
  system(buf);

}

void CdcHVControlCallback::timeout() throw()
{

}

void CdcHVControlCallback::store(int index) throw(IOException)
{
  LogFile::notice("store called : index = %d", index);
}

void CdcHVControlCallback::recall(int index) throw(IOException)
{
  LogFile::notice("recall called : index = %d", index);
}

void CdcHVControlCallback::setSwitch(int crate, int slot, int channel, bool switchon) throw(IOException)
{
  // add
  //  system("snmpset -v 2c -m +WIENER-CRATE-MIB -c private 192.168.0.21 sysMainSwitch.0 i 1");
  //  system("snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d i 1", nch);

  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  if (switchon == true) {
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d i 1", nch);
    system(buf);
  } else if (switchon == false) {
    sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d i 0", nch);
    system(buf);
  }

  LogFile::info("setswitch called : crate = %d, slot = %d, channel = %d, switch: %s",
                crate, slot, channel, (switchon ? "ON" : "OFF"));
}

void CdcHVControlCallback::setRampUpSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{

  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  //  int nch = 100*(slot-1) + (channel-1);
  int nch = 100 * (slot - 1) + (0); //.u0
  // koko
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", nch, voltage);
  //  sprintf(buf,"snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageRiseRate.u%d F %f", nch, voltage);
  system(buf);

  LogFile::info("setrampup called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setRampDownSpeed(int crate, int slot, int channel, float voltage) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  //  int nch = 100*(slot-1) + (channel-1);
  int nch = 100 * (slot - 1) + (0);
  // koko
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageFallRate.u%d F %f", nch, voltage);
  //  sprintf(buf,"snmpset -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltageFallRate.u%d F %f", nch, voltage);
  system(buf);

  LogFile::info("setrampdown called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setVoltageDemand(int crate, int slot, int channel, float voltage) throw(IOException)
{
  // set
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputVoltage.u%d F %f", nch, voltage);
  system(buf);

  // get and check
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  //  int nch = 100*(slot-1) + (channel-1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltage.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    //    sscanf(bufs, "%fV", &v );
    sscanf(bufs, "%fV\n", &v);
  }
  (void) pclose(fp);
  //
  printf("get set ch %f, %f, %d\n", v, voltage, nch);

  LogFile::info("setvoltagedemand called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setVoltageLimit(int crate, int slot, int channel, float voltage) throw(IOException)
{
  // ???
  LogFile::info("setvoltagelimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, voltage);
}

void CdcHVControlCallback::setCurrentLimit(int crate, int slot, int channel, float current) throw(IOException)
{
  char buf[256];
  memset((void*) buf, (int)'\0', sizeof(buf));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(buf, "snmpset -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputCurrent.u%d F %f", nch, current);
  system(buf);

  LogFile::info("setcurrentlimit called : crate = %d, slot = %d, channel = %d, voltage: %f",
                crate, slot, channel, current);
}

float CdcHVControlCallback::getRampUpSpeed(int crate, int slot, int channel) throw(IOException)
{
  // koko
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltageRiseRate.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0.;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    sscanf(bufs, "%fV", &v);
    //    printf("getrampup, %d, %s\n", nch, bufs);
  }
  (void) pclose(fp);

  return v;

  //  return getConfig().getChannel(crate, slot, channel).getRampUpSpeed();

}

float CdcHVControlCallback::getRampDownSpeed(int crate, int slot, int channel) throw(IOException)
{

  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltageFallRate.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    sscanf(bufs, "%fV", &v);
  }
  (void) pclose(fp);

  return v;

  //  return getConfig().getChannel(crate, slot, channel).getRampDownSpeed();
}

float CdcHVControlCallback::getVoltageDemand(int crate, int slot, int channel) throw(IOException)
{
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputVoltage.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    sscanf(bufs, "%fV", &v);
  }
  (void) pclose(fp);

  return v;

  //  //  return getConfig().getChannel(crate, slot, channel).getVoltageDemand();

}

float CdcHVControlCallback::getVoltageLimit(int crate, int slot, int channel) throw(IOException)
{
  //  return getConfig().getChannel(crate, slot, channel).getVoltageLimit();
  return 510.;
}

float CdcHVControlCallback::getCurrentLimit(int crate, int slot, int channel) throw(IOException)
{
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputCurrent.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    sscanf(bufs, "%fV", &v);
  }
  (void) pclose(fp);

  //  return v;
  return v * 1000000.;

  //  return getConfig().getChannel(crate, slot, channel).getCurrentLimit();

}

float CdcHVControlCallback::getVoltageMonitor(int crate, int slot, int channel) throw(IOException)
{
  // koko
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));

  int nch = 100 * (slot - 1) + (channel - 1);
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputMeasurementSenseVoltage.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    //    printf("%s\n", bufs);
    sscanf(bufs, "%fV", &v);
  }
  //  std::cout << "neko "<< nch << " " << v << std::endl;
  (void) pclose(fp);

  return v;

  //  return 6;
}

float CdcHVControlCallback::getCurrentMonitor(int crate, int slot, int channel) throw(IOException)
{

  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));

  int nch = 100 * (slot - 1) + (channel - 1);
  //  sprintf(bufs,"snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputMeasurementCurrent.u%d", nch );
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c public 192.168.0.21 outputMeasurementCurrent.u%d", nch);

  FILE* fp;
  fp = popen(bufs, "r");
  float v = 0;
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    //    printf("%s\n", bufs);
    sscanf(bufs, "%fV", &v);
  }
  //  std::cout << "neko "<< v << std::endl;
  (void) pclose(fp);

  //  return v;
  return v * 1000000.;

  //  return 9;
}

bool CdcHVControlCallback::getSwitch(int crate, int slot, int channel) throw(IOException)
{
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  int nch = 100 * (slot - 1) + (channel - 1);
  // get switch on/off
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d", nch);
  FILE* fp;
  fp = popen(bufs, "r");
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    std::string tmp = std::string(bufs);
    if (strncmp(tmp.c_str(), "on", 2) == 0) {
      return true;
    } else if (strncmp(tmp.c_str(), "off", 3) == 0) {
      return false;
    }
  }
  (void) pclose(fp);

}

int CdcHVControlCallback::getState(int crate, int slot, int channel) throw(IOException)
{
  //  return HVMessage::OCP;
  char bufs[256];
  memset((void*) bufs, (int)'\0', sizeof(bufs));
  int nch = 100 * (slot - 1) + (channel - 1);
  // get switch on/off
  sprintf(bufs, "snmpget -Oqv -v 2c -m +WIENER-CRATE-MIB -c guru 192.168.0.21 outputSwitch.u%d", nch);
  FILE* fp;
  fp = popen(bufs, "r");
  while (fgets(bufs, sizeof(bufs), fp) != NULL) {
    std::string tmp = std::string(bufs);
    if (strncmp(tmp.c_str(), "on", 2) == 0) {
      return HVMessage::ON;
      //      return true;
    } else if (strncmp(tmp.c_str(), "off", 3) == 0) {
      return HVMessage::OFF;
      //      return false;
    }
  }
  (void) pclose(fp);

  //  return HVMessage::neko;
}

