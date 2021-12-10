#ifndef __LOADERGPGPU_H__
#define __LOADERGPGPU_H__

#include <map>
#include <queue>
#include <strutil.h>
#include <fmt/core.h>

#include "Loader.h"

namespace trace {
namespace gpgpusim {

#define NUM_KEYS 17

// GPGPU-sim mem request type
enum reqTypeGPU
{
  GLOBAL_ACC_R  = 0,
  LOCAL_ACC_R   = 1,
  CONST_ACC_R   = 2,
  TEXTURE_ACC_R = 3,
  GLOBAL_ACC_W  = 4,
  LOCAL_ACC_W   = 5,
  L1_WRBK_ACC   = 6,
  L2_WRBK_ACC   = 7,
  INST_ACC_R    = 8,
};

// GPGPU-sim mem fetch type
enum fetchTypeGPU
{
  READ_REQUEST  = 0,
  WRITE_REQUEST = 1,
  READ_REPLY    = 2,
  WRITE_ACK     = 3,
};

struct MemReqGPU_t : public MemReq_t
{
  uint8_t kernelID;
  uint64_t cycle;

  uint32_t tpc;    // texture processing cluster
  uint32_t sid;    // shader core id
  uint32_t wid;    // warp id
  uint32_t pc;
  uint32_t instCnt;

  reqTypeGPU reqType;
  fetchTypeGPU mfType;

  // DRAM command
  uint32_t row;
  uint32_t chip;
  uint32_t bank;
  uint32_t col;

  virtual void Reset()
  {
    MemReq_t::Reset();

    kernelID = 0;
    cycle = 0;

    tpc = sid = wid = pc = instCnt = 0;

    reqType = GLOBAL_ACC_R;
    mfType = READ_REQUEST;

    row = chip = bank = col = 0;
  }
};

class LoaderGPGPU : public Loader
{
/*** public member functions ***/
public:
  /*** constructors ***/
  LoaderGPGPU(const char *filePath);
  LoaderGPGPU(const std::string filePath);

  /*** getters ***/
  // Get line size
  // It will reset the filepointer to the beginnig
  virtual unsigned GetCachelineSize();

  // Get a line
  virtual MemReq_t* GetCacheline(MemReq_t *memReq);

  /*** methods ***/
  // reset filepointer
  virtual void Reset();

/*** private member functions ***/
private:
  void isValid();

/*** member variables ***/
protected:
  // header data
  uint8_t m_NumKeys;
  std::map<std::string, int> m_KeySizeList;
  
};

}

namespace samsung {

#define NUM_CH 4

struct DatasetAttr
{
  uint64_t cycle;
  uint8_t clock;

  uint8_t valid[NUM_CH];
  uint8_t ready[NUM_CH];
  uint8_t last[NUM_CH];
  uint32_t strb[NUM_CH];

  uint8_t data[NUM_CH][ACCESS_GRAN];

  void Reset()
  {
    cycle = 0;
    clock = 0;

    memset(valid, 0, sizeof(uint8_t) * NUM_CH);
    memset(ready, 0, sizeof(uint8_t) * NUM_CH);
    memset(last, 0, sizeof(uint8_t) * NUM_CH);

    memset(data, 0, sizeof(uint8_t) * NUM_CH * ACCESS_GRAN);
  }

  void Print(rw_t RW)
  {
    printf("Time:%14lu, clk:%2d, ", cycle, clock);
    for (int i = 0; i < NUM_CH; i++)
      printf("val_%1d:%2d, ", i, valid[i]);
    for (int i = 0; i < NUM_CH; i++)
      printf("rdy_%1d:%2d, ", i, ready[i]);
    if (RW == READ)
      for (int i = 0; i < NUM_CH; i++)
        printf("last_%1d:%2d, ", i, last[i]);
    else if (RW == WRITE)
      for (int i = 0; i < NUM_CH; i++)
        printf("strb_%1d:%08x, ", i, strb[i]);
    for (int i = 0; i < NUM_CH; i++)
    {
      printf("d_%1d: ", i);
      for (int j = 0; j < ACCESS_GRAN; j++)
        printf("%02x", data[i][j]);
      printf(" ");
    }
    printf("\n");
  }
};

struct MemReqGPU_t : public MemReq_t
{
  uint64_t cycle;
  uint8_t ch;
  uint8_t last;
  uint32_t strb;

  virtual void Reset()
  {
    MemReq_t::Reset();

    cycle = 0;
    ch = 0;
    last = 0;
  }
};

class LoaderGPGPU : public Loader
{
/*** public member functions ***/
public:
  /*** constructors ***/
  LoaderGPGPU(const char *filePath);
  LoaderGPGPU(const std::string filePath);

  /*** getters ***/
  // Get line size
  virtual unsigned GetCachelineSize();

  // Get a line in 32B granularity
  virtual MemReq_t* GetCacheline(MemReq_t *memReq);

  /*** public methods ***/
  // Read a line from the file
  bool ReadLine(DatasetAttr &datasetAttr);

  // Reset the file pointer
  virtual void Reset();

/*** private member functions ***/
private:
  void isValid();

  bool readLineR(DatasetAttr &datasetAttr);
  bool readLineW(DatasetAttr &datasetAttr);

  MemReq_t* GetCachelineRead(MemReq_t *memReq);
  MemReq_t* GetCachelineWrite(MemReq_t *memReq);

  std::vector<uint8_t> getHandshakingChannels(DatasetAttr &datasetAttr);

/*** member variables ***/
protected:
  MemReq_t* (LoaderGPGPU::*mp_GetCacheline)(MemReq_t*);
  bool (LoaderGPGPU::*mp_ReadLine)(DatasetAttr&);

  rw_t m_RW;
  std::queue<MemReqGPU_t> m_MemReqQueue;
};


}}

#endif  // __LOADERGPGPU_H__
