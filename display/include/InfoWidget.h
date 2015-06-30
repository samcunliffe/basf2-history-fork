#pragma once

#include <TGHtml.h> //huge number of defines, might want to hide that part.
#include <TString.h>

#include <set>
#include <vector>

class TObject;
class TGWindow;

namespace Belle2 {
  /** text-based info viewer showing DataStore contents.
   *
   **/
  class InfoWidget : public TGHtml {
  public:
    /** ctor. */
    explicit InfoWidget(const TGWindow* p);
    ~InfoWidget();

    /** reset for new event. */
    void newEvent();

    /** Navigate to given URI
     *
     * @param uri what to show, see InfoWidget::URI
     * @param clearSelection when showing an object, this determines  wether to clear
     *        an existing selection
     */
    void show(const char* uri = "main:", bool clearSelection = true);

    /** Navigate to page belonging to given object.
     *
     * Special in that it doesn't clear the current selection.
     */
    void show(const TObject* obj);

    /** navigate to previous page, clearing current page from history. */
    void back();

    /** Used to colour visited links. */
    virtual int IsVisited(const char* uri); //please add 'override' here once CINT is gone

  private:
    /** a parsed URI. allowed URI formats are
     *
     * durability:[entryName[/arrayIndex]] (durability: event/persistent)
     * raw:addressOfTObject (for objects not in DataStore)
     * main: (the mian page)
     */
    struct URI {
      URI(): object(0), entryName(""), arrayIndex(-1) { }
      /** Construct URI by parsing given string. */
      explicit URI(const TString& uri);

      /** get URI string to given object. */
      static TString getURI(const TObject* obj);

      const TObject* object; /**< object referenced (or NULL). */
      TString scheme; /**< scheme name (part before first colon). */
      TString entryName; /**< name of DataStore entry. */
      int arrayIndex; /**< index in array, only valid if arrayName and object are filled. */
    };

    /** create DataStore overview. */
    TString createMainPage() const;
    /** create list of array contents. */
    TString createArrayPage(const URI& uri) const;
    /** create object info. */
    TString createObjectPage(const URI& uri) const;

    /** returns string with title, breadcrumbs, menu. */
    TString getHeader(const URI& uri = URI()) const;

    /** Get object contents (member data). */
    static TString getContents(const TObject* obj);

    /** return HTML-formatted list of related objects. */
    static TString getRelatedInfo(const TObject* obj);

    std::set<TString> m_visited; /**< list of all pages viewed in current event. */

    std::vector<TString> m_history; /**< ordered list of all pages viewed in current event. */

    ClassDef(InfoWidget, 0); /**< text-based info viewer showing DataStore contents. */
  };

}
