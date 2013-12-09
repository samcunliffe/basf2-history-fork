/* ---------------------------------------------------------------------- *\
   nsmlib2.c

   NSM client library
   core part, independent of general usage

   All external library functions have prefix "nsmlib_".
\* ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h> /* for struct hostent */
#include <net/if.h> /* for struct ifconf, ifreq */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/shm.h>

#include "nsm2.h"
#include "nsmlib2.h"

#define NSM2_PORT 8120 /* chosen as it corresponds to 0x2012
			  (2012 is the year started writing NSM2) */
#define NSMENV_HOST   "NSM2_HOST"
#define NSMENV_PORT   "NSM2_PORT"
#define NSMENV_SHMKEY "NSM2_SHMKEY"

#define NSMLIB_MAXRECURSIVE 50    /* default value */

static NSMcontext *nsmlib_context = 0;  /* top of the chain */
static FILE *nsmlib_logfp = 0;
static char *nsmlib_incpath = 0;
static char  nsmlib_errs[1024];    /* error string in case of NSMEUNEXPECTED */
static int   nsmlib_errc = 0;      /* context independent error code */
static int   nsmlib_debugflag = 0; /* context independent error code */

#define ADDR_N(a)    ((uint32)((a).sin_addr.s_addr))

#define VSPRINTF(buf,fmt,ap)  va_start(ap,fmt);vsprintf(buf,fmt,ap);va_end(ap)
#define VFPRINTF(fp,fmt,ap)   va_start(ap,fmt);vfprintf(fp,fmt,ap);va_end(ap)
#define SOCKOPT(s,o,v)        setsockopt(s,SOL_SOCKET,o,(char *)&(v),sizeof(v))

#define MEMPOS(base,ptr) ((char *)(ptr) - (char *)(base))
#define MEMPTR(base,off) ((char *)(base) + off)

#define ASSERT nsmlib_assert
#define ERROR  nsmlib_error
#define DBG    nsmlib_debug
#define LOG    nsmlib_log

int nsmlib_hash(NSMsys *, int32 *hashtable, int hashmax,
		const char *key, int create);
char * nsmlib_parse(const char *datname, int revision, const char *incpath);
char * nsmlib_parse_str(const char *filebuf_in, const char *filepath,
			const char *datname, int revision);
const char *nsmlib_parseerr(int *errcode);

/* -- time1ms -------------------------------------------------------- */
static uint64
time1ms()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64)tv.tv_sec * 1000 + tv.tv_usec / 1000; /* in 1msec unit */
}
/* -- htonll --------------------------------------------------------- */
static uint64
htonll(uint64 h)
{
  static int n42 = 0;
  if (n42 == 0) n42 = htons(42); /* 42 is the answer */
  if (n42 == 42) {
    return h;
  } else {
    uint64 n;
    int32 *hp = (int32 *)&h;
    int32 *np = (int32 *)&n;
    np[0] = htonl(hp[1]);
    np[1] = htonl(hp[0]);
    return n;
  }
}
/* -- nsmlib_assert -------------------------------------------------- */
static void
nsmlib_assert(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  fprintf(nsmlib_logfp, "ASSERT: ");
  VFPRINTF(nsmlib_logfp, fmt, ap);
  /* if (errno) fprintf(nsmlib_logfp, "(%s)", strerror(errno)); */
  fprintf(nsmlib_logfp, "\n");
  exit(1);
}
/* -- nsmlib_error --------------------------------------------------- */
static void
nsmlib_error(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  fprintf(nsmlib_logfp, "Error: ");
  VFPRINTF(nsmlib_logfp, fmt, ap);
  if (errno) fprintf(nsmlib_logfp, "(%s)", strerror(errno));
  fprintf(nsmlib_logfp, "\n");
}
/* -- nsmlib_debuglevel ---------------------------------------------- */
void
nsmlib_debuglevel(int val)
{
  nsmlib_debugflag = val;
}
/* -- nsmlib_debug --------------------------------------------------- */
static void
nsmlib_debug(const char *fmt, ...)
{
  va_list ap;
  if (! nsmlib_logfp) return;
  if (! nsmlib_debugflag) return;
  fprintf(nsmlib_logfp, "[DBG] ");
  VFPRINTF(nsmlib_logfp, fmt, ap);
  fprintf(nsmlib_logfp, "\n");
}
/* -- nsmlib_log ----------------------------------------------------- */
static void
nsmlib_log(const char *fmt, ...)
{
  va_list ap;
  
  if (! nsmlib_logfp) return;
  VFPRINTF(nsmlib_logfp, fmt, ap);
  fprintf(nsmlib_logfp, "\n");
}

/* -- nsm_strerror --------------------------------------------------- */
const char *
nsmlib_strerror(NSMcontext *nsmc)
{
  static char buf[256];
  char *syserr = 0;
  if (! nsmc) {
    switch (nsmlib_errc) {
    case 0: return "NSM is not initialized";
    case NSMESHMGETSYS:
      sprintf(buf, "shmget(sys): %s", strerror(errno));
      return buf;
    default:
      if (*nsmlib_errs) return nsmlib_errs;
      sprintf(buf, "NSM is not initialized (errcode=%d)", nsmlib_errc);
      return buf;
    }
  }

  switch (nsmc->errc) {
  case NSMENOERR:    return "no error";
  case NSMENODENAME: return "invalid nodename";  /* init */
  case NSMEALLOC:    return "memory allocation error"; /* init, etc */
  case NSMEHOSTNAME: return "invalid hostname"; /* initnet */
  case NSMEALREADYP: return "port already in use"; /* initnet */
  case NSMEALREADYS: return "shm key already in use";  /* initnet */
  case NSMEALREADYH: return "host already in use";     /* initnet */
  case NSMESOCKET:     syserr = "socket";       break; /* initnet */
  case NSMESOCKREUSE:  syserr = "so_reuseaddr"; break; /* initnet */
  case NSMESOCKSNDBUF: syserr = "so_sndbuf";    break; /* initnet */
  case NSMESOCKRCVBUF: syserr = "so_rcvbuf";    break; /* initnet */
  case NSMENONSMD:   syserr = "cannot connect to NSMD"; break; /* initnet */
  case NSMERDSELECT: syserr = "read select"; break; /* initnet */
  case NSMENOUID:    return "uid not received"; /* initnet */
  case NSMERDUID:    syserr = "uid read error"; break; /* initnet */
  case NSMERDCLOSE:  return "uid not fully received"; /* initnet */
  case NSMEACCESS:   return "invalid NSMD uid"; /* initnet */
  case NSMECLOSED:   return "NSMD connection closed"; break; /* recv */
  default:
    if (nsmc->errc < 0) {
      sprintf(buf, "undefined error code=%d", nsmc->errc);
    } else {
      sprintf(buf, "no error, code=%d", nsmc->errc);
    }
  }
  if (syserr) {
    sprintf(buf, "%s: %s", syserr, strerror(errno));
  }
  return buf;
}
/* -- nsmlib_atoi ---------------------------------------------------- */
static int
nsmlib_atoi(const char *a, int def)
{
  if (! a || ! *a) return def;
  if (! (isdigit(*a) || (*a == '-' && isdigit(*(a+1))))) {
    return def;
  } else {
    return (int)strtol(a,0,0);
  }
}

/* -- nsmlib_nodename ------------------------------------------------ */
const char *
nsmlib_nodename(NSMcontext *nsmc, int nodeid)
{
  int i;
  NSMsys *sysp;
  const char *namep;
  if (! nsmc || ! (sysp = nsmc->sysp)) return 0;

  if (nodeid == NSMSYS_MAX_NOD) return "(no-name)";
  if (nodeid < 0 || nodeid > NSMSYS_MAX_NOD) return "(invalid)";
  namep = sysp->nod[nodeid].name;
  if (! namep[0]) return 0;
  for (i=0; i <= NSMSYS_NAME_SIZ; i++) {
    if (namep[i] == 0) break;
    if (! isalnum(namep[i]) && namep[i] != '_') return "(broken)";
  }
  if (i > NSMSYS_NAME_SIZ) return "(broken)";
  return namep;
}

/* -- nsmlib_nodeid -------------------------------------------------- */
int
nsmlib_nodeid(NSMcontext *nsmc, const char *nodename)
{
  int i;
  NSMsys *sysp;
  const char *namep;

  if (! nsmc || ! (sysp = nsmc->sysp)) return -1;
  if (! nodename || *nodename == '(')  return -1;
  
  for (i=0; i <= NSMSYS_MAX_NOD; i++) {
    if (strcmp(nodename, sysp->nod[i].name) == 0) return i;
  }
  return -1;
}

/* -- nsmlib_checkif ------------------------------------------------- */
/*
   Check the ethernet interface and determine the IP address to use.
*/
static int
nsmlib_checkif(NSMcontext *nsmc, SOCKAD_IN *sap)
{
  int sock;
  struct ifconf ifc;
  struct ifreq *ifr;
  char buf[4096];
  int ifrgap;
  
  /* -- open socket -- */
  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    ERROR("nsmlib_checkif: socket");
    return NSMESOCKDGRAM;
  }
  
  /* -- broadcast address (only for the first, except localhost) -- */
  memset(buf, 0, sizeof(buf));
  ifc.ifc_len = sizeof(buf);
  ifc.ifc_buf = buf;
  if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
    ERROR("nsmlib_checkif: ioctl getconf");
    return NSMEGIFCONF;
  }
  
  for (ifr = ifc.ifc_req;
       (char *)ifr < (char *)ifc.ifc_req + ifc.ifc_len;
       ifr = (struct ifreq *)((char *)ifr + ifrgap)) {

    
#if defined(__FreeBSD__) || defined(MVME5100)
    /* This ugly code is taken from bind 4.8.x for FreeBSD */
    /* VxWorks on MVME5100 requires this, too */
#define xxx_max(a,b) ((a)>(b)?(a):(b))
#define xxx_size(p)  xxx_max((p).sa_len,sizeof(p))
    ifrgap = sizeof(ifr->ifr_name) + xxx_size(ifr->ifr_addr);
#else
    ifrgap = sizeof(*ifr);
#endif
    
    if (! ifr->ifr_name[0]) break;
    LOG("nsmlib_checkif: checking interface <%s>", ifr->ifr_name);
    
    if (ioctl(sock, SIOCGIFFLAGS, (char *)ifr) < 0) {
      ERROR("nsmlib_checkif: ioctl getflags");
      return NSMEGIFFLAGS;
      
    } else if ((ifr->ifr_flags & IFF_UP) == 0) {
      LOG("nsmlib_checkif: interface <%s> is down", ifr->ifr_name);
      
    } else if ((ifr->ifr_flags & IFF_BROADCAST) == 0) {
      LOG("nsmlib_checkif: interface <%s> does not support broadcast",
	  ifr->ifr_name);

    } else if (ioctl(sock, SIOCGIFADDR, (char *)ifr) < 0) {
      LOG("nsmlib_checkif: interface <%s> has no address", ifr->ifr_name);
      
    } else {
      SOCKAD_IN sa;
      memcpy(&sa, (SOCKAD_IN *)&ifr->ifr_addr, sizeof(sa));
    
      if (sa.sin_addr.s_addr != sap->sin_addr.s_addr) {
	LOG("nsmlib_checkif: address does not match for interface <%s>",
	    ifr->ifr_name);
      } else {
	close(sock);
	return 0;
      }
    }
  }

  LOG("nsmlib_checkif: cannot find network interface for %s",
      inet_ntoa(sap->sin_addr));
  close(sock);
  
  return NSMENOIF;
}
/* -- nsmlib_initnet ------------------------------------------------- */
int
nsmlib_initnet(NSMcontext *nsmc, const char *host, int port)
{
  struct hostent *hp;
  NSMcontext *nsmcp;
  int ret;
  int TRUE = 1;
  int size = NSM_TCPBUFSIZ;
  uid_t nsmd_euid;
  fd_set fdset;
  struct timeval tv;
  
  /* -- environment variables -- */
  if (! port)   port   = nsmlib_atoi(getenv(NSMENV_PORT), NSM2_PORT);
  if (! host)   host   = getenv(NSMENV_HOST);
  
  /* -- port and host -- */
  nsmc->port   = port;
  memset(nsmc->hostname, sizeof(nsmc->hostname), 0);
  if (  host) strncpy(nsmc->hostname, host, sizeof(nsmc->hostname));
  if (! host) gethostname(nsmc->hostname, sizeof(nsmc->hostname));
  if (! nsmc->hostname[0]) return NSMEHOSTNAME;

  hp = gethostbyname(nsmc->hostname);
  memset(&nsmc->sa, 0, sizeof(nsmc->sa));
  memcpy(&nsmc->sa.sin_addr, hp->h_addr, hp->h_length);
  nsmc->sa.sin_family = AF_INET;
  nsmc->sa.sin_port   = htons((short)nsmc->port);

  /* -- check if this port is already used -- */
  for (nsmcp = nsmlib_context; nsmcp; nsmcp = nsmcp->next) {
    if (nsmc == nsmcp) continue;
    if (nsmc->port       == nsmcp->port)       return NSMEALREADYP;
    if (nsmc->shmkey     == nsmcp->shmkey)     return NSMEALREADYS;
    if (ADDR_N(nsmc->sa) == ADDR_N(nsmcp->sa)) return NSMEALREADYH;
  }
  
  /* -- open a socket -- */
  if ((nsmc->sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) return NSMESOCKET;

  /* -- check interface -- */
  if ((ret = nsmlib_checkif(nsmc, &nsmc->sa)) < 0) return ret;

  /* -- socket options -- */
  if (SOCKOPT(nsmc->sock, SO_REUSEADDR, TRUE) < 0) return NSMESOCKREUSE;
  if (SOCKOPT(nsmc->sock, SO_SNDBUF,    size) < 0) return NSMESOCKSNDBUF;
  if (SOCKOPT(nsmc->sock, SO_RCVBUF,    size) < 0) return NSMESOCKRCVBUF;

  /* -- connect -- */
  if (connect(nsmc->sock, (SOCKAD *)&nsmc->sa, sizeof(nsmc->sa)) < 0)
    return NSMENONSMD;

  /* -- should receive 4 bytes -- */
  FD_ZERO(&fdset);
  FD_SET(nsmc->sock, &fdset);
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  while (1) {
    ret = select(nsmc->sock+1, &fdset, 0, 0, &tv);
    if (ret <  0 && errno == EINTR) continue;
    if (ret <  0) return NSMERDSELECT;
    if (ret == 0) return NSMENOUID;
    break;
  }
  
  /* -- check whether the nsmd belongs to the same uid -- */
  while (1) {
    ret = read(nsmc->sock, &nsmd_euid, sizeof(nsmd_euid));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) return NSMERDUID;
    if (ret < sizeof(nsmd_euid)) return NSMERDCLOSE;
    break;
  }
  
  if (nsmd_euid != 0 && nsmd_euid != getuid())
    return NSMEACCESS;
  
  nsmc->initnet_done = 1;
  
  return 0;
}
/* -- nsmlib_initshm ------------------------------------------------- */
/*
  initshm has to be called after initnet, as it depends on nsmc->port
 */
int
nsmlib_initshm(NSMcontext *nsmc, int shmkey)
{
  /* -- environment variables for shmkey -- */
  if (! shmkey) shmkey = nsmlib_atoi(getenv(NSMENV_SHMKEY), nsmc->port);
  nsmc->shmkey = shmkey;

  /* -- system shared memory (read-only) -- */
  LOG("shmkey=%d size=%d\n", shmkey, sizeof(NSMsys));
  nsmc->sysid = shmget(shmkey, sizeof(NSMsys), 0444);
  if (nsmc->sysid < 0) {
    return NSMESHMGETSYS;
  }

  nsmc->sysp = (NSMsys *)shmat(nsmc->sysid, 0, SHM_RDONLY);
  if (nsmc->sysp == (NSMsys *)-1) return NSMESHMATSYS;

  /* -- data shared memory (read/write) -- */
  nsmc->memid = shmget(shmkey+1, sizeof(NSMmem), 0775);
  if (nsmc->memid < 0) return NSMESHMGETMEM;

  nsmc->memp = (NSMmem *)shmat(nsmc->memid, 0, 0);
  if (nsmc->memp == (NSMmem *)-1) return NSMESHMATMEM;

  /* -- done -- */
  nsmc->initshm_done = 1;
  return 0;
}
/* -- nsmlib_select -------------------------------------------------- */
int
nsmlib_select(int fdr, int fdw, unsigned int msec)
{
  struct timeval tv;
  fd_set fdset;
  int ret;
  if (fdr) fdw = 0;
  FD_ZERO(&fdset);
  FD_SET(fdr + fdw, &fdset);
  tv.tv_sec  = msec / 1000;
  tv.tv_usec = (msec % 1000) * 1000;
  while (1) {
    ret = select(fdr+fdw+1, fdr ? &fdset : 0, fdr ? 0 : &fdset, 0, &tv);
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret < 0) {
    ASSERT("nsmlib_select should not reach here - ret=%d", ret);
  }
  return ret;
}
/* -- nsmlib_selectc ------------------------------------------------- */
/*
  usesig = 1 when called from signal handler
             then nsmc->usesig=1 is taken
  usesig = 0 when called from the user program
             then nsmc->usesig<1 is taken
 */
NSMcontext *
nsmlib_selectc(int usesig, unsigned int msec)
{
  fd_set fdset;
  struct timeval tv;
  int ret;
  int highest = -1;
  NSMcontext *nsmc;
  
  FD_ZERO(&fdset);
  for (nsmc = nsmlib_context; nsmc; nsmc = nsmc->next) {
    if (usesig != 0 && nsmc->usesig < 1) continue;
    if (usesig == 0 && nsmc->usesig > 0) continue;
    FD_SET(nsmc->sock, &fdset);
    if (highest < nsmc->sock + 1) highest = nsmc->sock + 1;
  }
  if (highest == -1) return 0;
  
  tv.tv_sec  = msec / 1000;
  tv.tv_usec = (msec % 1000) * 1000;
  while (1) {
    ret = select(highest, &fdset, 0, 0, &tv);
    if (ret != -1 || (errno != EINTR && errno != EAGAIN)) break;
  }
  if (ret > 0) {
    for (nsmc = nsmlib_context; nsmc; nsmc = nsmc->next) {
      if (FD_ISSET(nsmc->sock, &fdset)) return nsmc;
    }
    ASSERT("nsmlib_selectc should not reach here - no fd");
    return 0;
  }
  if (usesig > 0 && msec > 0 && ret <= 0) {
    ASSERT("nsmlib_selectc should not reach here - ret=%d msec=%d usesig=%d", ret, msec, nsmc->usesig);
  }
  return 0;
}
/* -- nsmlib_pipewrite ----------------------------------------------- */
/*
  write one int32 from pipe, to synchronize and get the result
 */
static int
nsmlib_pipewrite(NSMcontext *nsmc, int32 val)
{
  while (1) {
    int ret = write(nsmc->pipe_wr, &val, sizeof(int32));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) ASSERT("nsmlib_pipewrite %s", strerror(errno));
    break;
  }
  return 0;
}
/* -- nsmlib_piperead ------------------------------------------------ */
/*
  read one int32 from pipe, to synchronize and get the result
 */
static int
nsmlib_piperead(NSMcontext *nsmc, int32 *valp)
{
  DBG("piperead 1");
  while (1) {
    int ret = read(nsmc->pipe_rd, valp, sizeof(int32));
    if (ret < 0 && errno == EINTR) continue;
    if (ret < 0) return NSMEPIPEREAD;
    break;
  }
  DBG("piperead done");
  return 0;
}
/* -- nsmlib_queue --------------------------------------------------- */
int
nsmlib_queue(NSMcontext *nsmc, NSMtcphead *hp)
{
  int len = sizeof(NSMtcphead) + sizeof(int32)*hp->npar + htons(hp->len);
  NSMrecvqueue *p = malloc(sizeof(struct NSMtcpqueue *) + len);
  if (! p) return 0;
  
  NSMrecvqueue *q = nsmc->recvqueue;
  if (! q) {
    nsmc->recvqueue = p;
  } else {
    while (q->next) q = q->next;
    q->next = p;
  }
  p->next = 0;
  memcpy((char *)&p->h, (char *)hp, len);
  return 0;
}
/* -- nsmlib_recv ---------------------------------------------------- */
int
nsmlib_recv(NSMcontext *nsmc, NSMtcphead *hp, int wait_msec)
{
  int sock = nsmc->sock;
  int ret;
  int i;
  int recvlen = 0;
  int datalen = sizeof(NSMtcphead);
  char *bufp  = (char *)hp;

  DBG("recv 1");
  ret = nsmlib_select(sock, 0, wait_msec);
  DBG("recv 2");
  if (ret < 0) ASSERT("nsmlib_recv: should not reach here (1)");
  if (ret == 0) return 0; /* time out */

  for (i=0; i<2; i++) { /* i=0 for header / i=1 for data */
    DBG("recv 3 bufp=%x", bufp);
    while (recvlen < datalen) {
      DBG("recv 4");
      if (recvlen > 0) ret = nsmlib_select(sock, 0, 1); /* 1 msec */
      if (ret < 0)  ASSERT("nsmlib_recv: should not reach here (2)");
      if (ret == 0) ASSERT("nsmlib_recv: should not reach here (3)");
      if ((ret = read(sock, bufp+recvlen, datalen)) <= 0) {
	if (ret == -1 && errno == EINTR) continue;
	if (ret < 0) ASSERT("nsmlib_recv read error (1) %s", strerror(errno));
	return (nsmlib_errc = NSMECLOSED);
      }
      datalen -= ret;
      recvlen += ret;
    }

    DBG("recv 5 recvlen=%d", recvlen);
    /* for data */
    bufp += recvlen;
    recvlen = 0;
    datalen = hp->npar * sizeof(int32) + ntohs(hp->len);
  }
  DBG("recv 6 datalen=%d npar=%d len=%d", datalen, hp->npar, ntohs(hp->len));
  return sizeof(NSMtcphead) + datalen;
}
/* -- nsmlib_recvpar ------------------------------------------------- */
int
nsmlib_recvpar(NSMcontext *nsmc)
{
  char buf[NSM_TCPMSGSIZ];
  NSMtcphead *hp = (NSMtcphead *)buf;
  int32 *hp_pars = (int32 *)(buf + sizeof(NSMtcphead));
  int ret = 0;
  int val;

  DBG("recvpar 1");
  if (nsmc->usesig > 0) {
    DBG("recvpar 2");
    ret = nsmlib_piperead(nsmc, &val);
  } else {
    DBG("recvpar 3");
    int64 tlimit = time1ms() + 5000; /* 5 seconds */
    int wait_msec = 5000;
    while (1) {
      DBG("recvpar 4");
      if ((ret = nsmlib_recv(nsmc, hp, wait_msec)) <= 0) {
	if (ret == 0) {
	  sprintf(nsmc->errs, "recvpar: timeout");
	  ret = NSMEUNEXPECTED;
	}
	break;
      }
      DBG("recvpar 5");
	
      if (ntohs(hp->req) != nsmc->reqwait) {
	DBG("recvpar 6 req=%x wait=%x", ntohs(hp->req), nsmc->reqwait);
	if (nsmlib_queue(nsmc, hp) < 0) return -1;
      } else if (hp->npar < 1) {
	DBG("recvpar 7");
	sprintf(nsmc->errs, "recvpar: npar=0 for req=%x", nsmc->reqwait);
	ret = NSMEUNEXPECTED;
	break;
      } else {
	DBG("recvpar 8");
	val = htonl(hp_pars[0]);
	break;
      }
      wait_msec = (tlimit - time1ms());
    }
  }
  DBG("recvpar 9");
  nsmc->reqwait = 0;
  if (ret < 0) return nsmc->errc = ret;
  if (val < 0) nsmc->errc = val;
  return val;
}
/* -- nsmlib_call ---------------------------------------------------- */
void
nsmlib_call(NSMcontext *nsmc, NSMtcphead *hp)
{
  int i;
  NSMmsg msg;
  const char *recvbuf = (char *)hp + sizeof(NSMtcphead);
  NSMrequest *reqp = nsmc->req;
  NSMrequest *reqlast = reqp + nsmc->nreq;

  /* search request */
  msg.req = ntohs(hp->req);
  while (reqp < reqlast) {
    if (reqp->functype != NSMLIB_FNNON && reqp->req == msg.req) break;
    reqp++;
  }
  if (reqp == reqlast) {
    LOG("nsmlib_call unknown req=%04x", msg.req);
    for (reqp = nsmc->req; reqp < reqlast; reqp++) {
      DBG("reqp[%d] %02x %04x", reqp-nsmc->req, reqp->functype, reqp->req);
    }
    return;
  }
  if (! reqp->callback && reqp->functype != NSMLIB_FNSYS) {
    DBG("nsmlib_call no callback for req=%s", reqp->name);
    return;
  }
  DBG("nsmlib_call req=%s", reqp->name);

  /* fill other msg entries */
  msg.req  = ntohs(hp->req);
  msg.seq  = ntohs(hp->seq);
  msg.node = ntohs(hp->src);
  msg.npar = hp->npar;
  msg.len  = ntohs(hp->len);
  msg.pars[0] = msg.pars[1] = 0; /* to be compatible with NSM1 */
  for (i=0; i < msg.npar; i++) {
    msg.pars[i] = ntohl(*(int32 *)(recvbuf + i*sizeof(int32)));
  }
  msg.datap = msg.len ? recvbuf + msg.npar*sizeof(int32) : 0;

  switch (reqp->functype) {
  case NSMLIB_FNSYS:
    if (nsmc->reqwait != msg.req) {
      ASSERT("unexpected call %x, waiting for %x", msg.req, nsmc->reqwait);
    }
    nsmc->reqwait = 0;
    DBG("pipewrite pars0=%d", msg.pars[0]);
    nsmlib_pipewrite(nsmc, msg.pars[0]);
    break;
  case NSMLIB_FNSTD:
    if (nsmc->hook) {
      if (! nsmc->hook(&msg, nsmc)) {
	reqp->callback(&msg, nsmc);
      }
    }
    break;
  }
}
/* -- nsmlib_handler ------------------------------------------------- */
static void
#ifdef SIGRTMIN
nsmlib_handler(int sig, siginfo_t *info, void *ignored)
#else
nsmlib_handler(int sig)
#endif
{
  sigset_t mask;
  int wait_msec = 1000; /* 1 sec only for the first time */
  
  /* DBG("nsmlib_handler sig=%d", sig); */
  
  /* sigset */
  if (! sig) return; /* this should not be directly called */
  sigemptyset(&mask);
  sigaddset(&mask, sig);

  /* in the signal handler mode, it can handle more than one signals */
  while (1) {
    char buf[NSM_TCPMSGSIZ]; /* should not be static */
    NSMcontext *nsmc = nsmlib_selectc(1, wait_msec); /* usesig = 1 */
    wait_msec = 0; /* no wait at the later tries */
    
    if (! nsmc) break;

    /* receive */
    if (nsmlib_recv(nsmc, (NSMtcphead *)buf, 1000) <= 0) { /* 1 sec */
      break;
    }

    /* callback function */
    nsmc->currecursive++;
    sigprocmask(SIG_UNBLOCK, &mask, 0);
    nsmlib_call(nsmc, (NSMtcphead *)buf);
    sigprocmask(SIG_BLOCK, &mask, 0);
    nsmc->currecursive--;
  }

  /* unblock and return */
  /* DBG("nsmlib_handler return"); */
  sigprocmask(SIG_UNBLOCK, &mask, 0);
  return;
}
/* -- nsmlib_callbackid ---------------------------------------------- */
/*
  Calling with callback == 0 will disable the function,
   except for NSMLIB_FNSYS which has no callback.
   Even with callback == 0, it can be used in the non-signal mode.
  Calling with existing req  will replace the function in   the list.
  Calling with NSMLIB_FNNON  will remove  the function from the list.
  System callback functions are protected from removing/replacing.
 */
int
nsmlib_callbackid(NSMcontext *nsmc, int req, const char *name,
		  NSMcallback_t callback, NSMfunctype functype)
{
  NSMrequest *reqp = nsmc->req;
  NSMrequest *reqlast = reqp + nsmc->nreq;
  NSMrequest *reqfound = 0;
  NSMrequest *reqfree = 0;
  int sigfound = 0;
  
  /* basic checks */
  if (! nsmc || ! nsmc->memp)         return NSMENOINIT;
  if (nsmc->nodeid == NSMSYS_MAX_NOD) return NSMEPERM; /* anonymous */

  /* search for existing and free-space */
  while (reqp < reqlast) {
    if (! reqfree && reqp->functype == NSMLIB_FNNON) {
      reqfree = reqp; /* first free place, but still need to search for req */
    }
    if (reqp->req == req) {
      reqfree = reqfound = reqp;
      if (sigfound) break;
    } else if (reqp->callback) {
      sigfound = 1;
      if (reqfound) break;
    }
    reqp++;
  }

  /* system function check */
  if (nsmc->initsig_done   && functype == NSMLIB_FNSYS) return NSMEINVFUNC;
  if (! nsmc->initsig_done && functype != NSMLIB_FNSYS) return NSMEINVFUNC;
  if (reqfound  &&  reqfound->functype == NSMLIB_FNSYS) return NSMEINVFUNC;
  
  /* attach to the end if not found */
  if (! reqfree) {
    if (nsmc->nreq >= NSMLIB_MAX_CALLBACK) return NSMEMAXFUNC;
    reqfree = reqlast;
    nsmc->nreq++;
  }

  /* update request table */
  reqfree->req      = req;
  reqfree->functype = functype;
  reqfree->callback = callback;
  if (name) {
    strcpy(reqfree->name, name);
  } else {
    sprintf(reqfree->name, "#%04x", req);
  }
  return 0;
} 
/* -- nsmlib_callback ------------------------------------------------ */
int
nsmlib_callback(NSMcontext *nsmc, const char *name, 
		NSMcallback_t callback, NSMfunctype functype)
{
  NSMsys *sysp = nsmc->sysp;
  int hash = nsmlib_hash(sysp, sysp->reqhash, NSMSYS_MAX_HASH, name, 0);
  NSMreq *reqp = (NSMreq *)MEMPTR(sysp, ntohl(sysp->reqhash[hash]));

  if (hash < 0) return -1;

  if (strcmp(reqp->name, name)) {
    printf("wrong hash? name %s %s\n", reqp->name, name);
    return -1;
  }
  return nsmlib_callbackid(nsmc, ntohs(reqp->code), name,
			   callback, functype);
}
/* -- nsmlib_usesig -------------------------------------------------- */
void
nsmlib_usesig(NSMcontext *nsmc, int usesig)
{
  if (nsmc) nsmc->usesig = usesig;
}
/* -- nsmlib_initsig ------------------------------------------------- */
int
nsmlib_initsig(NSMcontext *nsmc)
{
#ifdef SIGRTMIN
  void nsmlib_handler(int signo, siginfo_t *info, void *ignored);
  int  sig = SIGRTMIN;
#define NSMLIB_SETHANDLER(s,h) (s).sa_sigaction=(h)
#else
  void nsmlib_handler(int);
  int  sig = SIGUSR1;
#define NSMLIB_SETHANDLER(a,h) (a).sa_sighandler=(h)
#endif
  struct sigaction action;
  int pipes[2];

  nsmc->maxrecursive = NSMLIB_MAXRECURSIVE;
  nsmc->currecursive = 0;

  /* create pipes */
  if (pipe(pipes) < 0) return NSMENOPIPE;
  nsmc->pipe_rd = pipes[0];
  nsmc->pipe_wr = pipes[1];

  /* setup callback handlers */
  nsmlib_callbackid(nsmc, NSMCMD_NEWCLIENT, "cmd_newclient", 0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_ALLOCMEM,  "cmd_allocmem",  0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_OPENMEM,   "cmd_openmem",   0, NSMLIB_FNSYS);
  nsmlib_callbackid(nsmc, NSMCMD_NEWREQ,    "cmd_newreq",    0, NSMLIB_FNSYS);
  /* ... and many more to follow ----> */
  
  /* setup signals */
  sigemptyset(&action.sa_mask);
  sigaddset(&action.sa_mask, sig);
  action.sa_flags - 0;
  NSMLIB_SETHANDLER(action, nsmlib_handler);
  sigaction(sig, &action, 0);

  nsmc->usesig = -1; /* undecided */
  nsmc->initsig_done = 1;
  return 0;
}
/* -- nsmlib_send ---------------------------------------------------- */
int
nsmlib_send(NSMcontext *nsmc, NSMmsg *msgp)
{
  static char buf[NSM_TCPMSGSIZ];
  NSMtcphead *hp = (NSMtcphead *)buf;
  char *datap  = buf + sizeof(NSMtcphead);
  const char *writep;
  int writelen;
  int err = 0;
  int i;

  if (! nsmc)                         return NSMENOINIT;
  if (nsmc->sock < 0)                 return NSMENOINIT;
  if (nsmc->nodeid == NSMSYS_MAX_NOD) return NSMEPERM; /* anonymous */

  if (msgp->len == 0 && msgp->datap != 0)   return NSMEINVDATA;
  if (msgp->len == 0 && msgp->datap != 0)   return NSMEINVDATA;
  
  DBG("nsmlib_send nodeid=%d, req=%04x", msgp->node, msgp->req);
  
  hp->src  = htons(nsmc->nodeid);
  hp->dest = htons(msgp->node);
  hp->req  = htons(msgp->req);
  hp->seq  = htons(msgp->seq);
  hp->npar = msgp->npar;
  hp->len  = htons(msgp->len);
  for (i=0; i<hp->npar; i++) {
    *(int32 *)datap = htonl(msgp->pars[i]);
    datap += sizeof(int32);
  }
  if (msgp->len && msgp->len <= NSM_TCPTHRESHOLD) {
    memcpy(datap, msgp->datap, msgp->len);
    writelen = sizeof(NSMtcphead) + msgp->npar*sizeof(int32) + msgp->len;
  } else {
    writelen = sizeof(NSMtcphead) + msgp->npar*sizeof(int32);
  }
  writep = buf;

  DBG("writep = %x", writep);

  while (writelen > 0) {
    int ret = nsmlib_select(0, nsmc->sock, 1000); /* 1 sec */
    if (ret <  0) { err = NSMESELECT;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMETIMEOUT; goto nsmlib_send_error; }

    ret = write(nsmc->sock, writep, writelen);
    if (ret < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (ret <  0) { err = NSMEWRITE;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMECLOSED; goto nsmlib_send_error; }

    writelen -= ret;
    writep   += ret;
  }

  if (msgp->len > NSM_TCPTHRESHOLD) {
    writelen = msgp->len;
    writep = msgp->datap;
  }
  
  while (writelen > 0) {
    int ret = nsmlib_select(0, nsmc->sock, 1000); /* 1 sec */
    if (ret <  0) { err = NSMESELECT;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMETIMEOUT; goto nsmlib_send_error; }

    ret = write(nsmc->sock, writep, writelen);
    if (ret < 0 && (errno == EINTR || errno == EAGAIN)) continue;
    if (ret <  0) { err = NSMEWRITE;  goto nsmlib_send_error; }
    if (ret == 0) { err = NSMECLOSED; goto nsmlib_send_error; }

    writelen -= ret;
    writep   += ret;
  }
  return 0;
  
 nsmlib_send_error:
  shutdown(nsmc->sock, 2);
  nsmc->sock = -1;
  return err;
}
/* -- nsmlib_initcli ------------------------------------------------- */
static int
nsmlib_initcli(NSMcontext *nsmc, const char *nodename)
{
  NSMmsg msg;
  int ret;

  /* request setup */
  memset(&msg, 0, sizeof(msg));
  msg.node = -1;
  msg.pars[0] = geteuid();
  msg.pars[1] = getpid();
  msg.npar    = 2;
  msg.datap   = nodename;
  if (nodename) msg.len = strlen(nodename) + 1;
#ifdef SIGRTMIN
  msg.req = NSMCMD_NEWCLIENT;
#else
  msg.req = NSMCMD_NEWCLIENTOB; /* obsolete */
#endif

  /* send request */
  nsmc->reqwait = msg.req;
  if ((ret = nsmlib_send(nsmc, &msg)) < 0) {
    nsmc->reqwait = 0;
    return ret;
  }

  /* receive request */
  ret = nsmlib_recvpar(nsmc);
  DBG("initcli: recvpar=%d", ret);
  if (ret < 0) {
    sprintf(nsmlib_errs, "initcli: recvpar error=%d", ret);
    return nsmlib_errc = NSMEUNEXPECTED;
  }
  if (ret > NSMSYS_MAX_NOD) {
    sprintf(nsmlib_errs, "initcli: invalid nodeid=%d", ret);
    return nsmlib_errc = NSMEUNEXPECTED;
  }
  nsmc->nodeid = ret;
  return 0;
}
/* -- nsmlib_sendreqid ----------------------------------------------- */
int
nsmlib_sendreqid(NSMcontext *nsmc,
		 const char *node, uint16 req, uint npar, int *pars,
		 int len, const char *datap)
{
  NSMmsg msg;
  int i;
  int nodeid = nsmlib_nodeid(nsmc, node);

  if (nodeid < 0) return -1;

  msg.req = req;
  msg.seq = nsmc->seq++;
  msg.node = (uint16)nodeid;
  msg.npar = npar > 256 ? 0 : npar;
  for (i=0; i<msg.npar; i++) msg.pars[i] = pars[i];
  msg.len = len;
  msg.datap = datap;
  return nsmlib_send(nsmc, &msg);
}
/* -- nsmlib_reqid --------------------------------------------------- */
int
nsmlib_reqid(NSMcontext *nsmc, const char *reqname)
{
  NSMsys *sysp = nsmc->sysp;
  int hash = nsmlib_hash(sysp, sysp->reqhash, NSMSYS_MAX_HASH, reqname, 0);
  NSMreq *reqp = (NSMreq *)MEMPTR(sysp, ntohl(sysp->reqhash[hash]));

  if (hash < 0) { printf("no hash for %s\n", reqname); return -1; }

  if (strcmp(reqp->name, reqname)) {
    printf("wrong hash? name %s %s\n", reqp->name, reqname);
    return -1;
  }
  return ntohs(reqp->code);
}
/* -- nsmlib_reqname ------------------------------------------------- */
const char *
nsmlib_reqname(NSMcontext *nsmc, int reqid)
{
  NSMsys *sysp = nsmc->sysp;
  reqid -= NSMREQ_FIRST;
  if (reqid < 0 || reqid >= NSMSYS_MAX_REQ) return "(invalid)";
  if (! sysp->req[reqid].name[0]) return "(undef)";
  return sysp->req[reqid].name;
}
/* -- nsmlib_sendreq ------------------------------------------------- */
int
nsmlib_sendreq(NSMcontext *nsmc, const char *node, const char *req,
	       uint npar, int *pars, int len, const char *datap)
{
  int reqid = nsmlib_reqid(nsmc, req);
  if (reqid < 0) { printf("no reqid for %s\n", req); return -1; }
  return nsmlib_sendreqid(nsmc, node, reqid, npar, pars, len, datap);
}
/* -- nsmlib_register_request ---------------------------------------- */
int
nsmlib_register_request(NSMcontext *nsmc, const char *name)
{
  NSMmsg msg;
  int ret;
  int reqid;
  memset(&msg, 0, sizeof(msg));
  msg.req   = NSMCMD_NEWREQ;
  msg.node  = -1;
  msg.len   = strlen(name) + 1;
  msg.datap = name;
  nsmc->reqwait = msg.req;

  /* send request */
  ret = nsmlib_send(nsmc, &msg);
  if (ret < 0) { nsmc->reqwait = 0; return ret; }

  /* receive request */
  ret = nsmlib_recvpar(nsmc);
  if (ret < 0) return ret;
  return 0;
}
/* -- nsmlib_openmem ------------------------------------------------- */
void *
nsmlib_openmem(NSMcontext *nsmc,
		 const char *datname, const char *fmtname, int revision)
{
  const char *fmtstr;
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;
  NSMmsg msg;
  int ret;
  int datid;
  NSMdat *datp;
  
  if (! fmtname) fmtname = datname;
  
  if (revision <= 0) {
    printf("openmem: revision %d\n", revision);
    return 0;
  }
  if (! (fmtstr = nsmlib_parse(fmtname, revision, nsmlib_incpath))) {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);

    printf("openmem: no fmtstr (%d) %s\n", errcode, errstr);
    return 0;
  }

  /* linear search, to be replaced with a hash version */
  for (datid = 0; datid < NSMSYS_MAX_DAT; datid++) {
    datp = sysp->dat + datid;
    if (strcmp(datp->dtnam, datname) == 0) break;
  }
  if (datid == NSMSYS_MAX_DAT) {
    printf("openmem: data %s not found\n", datname);
    return 0;
  }
  if (strcmp(datp->dtfmt, fmtstr) != 0) {
    printf("openmem: data %s fmt mismatch %s %s\n",
	   datname, fmtstr, datp->dtfmt);
    return 0;
  }
  if (ntohs(datp->dtrev) != revision) {
    printf("openmem: data %s revision mismatch %d %d\n",
	   datname, revision, datp->dtrev);
    return 0;
  }

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_OPENMEM;
  msg.node = -1;
  msg.npar = 1;
  msg.pars[0] = datid;
  nsmc->reqwait = msg.req;
  ret = nsmlib_send(nsmc, &msg);
  
  if (ret < 0) {
    printf("openmem: send error ret=%d\n", ret);
    nsmc->reqwait = 0;
    return 0;
  }
  ret = nsmlib_recvpar(nsmc);
  if (ret < 0) {
    printf("openmem: piperead error ret=%d\n", ret);
    return 0;
  }
  if (ret != datp - sysp->dat) {
    printf("openmem: datid mismatch %d %d\n", ret, datp - sysp->dat);
    return 0;
  }
  return (char *)memp + ntohl(datp->dtpos);
}
/* -- nsmlib_openmem ------------------------------------------------- */
void *
nsmlib_openmem_str(NSMcontext *nsmc, const char *filebuf,
		   const char *datname, const char *fmtname, int revision)
{
  const char *fmtstr;
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;
  NSMmsg msg;
  int ret;
  int datid;
  NSMdat *datp;
  
  if (! fmtname) fmtname = datname;
  
  if (revision <= 0) {
    printf("openmem: revision %d\n", revision);
    return 0;
  }
  if (! (fmtstr = nsmlib_parse_str(filebuf, datname, fmtname, revision))) {
    int errcode;
    const char *errstr = nsmlib_parseerr(&errcode);

    printf("openmem: no fmtstr (%d) %s\n", errcode, errstr);
    return 0;
  }

  /* linear search, to be replaced with a hash version */
  for (datid = 0; datid < NSMSYS_MAX_DAT; datid++) {
    datp = sysp->dat + datid;
    if (strcmp(datp->dtnam, datname) == 0) break;
  }
  if (datid == NSMSYS_MAX_DAT) {
    printf("openmem: data %s not found\n", datname);
    return 0;
  }
  if (strcmp(datp->dtfmt, fmtstr) != 0) {
    printf("openmem: data %s fmt mismatch %s %s\n",
	   datname, fmtstr, datp->dtfmt);
    return 0;
  }
  if (ntohs(datp->dtrev) != revision) {
    printf("openmem: data %s revision mismatch %d %d\n",
	   datname, revision, datp->dtrev);
    return 0;
  }

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_OPENMEM;
  msg.node = -1;
  msg.npar = 1;
  msg.pars[0] = datid;
  nsmc->reqwait = msg.req;
  ret = nsmlib_send(nsmc, &msg);
  
  if (ret < 0) {
    printf("openmem: send error ret=%d\n", ret);
    nsmc->reqwait = 0;
    return 0;
  }
  ret = nsmlib_recvpar(nsmc);
  if (ret < 0) {
    printf("openmem: piperead error ret=%d\n", ret);
    return 0;
  }
  if (ret != datp - sysp->dat) {
    printf("openmem: datid mismatch %d %d\n", ret, datp - sysp->dat);
    return 0;
  }
  return (char *)memp + ntohl(datp->dtpos);
}
/* -- nsmlib_allocmem ------------------------------------------------ */
void *
nsmlib_allocmem(NSMcontext *nsmc, const char *datname, const char *fmtname,
		int revision, float cycle)
{
  NSMmsg msg;
  int ret;
  int reqid;
  const char *fmtstr;
  char *p;
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;

  DBG("allocmem 1");
  
  if (! fmtname) fmtname = datname;
  if (! nsmc) { nsmlib_errc = NSMENOINIT; return 0; }
  if (! datname || revision < 0) { nsmc->errc = NSMEINVPAR; }

  if (revision <= 0) {
    nsmc->errc = NSMEINVPAR;
    return 0;
  }
  
  if (! (fmtstr = nsmlib_parse(fmtname, revision, nsmlib_incpath))) return 0;
  if (! (p = malloc(strlen(datname) + strlen(fmtstr) + 2))) return 0;

  sprintf(p, "%s %s", datname, fmtstr);

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_ALLOCMEM;
  msg.node = -1;
  msg.npar = 2;
  msg.pars[0] = (cycle != 0) ? (int)(cycle * 100) : 500; /* in 10ms unit */
  msg.pars[1] = revision;
  msg.len = strlen(p) + 1;
  msg.datap = p;
  nsmc->reqwait = msg.req;
  ret = nsmlib_send(nsmc, &msg);
  free(p);
  if (ret < 0) { nsmc->reqwait = 0; return 0; }
  DBG("allocmem 2");
  ret = nsmlib_recvpar(nsmc);
  DBG("allocmem 3");
  nsmc->reqwait = 0;
  if (ret < 0) return 0;
  printf("allocmem: ret=%d dtpos=%d\n", ret, ntohl(sysp->dat[ret].dtpos));
  p = (char *)memp + ntohl(sysp->dat[ret].dtpos);
  return p;
}
/* -- nsmlib_allocmem ------------------------------------------------ */
void *
nsmlib_allocmem_str(NSMcontext *nsmc, const char *filebuf,
		    const char *datname, const char *fmtname,
		    int revision, float cycle)
{
  NSMmsg msg;
  int ret;
  int reqid;
  const char *fmtstr;
  char *p;
  NSMsys *sysp = nsmc->sysp;
  NSMmem *memp = nsmc->memp;

  DBG("allocmem 1");
  
  if (! fmtname) fmtname = datname;
  if (! nsmc) { nsmlib_errc = NSMENOINIT; return 0; }
  if (! datname || revision < 0) { nsmc->errc = NSMEINVPAR; }

  if (revision <= 0) {
    nsmc->errc = NSMEINVPAR;
    return 0;
  }
  
  if (! (fmtstr = nsmlib_parse_str(filebuf, datname, fmtname, revision))) return 0;
  if (! (p = malloc(strlen(datname) + strlen(fmtstr) + 2))) return 0;

  sprintf(p, "%s %s", datname, fmtstr);

  memset(&msg, 0, sizeof(msg));
  msg.req = NSMCMD_ALLOCMEM;
  msg.node = -1;
  msg.npar = 2;
  msg.pars[0] = (cycle != 0) ? (int)(cycle * 100) : 500; /* in 10ms unit */
  msg.pars[1] = revision;
  msg.len = strlen(p) + 1;
  msg.datap = p;
  nsmc->reqwait = msg.req;
  ret = nsmlib_send(nsmc, &msg);
  free(p);
  if (ret < 0) { nsmc->reqwait = 0; return 0; }
  DBG("allocmem 2");
  ret = nsmlib_recvpar(nsmc);
  DBG("allocmem 3");
  nsmc->reqwait = 0;
  if (ret < 0) return 0;
  printf("allocmem: ret=%d dtpos=%d\n", ret, ntohl(sysp->dat[ret].dtpos));
  p = (char *)memp + ntohl(sysp->dat[ret].dtpos);
  return p;
}
/* -- nsmlib_init ------------------------------------------------------- */
NSMcontext *
nsmlib_init(const char *nodename, const char *host, int port, int shmkey)
{
  NSMcontext *nsmc;
  int ret = 0;
  int i;

  if (! nsmlib_logfp) nsmlib_logfp = stdout;

  if (! nodename || strlen(nodename) > NSMSYS_NAME_SIZ) ret = -1;
  for (i=0; ret == 0 && nodename[i]; i++) {
    if (! isalnum(nodename[i]) && nodename[i] != '_') ret = -1;
  }
  if (ret < 0) {
    nsmlib_errc = NSMENODENAME;
    return 0;
  }
  
  if (! (nsmc = malloc(sizeof(*nsmc)))) {
    nsmlib_errc = NSMEALLOC;
    return 0;
  }

  /* -- set up nsm context -- */
  memset(nsmc, sizeof(*nsmc), 0);
  strcpy(nsmc->nodename, nodename);
  nsmc->sock = -1;

  /* -- network initialization -- */
  if (ret == 0) ret = nsmlib_initnet(nsmc, host, port);

  /* -- shared memory initialization -- */
  if (ret == 0) ret = nsmlib_initshm(nsmc, shmkey);

  /* -- signal handler initialization -- */
  if (ret == 0) ret = nsmlib_initsig(nsmc);

  /* -- client initialization -- */
  if (ret == 0) ret = nsmlib_initcli(nsmc, nodename);

  /* -- nsm context -- */
  if (ret == 0) {
    nsmc->next = nsmlib_context;
    nsmlib_context = nsmc;    /* -- insert at the top -- */
  } else {
    nsmlib_errc = ret;
    free(nsmc);
    nsmc = 0;
  }

  return nsmc;
}
/* -- (emacs outline mode setup) ------------------------------------- */
/*
// Local Variables: ***
// mode:outline-minor ***
// outline-regexp:"^/\\* --[+ ]" ***
// End: ***
*/
