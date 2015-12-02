#ifndef _Belle2_NSMVar_h
#define _Belle2_NSMVar_h

#include <daq/slc/base/Serializable.h>
#include <daq/slc/base/Date.h>

#include <vector>

namespace Belle2 {

  class NSMVar : public Serializable {

  public:
    enum Type {
      NONE = 0,
      INT,
      FLOAT,
      TEXT,
    };

  public:
    static const NSMVar NOVALUE;

  public:
    NSMVar() : m_name(), m_type(NONE), m_len(0), m_value(NULL) {}
    NSMVar(const std::string& name, Type type, int len, const void* value)
    {
      copy(name, type, len, value);
    }
    NSMVar(const std::string& name, const std::string& value)
    {
      copy(name, TEXT, value.size(), value.c_str());
    }
    NSMVar(const std::string& name, int value) { copy(name, INT, 0, &value); }
    NSMVar(const std::string& name, float value) { copy(name, FLOAT, 0, &value); }
    NSMVar(const std::string& name, int len, int* value) { copy(name, INT, len, value); }
    NSMVar(const std::string& name, int len, float* value) { copy(name, FLOAT, len, value); }
    NSMVar(const std::string& name, const std::vector<int>& value);
    NSMVar(const std::string& name, const std::vector<float>& value);
    NSMVar(const std::string& name) : m_name(name), m_type(NONE), m_len(0), m_value(NULL) {}
    NSMVar(const NSMVar& var) { *this = var; }
    ~NSMVar() throw();

  public:
    const NSMVar& operator=(const NSMVar& var)
    {
      copy(var.m_name, var.m_type, var.m_len, var.m_value, var.m_id, var.m_rev);
      m_node = var.m_node;
      m_date = var.m_date;
      return *this;
    }
    const NSMVar& operator=(int val)
    {
      copy(m_name, INT, 0, &val, m_id, m_rev);
      return *this;
    }
    const NSMVar& operator=(float val)
    {
      copy(m_name, FLOAT, 0, &val, m_id, m_rev);
      return *this;
    }
    const NSMVar& operator=(const std::string& val)
    {
      if (val.size() > 0) {
        copy(m_name, TEXT, val.size() + 1, val.c_str(), m_id, m_rev);
      } else {
        copy(m_name, TEXT, 1, "", m_id, m_rev);
      }
      return *this;
    }
    const NSMVar& operator=(const std::vector<int>& val);
    const NSMVar& operator=(const std::vector<float>& val);
    const NSMVar& operator>>(int& val) const { val = getInt(); return *this; }
    const NSMVar& operator>>(float& val) const { val = getFloat(); return *this; }
    const NSMVar& operator>>(std::string& val) const { val = getText(); return *this; }
    const NSMVar& operator>>(std::vector<int>& val) const;
    const NSMVar& operator>>(std::vector<float>& val) const;

  public:
    void setNode(const std::string& node) { m_node = node; }
    void setName(const std::string& name) { m_name = name; }
    const void* get() const { return m_value; }
    void* get() { return m_value; }
    int size() const;
    const std::string& getNode() const { return m_node; }
    const std::string& getName() const { return m_name; }
    Type getType() const { return m_type; }
    const char* getTypeLabel() const;
    int getLength() const { return m_len; }
    int getInt() const;
    float getFloat() const;
    const char* getText() const;
    int getInt(int i) const;
    float getFloat(int i) const;
    int getId() const { return m_id; }
    int getRevision() const { return m_rev; }
    void setId(int id) { m_id = id; }
    void setRevision(int rev) { m_rev = rev; }
    void setDate(int date) { m_date = date; }
    void setDate(const Date& date) { m_date = date.get(); }
    int getDate() const { return m_date; }

  public:
    virtual void readObject(Reader&) throw(IOException);
    virtual void writeObject(Writer&) const throw(IOException);

  public:
    void copy(const std::string& name, Type type, int len,
              const void* value, int id = 0, int rev = 0);

  private:
    std::string m_node;
    std::string m_name;
    Type m_type;
    int m_len;
    void* m_value;
    int m_id;
    int m_rev;
    int m_date;

  };

  typedef std::vector<NSMVar> NSMVarList;

}

#endif
