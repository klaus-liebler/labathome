#pragma once
#define STM_ERROR_CHECK(x) do {                                         \
        HAL_StatusTypeDef err_rc_ = (x);                                        \
        if (unlikely(err_rc_ != HAL_OK)) {                              \
            log_fatal("HAL Error");                 \
        }                                                               \
    } while(0)

#include <cstdint>
#include <cstdio>
#include <cstring>

template <size_t ERR_CNT, typename T >
class ErrorManager{
  private:
  uint32_t errorsCnt{0};
  T lastErrors[ERR_CNT]={0};
  size_t errIndex{0};
  public:
  void add(T err){
    errorsCnt++;
    lastErrors[errIndex]=err;
    errIndex++;
    errIndex%=ERR_CNT;
  }
  char* GetLast8AsCharBuf_DoNotForgetToFree(){
    char* buf;
    asiprintf(&buf, "[%d,%d,%d,%d,%d,%d,%d,%d]", lastErrors[errIndex], lastErrors[(errIndex+ERR_CNT-1)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-2)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-3)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-4)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-5)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-6)%ERR_CNT], lastErrors[(errIndex+ERR_CNT-7)%ERR_CNT] );
    return buf;
  }
  uint32_t GetCount(){
    return errorsCnt;
  }
};

