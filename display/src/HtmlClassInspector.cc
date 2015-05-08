#include <display/HtmlClassInspector.h>

#include <TDataType.h>
#include <TDataMember.h>
#include <TClass.h>
#include <TStreamerInfo.h>
#include <TStreamerElement.h>
#include <TROOT.h>

using namespace Belle2;

TString HtmlClassInspector::getTable() const
{
  return "<table width=100\% cellpadding=2 bgcolor=eeeeee><tbody>" + m_info + "</tbody></table>";
}
void HtmlClassInspector::Inspect(TClass* cl, const char* pname, const char* mname, const void* add)
{
  // Print value of member mname.
  //
  // This method is called by the ShowMembers() method for each
  // data member when object.Dump() is invoked.
  //
  //    cl    is the pointer to the current class
  //    pname is the parent name (in case of composed objects)
  //    mname is the data member name
  //    add   is the data member address

  const Int_t kvalue = 30;
  const Int_t kline  = 1024;
  Int_t cdate = 0;
  Int_t ctime = 0;
  UInt_t* cdatime = 0;
  char line[kline];

  TDataType* membertype;
  TString memberTypeName;
  TString memberName;
  const char* memberFullTypeName;
  TString memberTitle;
  Bool_t isapointer;
  Bool_t isbasic;

  if (TDataMember* member = cl->GetDataMember(mname)) {
    memberTypeName = member->GetTypeName();
    memberName = member->GetName();
    memberFullTypeName = member->GetFullTypeName();
    memberTitle = member->GetTitle();
    isapointer = member->IsaPointer();
    isbasic = member->IsBasic();
    membertype = member->GetDataType();
  } else if (!cl->IsLoaded()) {
    // The class is not loaded, hence it is 'emulated' and the main source of
    // information is the StreamerInfo.
    TVirtualStreamerInfo* info = cl->GetStreamerInfo();
    if (!info) return;
    const char* cursor = mname;
    while ((*cursor) == '*') ++cursor;
    TString elname(cursor);
    Ssiz_t pos = elname.Index("[");
    if (pos != kNPOS) {
      elname.Remove(pos);
    }
    TStreamerElement* element = (TStreamerElement*)info->GetElements()->FindObject(elname.Data());
    if (!element) return;
    memberFullTypeName = element->GetTypeName();

    memberTypeName = memberFullTypeName;
    memberTypeName = memberTypeName.Strip(TString::kTrailing, '*');
    if (memberTypeName.Index("const ") == 0) memberTypeName.Remove(0, 6);

    memberName = element->GetName();
    memberTitle = element->GetTitle();
    isapointer = element->IsaPointer() || element->GetType() == TVirtualStreamerInfo::kCharStar;
    membertype = gROOT->GetType(memberFullTypeName);

    isbasic = membertype != 0;
  } else {
    return;
  }


  Bool_t isdate = kFALSE;
  if (strcmp(memberName, "fDatime") == 0 && strcmp(memberTypeName, "UInt_t") == 0) {
    isdate = kTRUE;
  }
  Bool_t isbits = kFALSE;
  if (strcmp(memberName, "fBits") == 0 && strcmp(memberTypeName, "UInt_t") == 0) {
    isbits = kTRUE;
  }
  TClass* dataClass = TClass::GetClass(memberFullTypeName);
  Bool_t isTString = (dataClass == TString::Class());
  static TClassRef stdClass("std::string");
  Bool_t isStdString = (dataClass == stdClass);

  Int_t i;
  for (i = 0; i < kline; i++) line[i] = ' ';
  line[kline - 1] = 0;
  //snprintf(line,kline,"%s%s ",pname,mname);
  //i = strlen(line); line[i] = ' ';

  // Encode data value or pointer value
  char* pointer = (char*)add;
  char** ppointer = (char**)(pointer);

  if (isapointer) {
    char** p3pointer = (char**)(*ppointer);
    if (!p3pointer)
      snprintf(&line[kvalue], kline - kvalue, "->0");
    else if (!isbasic)
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)p3pointer);
    else if (membertype) {
      if (!strcmp(membertype->GetTypeName(), "char")) {
        i = strlen(*ppointer);
        if (kvalue + i > kline) i = kline - 1 - kvalue;
        Bool_t isPrintable = kTRUE;
        for (Int_t j = 0; j < i; j++) {
          if (!std::isprint((*ppointer)[j])) {
            isPrintable = kFALSE;
            break;
          }
        }
        if (isPrintable) {
          strncpy(line + kvalue, *ppointer, i);
          line[kvalue + i] = 0;
        } else {
          line[kvalue] = 0;
        }
      } else {
        strncpy(&line[kvalue], membertype->AsString(p3pointer), TMath::Min(kline - 1 - kvalue,
                (int)strlen(membertype->AsString(p3pointer))));
      }
    } else if (!strcmp(memberFullTypeName, "char*") ||
               !strcmp(memberFullTypeName, "const char*")) {
      i = strlen(*ppointer);
      if (kvalue + i >= kline) i = kline - 1 - kvalue;
      Bool_t isPrintable = kTRUE;
      for (Int_t j = 0; j < i; j++) {
        if (!std::isprint((*ppointer)[j])) {
          isPrintable = kFALSE;
          break;
        }
      }
      if (isPrintable) {
        strncpy(line + kvalue, *ppointer, i);
        line[kvalue + i] = 0;
      } else {
        line[kvalue] = 0;
      }
    } else {
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)p3pointer);
    }
  } else if (membertype) {
    if (isdate) {
      cdatime = (UInt_t*)pointer;
      TDatime::GetDateTime(cdatime[0], cdate, ctime);
      snprintf(&line[kvalue], kline - kvalue, "%d/%d", cdate, ctime);
    } else if (isbits) {
      snprintf(&line[kvalue], kline - kvalue, "0x%08x", *(UInt_t*)pointer);
    } else {
      strncpy(&line[kvalue], membertype->AsString(pointer), TMath::Min(kline - 1 - kvalue, (int)strlen(membertype->AsString(pointer))));
    }
  } else {
    if (isStdString) {
      std::string* str = (std::string*)pointer;
      snprintf(&line[kvalue], kline - kvalue, "%s", str->c_str());
    } else if (isTString) {
      TString* str = (TString*)pointer;
      snprintf(&line[kvalue], kline - kvalue, "%s", str->Data());
    } else {
      snprintf(&line[kvalue], kline - kvalue, "->%lx ", (Long_t)pointer);
    }
  }


  TString memberValue(line);
  m_info += "<tr>";
  if (TString(pname) == "")
    m_info += "<td>" + memberName + "</td>";
  else //indent nested members
    m_info += "<td>&nbsp;&nbsp;" + memberName + "</td>";
  m_info += "<td align=right>" + memberValue + "</td>";
  //takes up too much space. alt text on image _might_ work, there's code for it at least.
  //m_info += "<td>" + memberTitle + "</td>";
  m_info += "</tr>";
}
