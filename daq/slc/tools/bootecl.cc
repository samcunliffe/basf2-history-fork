#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include <daq/slc/base/StringUtil.h>

int cprecl01[] = {5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5010, 5011, 5012, 5013};
int cprecl04[] = {5014, 5015, 5016, 5017, 5018};

int* getcopper(const char* hostname, int& ncpr, std::string& ropcname)
{
  ropcname = Belle2::StringUtil::replace(hostname, "ecl0", "ropc50");
  if (strcmp(hostname, "ecl01") == 0) {
    ncpr = sizeof(cprecl01) / sizeof(int);
    return cprecl01;
  } else if (strcmp(hostname, "ecl04") == 0) {
    ncpr = sizeof(cprecl04) / sizeof(int);
    return cprecl04;
  }
  ncpr = 0;
  return NULL;
}

int main(int argc, char** argv)
{
  bool nsm = false;
  bool cpr = false;
  if (argc == 1) {
    nsm = true;
    cpr = true;
  }
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-nsm") == 0) {
      nsm = true;
      continue;
    } else if (strcmp(argv[i], "-cpr") == 0) {
      cpr = true;
      continue;
    }
  }

  const char* hostname = getenv("HOSTNAME");
  std::string ropcname;
  int ncpr = 0;
  int* cprecl = getcopper(hostname, ncpr, ropcname);
  char s[256];
  if (nsm) {
    sprintf(s, "killall nsmd2 runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall nsmd2 cprcontrold\"", cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    sprintf(s, "bootnsmd2; bootnsmd2 -g");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; bootnsmd2 cpr%d &\"",
              cprecl[i], cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "nsm2socket 9090 -d");
    printf("%s\n", s);
    system(s);
  }
  if (cpr) {
    if (nsm) sleep(1);
    sprintf(s, "killall runcontrold rocontrold");
    printf("%s\n", s);
    system(s);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"killall -9 cprcontrold basf2;\" ", cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    usleep(100000);
    for (int i = 0; i < ncpr; i++) {
      sprintf(s, "ssh cpr%d \"source ~/.bash_profile; cprcontrold cpr%d -d\"", cprecl[i], cprecl[i]);
      printf("%s\n", s);
      system(s);
    }
    sprintf(s, "runcontrold %s -d; rocontrold %s -d", hostname, ropcname.c_str());
    printf("%s\n", s);
    system(s);
    sprintf(s, "logcollectord -d");
    printf("%s\n", s);
    system(s);
  }
  return 0;
}
