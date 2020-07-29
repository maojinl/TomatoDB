typedef int size_type;

template <typename _Ty>
class ObjectsPool {
 private:
  typedef _Ty T;
  size_type _M_max_entry;
  T* _M_table;
  size_type* _M_allocate_table;
  size_type _M_read_point;
  size_type _M_write_point;
  size_type _M_assigned;

 public:
  ObjectsPool(size_type n = 0) {
    _M_read_point = 0;
    _M_write_point = 0;
    _M_assigned = 0;
    _M_max_entry = n;
    _M_allocate_table = 0;
    _M_table = 0;
    if (_M_max_entry > 0) {
      _M_table = new T[_M_max_entry];
      _M_allocate_table = new size_type(_M_max_entry);

      for (size_type i = 0; i < _M_max_entry; i++) {
        _M_allocate_table[i] = i;
      }
    }
  }

  int Init(size_type n) {
    if (_M_max_entry > 0) return -1;

    _M_read_point = 0;
    _M_write_point = 0;
    _M_max_entry = n;
    _M_allocate_table = 0;
    _M_table = 0;
    if (_M_max_entry > 0) {
      _M_table = new T[_M_max_entry];
      _M_allocate_table = new size_type[_M_max_entry];

      for (size_type i = 0; i < _M_max_entry; i++) {
        _M_allocate_table[i] = i;
      }
    }

    return 0;
  }

  T* Malloc() {
    if (_M_table == 0) return 0;

    if (_M_assigned >= _M_max_entry) return 0;

    if (_M_max_entry == 0) return 0;

    size_type assignNum = _M_allocate_table[_M_read_point];
    _M_assigned++;
    _M_read_point = (_M_read_point + 1) % _M_max_entry;
    return &_M_table[assignNum];
  }

  int Free(T* pDelete) {
    if (_M_table == 0) return -1;

    if (_M_assigned <= 0) return -1;

    if (_M_max_entry == 0) return -1;

    size_type deleteNo = pDelete - _M_table;

    if (deleteNo < 0) return -1;

    _M_assigned--;
    _M_allocate_table[_M_write_point] = deleteNo;
    _M_write_point = (_M_write_point + 1) % _M_max_entry;
    return 0;
  }

  void Reset() {
    _M_read_point = 0;
    _M_write_point = 0;
    _M_assigned = 0;
  }

  virtual ~ObjectsPool() {
    if (_M_max_entry >= 0) {
      /*if (_M_max_entry == 1)
      {
              delete _M_table;
              delete _M_allocate_table;
      }
      else
      {*/
      delete[] _M_table;
      delete[] _M_allocate_table;
      //}
      _M_table = 0;
      _M_allocate_table = 0;
      _M_read_point = 0;
      _M_write_point = 0;
      _M_assigned = 0;
      _M_write_point = 0;
      _M_max_entry = 0;
    }
  }
};
