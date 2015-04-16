#include <framework/conditions/ConditionsService.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/Logger.h>
#include <TFile.h>
#include <TList.h>
#include <TXMLEngine.h>
#include <TMD5.h>
#include <TSystem.h>

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

using namespace Belle2;

ConditionsService* ConditionsService::m_Instance = NULL;

ConditionsService* ConditionsService::GetInstance()
{

  if (!m_Instance) m_Instance = new ConditionsService;

  return m_Instance;
};


ConditionsService::ConditionsService()
{
  m_RESTbase = "http://belle2db.hep.pnnl.gov/b2s/rest/v1/";
  m_FILEbase = "http://belle2db.hep.pnnl.gov/";
}

ConditionsService::~ConditionsService()
{

}


void ConditionsService::GetPayloads(std::string GlobalTag, std::string ExperimentName, std::string RunName)
{

  m_run_payloads.clear();

  CURL* curl;
  CURLcode res;

  curl = curl_easy_init();

  int count = 0;

  if (curl) {
    std::string REST_payloads = m_RESTbase + "payloads/?gtName=" + GlobalTag + "&expName=" + ExperimentName + "&runName=" + RunName;
    B2INFO("rest payload call: " << REST_payloads);
    curl_easy_setopt(curl, CURLOPT_URL, REST_payloads.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_payloads);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&count);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      B2ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }

  B2INFO("Conditions service retrieved " << m_run_payloads.size() << " payloads for experiment " << ExperimentName << " and run " <<
         RunName << " listed under global tag " << GlobalTag);

  return ;
}

void DisplayNodes(TXMLEngine* xml, XMLNodePointer_t node, Int_t level)
{
  // this function display all accessible information about xml node and its childs

  printf("%*c node: %s\n", level, ' ', xml->GetNodeName(node));

  // display namespace
  XMLNsPointer_t ns = xml->GetNS(node);
  if (ns != 0)
    printf("%*c namespace: %s refer: %s\n", level + 2, ' ', xml->GetNSName(ns), xml->GetNSReference(ns));

  // display attributes
  XMLAttrPointer_t attr = xml->GetFirstAttr(node);
  while (attr != 0) {
    printf("%*c attr: %s value: %s\n", level + 2, ' ', xml->GetAttrName(attr), xml->GetAttrValue(attr));
    attr = xml->GetNextAttr(attr);
  }

  // display content (if exists)
  const char* content = xml->GetNodeContent(node);
  if (content != 0)
    printf("%*c cont: %s\n", level + 2, ' ', content);

  // display all child nodes
  XMLNodePointer_t child = xml->GetChild(node);
  while (child != 0) {
    DisplayNodes(xml, child, level + 2);
    child = xml->GetNext(child);
  }
}


size_t ConditionsService::parse_return(void* buffer, size_t size, size_t nmemb, void* /*userp*/)
{

  std::string temp(static_cast<const char*>(buffer), size * nmemb);

  //int count = *((int*)userp);
  //count = 0;

  TXMLEngine* xml = new TXMLEngine;

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());

  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  DisplayNodes(xml, mainnode, 1);

  return size * nmemb;
}

size_t ConditionsService::parse_payloads(void* buffer, size_t size, size_t nmemb, void* userp)
{

  std::string temp(static_cast<const char*>(buffer), size * nmemb);

  //  B2INFO("Rest return: "<<temp);

  int count = *((int*)userp);

  TXMLEngine* xml = new TXMLEngine;

  XMLDocPointer_t xmldoc = xml->ParseString(temp.c_str());

  // take access to main node
  XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);

  // display recursively all nodes and subnodes
  //  DisplayNodes(xml, mainnode, 1);

  // parse the payloads
  XMLNodePointer_t payload_node = xml->GetChild(mainnode);
  XMLNodePointer_t child_node = xml->GetChild(payload_node);

  std::string PackageName, ModuleName, PayloadURL, Checksum;

  count = 0;

  while (payload_node) {

    std::string nodeName = xml->GetNodeName(payload_node);
    //    B2INFO("Parsing payload... "<<nodeName);
    if (nodeName == "payload") { /// Found a payload, now parse the needed information.
      count++;
      while (child_node) { // Search for the module
        nodeName = xml->GetNodeName(child_node);

        if (nodeName == "payloadUrl") {
          PayloadURL = xml->GetNodeContent(child_node);
        }
        if (nodeName == "checksum") {
          Checksum = xml->GetNodeContent(child_node);
        }
        if (nodeName == "basf2Module") { // module found
          XMLNodePointer_t module_node = child_node;
          child_node = xml->GetChild(child_node);
          while (child_node) { // Search for the package
            nodeName = xml->GetNodeName(child_node);
            if (nodeName == "name") {
              // Get the module name.
              ModuleName = xml->GetNodeContent(child_node);
            } else if (nodeName == "basf2Package") {
              XMLNodePointer_t package_node = child_node;
              child_node = xml->GetChild(child_node);
              while (child_node) { // Search for the package name
                nodeName = xml->GetNodeName(child_node);
                if (nodeName == "name") {
                  // Get the module name.
                  PackageName = xml->GetNodeContent(child_node);
                }
                child_node = xml->GetNext(child_node);
              }
              child_node = package_node;
            }
            child_node = xml->GetNext(child_node);
          }
          child_node = module_node;
        }
        child_node = xml->GetNext(child_node);
      }

      if (PackageName.size() == 0 || ModuleName.size() == 0 || PayloadURL.size() == 0) {
        B2ERROR("ConditionsService::parse_payload Payload not parsed correctly.");
      } else {
        std::string payloadKey = PackageName + ModuleName;
        B2INFO("Found payload for module " << ModuleName << " in package " << PackageName << " at URL " << PayloadURL <<
               ".  Storing with key: " << payloadKey << " and checksum: " << Checksum);
        ConditionsService::GetInstance()->AddPayloadURL(payloadKey, PayloadURL);
        ConditionsService::GetInstance()->AddChecksum(payloadKey, Checksum);

      }
    }
    payload_node = xml->GetNext(payload_node);
  }

  // Release memory before exit
  xml->FreeDoc(xmldoc);
  delete xml;


  return size * nmemb;

}


void ConditionsService::WritePayloadFile(std::string payloadFileName,
                                         std::string packageName,
                                         std::string moduleName)
{

  TMD5* checksum = TMD5::FileChecksum(payloadFileName.c_str());

  if (!checksum) {
    B2ERROR("Error calculating checksum for file " << payloadFileName << ".  Are you sure it exists?");
    return;
  }
  std::string username = "BASF2 Conditions User";

  std::string description = "Automated add by BASF2, no user description provided.";

  TFile* f = TFile::Open(payloadFileName.c_str());
  if (!f->IsZombie()) {
  } else { // File is not a valid ROOT file.  Consider throwing an error.
    B2WARNING("The conditions payload " << payloadFileName.c_str() << " could not be loaded by ROOT properly.");
  }

  std::string json_header = "{ \"checksum\": \"";
  json_header = json_header + checksum->AsString() + "\" , \"isDefault\": false , \"modifiedBy\": \"" + username +
                "\" , \"description\": \"" + description + "\" }";
  B2INFO("json header: " << json_header);

  CURL* curl;
  CURLcode res;

  // build post
  std::string REST_payloads = m_RESTbase + "package/" + packageName + "/module/" + moduleName + "/payload";
  B2INFO("rest payload post: " << REST_payloads);


  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  struct curl_slist* headerlist = NULL;
  //  static const char buf[] = "Expect:";

  curl_global_init(CURL_GLOBAL_ALL);

  curl_formadd(&post, &last,
               CURLFORM_COPYNAME, "metadata",
               CURLFORM_COPYCONTENTS, json_header.c_str(),
               CURLFORM_CONTENTTYPE, "application/json", CURLFORM_END);
  curl_formadd(&post, &last,
               CURLFORM_COPYNAME, "contents",
               CURLFORM_FILECONTENT, payloadFileName.c_str(),
               CURLFORM_CONTENTTYPE, "application/x-root", CURLFORM_END);

  headerlist = curl_slist_append(headerlist, "Content-Type: multipart/mixed");

  curl = curl_easy_init();
  if (curl) {

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
    curl_easy_setopt(curl, CURLOPT_URL, REST_payloads.c_str());
    curl_easy_setopt(curl, CURLOPT_USERNAME, username.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, parse_return);
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      B2ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
  }

}

size_t ConditionsService:: write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
}


std::string ConditionsService::GetPayloadFileURL(std::string packageName, std::string moduleName)
{


  std::string remote_file = m_run_payloads[packageName + moduleName];
  std::string local_file = m_FILEbase + remote_file; // This will work for local files and CVMFS.



  if (m_FILEbase.substr(0, 7) == "http://") { // May need to transfer files locally.
    local_file = "/tmp/" + boost::filesystem::path(remote_file).filename().string();
    TMD5* checksum = TMD5::FileChecksum(local_file.c_str()); // check if the file exists

    remote_file = m_FILEbase + remote_file;
    if (!checksum) { // file isn't there.  Start downloading.

      B2INFO("Did not find file " << local_file << " starting download from " << remote_file);

      CURL* curl;
      FILE* fp;
      CURLcode res;

      curl = curl_easy_init();
      if (curl) {
        fp = fopen(local_file.c_str(), "wb");
        curl_easy_setopt(curl, CURLOPT_URL, remote_file.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
        //  curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_func);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
          B2ERROR("curl_easy_perform() failed: " << curl_easy_strerror(res));
        }

        curl_easy_cleanup(curl);
        fclose(fp);
      }
      checksum = TMD5::FileChecksum(local_file.c_str()); // check checksum
    } else if (checksum->AsString() == m_run_checksums[packageName + moduleName]) {
      B2INFO("Found file: " << local_file << " with correct MD5 checksum: " << checksum->AsString());
      return local_file;
    }


    TMD5* checksum_new;
    while (checksum->AsString() != m_run_checksums[packageName + moduleName]) { // then there was a checksum mis-match
      gSystem->Sleep(1000);
      checksum_new = TMD5::FileChecksum(local_file.c_str()); // check checksum again
      if (checksum->AsString() != checksum_new->AsString()) {   // Then we are downloading the file already.
        B2INFO("File with incorrect checksum found, download appears to be occuring... waiting for file to complete");
        checksum = checksum_new;
      } else { // File isn't downloading, but checksums don't match.  Throw an error.
        B2ERROR("Error with file " << local_file.c_str() << " checksum expected: " << m_run_checksums[packageName + moduleName] <<
                " found: " << checksum);
      }
    }
  }

  TMD5* checksum = TMD5::FileChecksum(local_file.c_str()); // check if the file exists
  if (!checksum) { // file isn't there.  Toss an error.
    B2ERROR("Did not find file " << local_file << " check inputs.");
  } else if (checksum->AsString() != m_run_checksums[packageName + moduleName]) { // MD5 checksum doesn't match the database entry.
    B2ERROR("Conditions file " << local_file << " error! Expected MD5 checksum " << m_run_checksums[packageName + moduleName] <<
            " and calculated checksum " << checksum->AsString());
  }

  return local_file;
}

int ConditionsService::progress_func(void* /*ptr*/, double TotalToDownload, double NowDownloaded,
                                     double TotalToUpload, double NowUploaded)
{
  // how wide you want the progress meter to be
  int totaldotz = 40;

  double fractiondownloaded = NowDownloaded / TotalToDownload;
  double fractionuploaded   = NowUploaded / TotalToUpload;
  // part of the progressmeter that's already "full"
  int dotz = round(fractiondownloaded * totaldotz);
  printf("%3.0f%% [", fractiondownloaded * 100);
  if (TotalToUpload > 0) {
    dotz = round(fractionuploaded * totaldotz);
    printf("%3.0f%% [", fractionuploaded * 100);
  }

  // create the "meter"
  int ii = 0;

  // part  that's full already
  for (; ii < dotz; ii++) {
    printf("=");
  }
  // remaining part (spaces)
  for (; ii < totaldotz; ii++) {
    printf(" ");
  }
  // and back to line begin - do not forget the fflush to avoid output buffering problems!
  printf("]\r");
  fflush(stdout);
  // if you don't return 0, the transfer will be aborted - see the documentation
  return 0;
}
