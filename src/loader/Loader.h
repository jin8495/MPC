#ifndef __LOADER_H__
#define __LOADER_H__

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#define WORD_SIZE uint8_t
#define ACCESS_GRAN 32

typedef uint64_t addr_t;

namespace trace
{

enum rw_t
{
	READ,
	WRITE,
  NA,
};

struct MemReq_t
{
	addr_t addr;
	rw_t rw;

	uint32_t reqSize;
	std::vector<WORD_SIZE> data;

	bool isEnd;

  virtual void Reset()
  {
    addr = 0;
    rw = NA;

    reqSize = 0;
    data.clear();

    isEnd = false;
  }

  // setters
  void Set(MemReq_t &memReq)
  {
    *this = memReq;
  }
  MemReq_t &operator=(MemReq_t &rhs)
  {
    this->addr = rhs.addr;
    this->rw = rhs.rw;
    this->reqSize = rhs.reqSize;
    this->data = rhs.data;
    this->isEnd = rhs.isEnd;
    
    return *this;
  }
};

class Loader
{
public:
	/*** constructors ***/
	Loader(const char *filePath)
    : m_FilePath(filePath)
  {
    m_FileStream.open(m_FilePath, std::ios_base::in | std::ios_base::binary);
  };
	Loader(const std::string filePath)
    : m_FilePath(filePath)
  {
      m_FileStream.open(filePath.c_str(), std::ios_base::in | std::ios_base::binary);
  }
	/*** getters ***/
	virtual MemReq_t* GetCacheline(MemReq_t *) = 0;
  virtual unsigned GetCachelineSize() = 0;
  virtual unsigned long long GetNumLines() = 0;

	/*** methods ***/
	virtual void Reset() = 0;

protected:
	const std::string m_FilePath;
	std::ifstream m_FileStream;

};

}

#endif  // __LOADER_H__
