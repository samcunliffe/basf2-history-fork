
// class to replace POD 2D array to save stack usage
// since it just allocates memory dynamically.

namespace Belle2 {
  namespace ECL {
    template <typename T>
    class WrapArray2D {
    public:
      WrapArray2D(int rows,  int cols) :
        m_data(new T[rows * cols]), m_ncols(cols)
      {}
      ~WrapArray2D()
      {  delete [] m_data;  }
      T* operator[](int irow)
      {
        return m_data + irow * m_ncols;
      }
      operator T* () { return m_data; }
    private:
      T* m_data;
      int m_ncols;
    };
  }
}
