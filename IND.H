#ifndef  _IND_H
#define  _IND_H

struct Indexes {
  int *p;
  int n;
  Indexes();
  Indexes(const Indexes &ind);
  ~Indexes();
  void operator=(const Indexes &ind);
  void Alloc(int _n);

  int contain(int i) const;
};

#endif
